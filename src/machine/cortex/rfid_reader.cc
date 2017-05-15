// EPOS ARM Cortex RFID Reader Mediator Implementation

#include <system/config.h>

#ifdef __RFID_READER_H

#include <machine/cortex/rfid_reader.h>

__BEGIN_SYS

// RFID Reader API

// Class attributes
RFID_Reader::Observed RFID_Reader::_observed;

// Methods


// MFRC522 MIFARE Reader chip from NXP

// Class attributes

// Methods
MFRC522::MFRC522(SPI * spi, GPIO * select, GPIO * reset) : _spi(spi), _select(select), _reset(reset)
{
    _spi->disable();
    _select->direction(GPIO::OUT);
    _select->set();
    _reset->direction(GPIO::INOUT);
    _reset->clear();
}

MFRC522::~MFRC522()
{
    _spi->disable();
}

void MFRC522::initialize()
{
    if(!_reset->get()) {
        _reset->set();
        Machine::delay(100000); // According to datasheet section 8.8.2, the MFRC522 start-up delay time is the start-up time of the crystal oscilator circuit + 37.74us.
    }
    else
        reset();

    // Set timeout
    write_reg(PCD_Register::T_MODE, 0x80); // Start timer automatically

    write_reg(PCD_Register::T_PRESCALER, 0xA9); // freq timer = 40kHz (25μs)
    write_reg(PCD_Register::T_RELOAD_H, 0x03); // Reload timer after 1000 (0x03E8) ticks (25ms)
    write_reg(PCD_Register::T_RELOAD_L, 0xE8);

    write_reg(PCD_Register::TX_ASK, 0x40); // Force 100% ASK modulation
    write_reg(PCD_Register::MODE, 0x3D); // Set CRC coprocessor preset value

    // Turn antenna on
    CPU::Reg8 value = read_reg(PCD_Register::TX_CONTROL);
    if((value & 0x03) != 0x03)
        write_reg(PCD_Register::TX_CONTROL, value | 0x03);
}

void MFRC522::reset()
{
    write_reg(PCD_Register::COMMAND, PCD_Command::SOFT_RESET);
    Machine::delay(100000); // According to datasheet section 8.8.2, the MFRC522 start-up delay time is the start-up time of the crystal oscilator circuit + 37.74us.
    while(read_reg(PCD_Register::COMMAND) & (1 << 4)); // PCD still restarting
}

bool MFRC522::card_present()
{
    CPU::Reg8 buffer_answer[2];
    CPU::Reg8 buffer_size = sizeof(buffer_answer);

    write_reg(PCD_Register::COLL, read_reg(PCD_Register::COLL) & (~0x80));  // Clear bits received after collision.
    CPU::Reg8 valid_bits = 7;
    CPU::Reg8 command = MIFARE::PICC_Command::REQA;
    CPU::Reg8 wait_irq = 0x30;
    PCD_Status_Code status = communicate_with_picc(PCD_Command::TRANSCEIVE, wait_irq, &command, 1, buffer_answer, &buffer_size, &valid_bits);
    if(status == PCD_Status_Code::COLLISION)
        return true;

    if((buffer_size != 2) || (valid_bits != 0))  // ATQA must have 16 bits
        return false;

    return status == PCD_Status_Code::OK;
}

bool MFRC522::read_card(UID * uid, unsigned int valid_bits)
{
    write_reg(PCD_Register::COLL, read_reg(PCD_Register::COLL) & (~0x80));

    bool use_cascade_tag;
    CPU::Reg8 cascade_level = 1;
    CPU::Reg8 buffer[9];
    CPU::Reg8 uid_index, count, index;

    for(;;) {
        switch(cascade_level) {
            case 1:
                buffer[0] = MIFARE::PICC_Command::SEL_CL1;
                uid_index = 0;
                use_cascade_tag = (valid_bits > 0) && (uid->size() > 4);  // Use only when it's known that the UID has more than 4 bytes.
            break;
            case 2:
                buffer[0] = MIFARE::PICC_Command::SEL_CL2;
                uid_index = 3;
                use_cascade_tag = (valid_bits > 0) && (uid->size() > 7); // Use only when it's known that the UID has more than 7 bytes.
            break;
            case 3:
                buffer[0] = MIFARE::PICC_Command::SEL_CL3;
                uid_index = 6;
                use_cascade_tag = false;  // Never used in level 3.
            break;
            default:
                return false;
            break;
        }

        // How many UID bits are known in the current level?
        int current_level_known_bits = valid_bits - (8 * uid_index);
        if(current_level_known_bits < 0)
            current_level_known_bits = 0;

        // Copy known bits from uid to buffer
        index = 2; // destination index in buffer[]
        if(use_cascade_tag)
            buffer[index++] = MIFARE::PICC_Command::CT;

        unsigned int bytes_to_copy = current_level_known_bits / 8 + ((current_level_known_bits % 8) ? 1 : 0); // The number of bytes to be copied.
        if(bytes_to_copy) {
            unsigned int max_bytes = use_cascade_tag ? 3 : 4;  // Max of 4 bytes per level. Cascade Tag takes 1 byte.
            if(bytes_to_copy > max_bytes)
                bytes_to_copy = max_bytes;
            for(count = 0; count < bytes_to_copy; count++)
                buffer[index++] = uid->uid()[uid_index + count];
        }

        // Include the 8 bits in Cascade Tag in current_level_known_bits
        if(use_cascade_tag)
            current_level_known_bits += 8;

        CPU::Reg8 response_length, tx_last_bits, buffer_used;
        CPU::Reg8* response_buffer;
        // Repeat anti collision loop until it is possible to transmit all UID bits + BCC and receive a SAK
        for(;;) {
            if(current_level_known_bits >= 32) { // All UID bits in this Cascade Level are known. This is a SELECT.
                buffer[1] = 0x70; // Number of Valid Bits = Seven bytes
                buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];  // Calculate BCC.
                unsigned int result = crc(buffer, 7, &buffer[7]);  // Calculate CRC_A.
                if(result != PCD_Status_Code::OK)
                    return false;

                tx_last_bits = 0;  // 0 -> 8 valid bits
                buffer_used = 9;
                // Store response in the last 3 bytes of buffer (BCC and CRC_A - not needed after tx)
                response_buffer = &buffer[6];
                response_length = 3;
            } else { // anti collision
                tx_last_bits   = current_level_known_bits % 8;
                count          = current_level_known_bits / 8; // Number bytes in the UID part.
                index          = 2 + count;                    // Number bytes: SEL + NVB + UIDs
                buffer[1]      = (index << 4) + tx_last_bits;  // Number of Valid Bits
                buffer_used    = index + (tx_last_bits ? 1 : 0);
                // Store response in the unused part of buffer
                response_buffer = &buffer[index];
                response_length = sizeof(buffer) - index;
            }

            CPU::Reg8 rx_align = tx_last_bits;
            write_reg(PCD_Register::BIT_FRAMING, (rx_align << 4) + tx_last_bits);

            CPU::Reg8 wait_irq = 0x30;
            PCD_Status_Code result = communicate_with_picc(
                PCD_Command::TRANSCEIVE, wait_irq, buffer, buffer_used, response_buffer, &response_length, &tx_last_bits, rx_align
            );

            if(result == PCD_Status_Code::COLLISION) { // More than one PICC found
                CPU::Reg8 coll_reg_val = read_reg(PCD_Register::COLL);
                if(coll_reg_val & 0x20) // CollPosNotValid
                    return false; // No valid collision position.

                CPU::Reg8 collision_pos = coll_reg_val & 0x1F;
                if(collision_pos == 0)
                    collision_pos = 32;

                if(collision_pos <= current_level_known_bits) // No progress
                    return false;

                current_level_known_bits = collision_pos;
                count = (current_level_known_bits - 1) % 8; // The bit to be modified
                index = 1 + (current_level_known_bits / 8) + (count ? 1 : 0);
                buffer[index] |= (1 << count);
            }
            else if(result != PCD_Status_Code::OK)
                return false;
            else {
                if(current_level_known_bits >= 32)  // SELECT done
                    break;
                else
                    current_level_known_bits = 32;
            }
        }

        // Copy UID bytes from buffer to uid
        index = (buffer[2] == MIFARE::PICC_Command::CT) ? 3 : 2;
        bytes_to_copy = (buffer[2] == MIFARE::PICC_Command::CT) ? 3 : 4;
        memcpy(&uid->uid()[uid_index], &buffer[index], bytes_to_copy);

        // Check response SAK
        if((response_length != 3) || (tx_last_bits != 0)) // SAK must have 24 bits (1 byte + CRC_A).
            return false;

        // Verify CRC_A
        if(crc(response_buffer, 1, &buffer[2]) != PCD_Status_Code::OK)
            return false;

        if((buffer[2] != response_buffer[1]) || (buffer[3] != response_buffer[2]))
            return false;

        if(response_buffer[0] & 0x04) // Cascade bit set - UID not ready
            cascade_level++;
        else {
            uid->size(3 * cascade_level + 1);
            uid->sak(response_buffer[0]);
            break;
        }
    }

    return true;
}

void MFRC522::halt_card()
{
    CPU::Reg8 buffer[4];

    buffer[0] = MIFARE::PICC_Command::HLTA;
    buffer[1] = 0;

    if(crc(buffer, 2, &buffer[2]) != PCD_Status_Code::OK)
        return;

    CPU::Reg8 wait_irq = 0x30; // RX_IRQ and IDLE_IRQ
    communicate_with_picc(PCD_Command::TRANSCEIVE, wait_irq, buffer, sizeof(buffer));
}

unsigned int MFRC522::put(unsigned int block, const Block data)
{
    CPU::Reg8 command[2] = {MIFARE::PICC_Command::MF_WRITE, static_cast<CPU::Reg8>(block)};
    if(mifare_transceive(command, sizeof(command)) != PCD_Status_Code::OK)
        return 0;
    return mifare_transceive(data, 16) == PCD_Status_Code::OK;
}

unsigned int MFRC522::read(unsigned int block, Block data)
{
    CPU::Reg8 buffer[18];
    buffer[0] = MIFARE::PICC_Command::MF_READ;
    buffer[1] = block;
    if(crc(buffer, 2, &buffer[2]) != PCD_Status_Code::OK)
        return 0;
    else {
        CPU::Reg8 wait_irq = 0x30;
        CPU::Reg8 buffer_size = sizeof(buffer);
        PCD_Status_Code status = communicate_with_picc(PCD_Command::TRANSCEIVE, wait_irq, buffer, 4, buffer, &buffer_size, 0, 0, true);
        if(status == PCD_Status_Code::OK)
            memcpy(data, buffer, 16);
        return status == PCD_Status_Code::OK;
    }
}

bool MFRC522::authenticate(unsigned int sector, const Key & key, const UID & uid)
{
    CPU::Reg8 block;
    block = MFRC522::key_block(sector);

    CPU::Reg8 cmd = key.type() == Key::TYPE_A ? MIFARE::MF_AUTH_KEY_A : MIFARE::MF_AUTH_KEY_B;

    // buffer[0..1] = {cmd, block}
    CPU::Reg8 buffer[12] = {cmd, block};

    // buffer[2..7] = key bytes
    for(CPU::Reg8 i = 0; i < key.size(); i++)
        buffer[2+i] = key.key()[i];

    for(unsigned int i = 0; i < 4; i++)
        buffer[8+i] = uid.uid()[(uid.size()-4)+i];

    CPU::Reg8 wait_irq = 0x10; //IdleIRq

    return communicate_with_picc(
        PCD_Command::MF_AUTHENT, wait_irq, buffer, sizeof(buffer)
    ) == PCD_Status_Code::OK;
}

MFRC522::PCD_Status_Code MFRC522::crc(const void * data, unsigned int length, unsigned char * result)
{
    write_reg(PCD_Register::COMMAND, PCD_Command::IDLE);    // Stop active commands
    write_reg(PCD_Register::DIV_IRQ, 0x04);                  // Clear CRCIRq interrupt request bit
    write_reg(PCD_Register::FIFO_LEVEL, read_reg(PCD_Register::FIFO_LEVEL) | 0x80);  // FIFO initialization
    write_reg(PCD_Register::FIFO_DATA, length, data);        // Push data to the FIFO
    write_reg(PCD_Register::COMMAND, PCD_Command::CALC_CRC); // Start calculation

    // Each loop iteration takes 7.3 us. Timeout if it takes more than 90 ms.
    for(unsigned int i = 13000;;) {
        CPU::Reg8 n = read_reg(PCD_Register::DIV_IRQ);
        if(n & 0x04) // CRC_IRQ bit set - calculation done
            break;

        if(--i == 0)
            return PCD_Status_Code::TIMEOUT;
    }

    result[0] = read_reg(PCD_Register::CRC_RESULT_L);
    result[1] = read_reg(PCD_Register::CRC_RESULT_H);

    return PCD_Status_Code::OK;
}


MFRC522::PCD_Status_Code MFRC522::communicate_with_picc(
    CPU::Reg8 command, CPU::Reg8 wait_irq, CPU::Reg8* send_data, CPU::Reg8 send_len, CPU::Reg8* back_data,
    CPU::Reg8* back_len, CPU::Reg8* valid_bits, CPU::Reg8 rx_align, bool check_crc
)
{
    // Prepare values forBitFramingReg
    CPU::Reg8 tx_last_bits = valid_bits ? *valid_bits : 0;
    CPU::Reg8 bit_framing = (rx_align << 4) + tx_last_bits;  // rx_align = bit_framing_reg[6..4]. tx_last_bits = bit_framing_reg[2..0]

    write_reg(PCD_Register::COMMAND, PCD_Command::IDLE);     // Stop active commands
    write_reg(PCD_Register::COM_IRQ, 0x7f);                  // Clear all interrupt request bits
    write_reg(PCD_Register::FIFO_LEVEL, read_reg(PCD_Register::FIFO_LEVEL) | 0x80); // FIFO initialization
    write_reg(PCD_Register::FIFO_DATA, send_len, send_data); // Push data to the FIFO
    write_reg(PCD_Register::BIT_FRAMING, bit_framing);       // Bit adjustments
    write_reg(PCD_Register::COMMAND, command);               // Start command execution

    if(command == PCD_Command::TRANSCEIVE)
        write_reg(PCD_Register::BIT_FRAMING, read_reg(PCD_Register::BIT_FRAMING) | 0x80); // StartSend=1, transmission of data starts

    // Each loop iteration takes 7.3 us. Timeout if it takes more than 90 ms.
    for(unsigned int i = 13000;;) {
        CPU::Reg8 n = read_reg(PCD_Register::COM_IRQ);
        if(n & wait_irq) // Success interruption bit set
            break;
        else if(n & 0x01)
            return PCD_Status_Code::TIMEOUT;
        else if(--i == 0)
            return PCD_Status_Code::TIMEOUT;
    }

    CPU::Reg8 error = read_reg(PCD_Register::ERROR_REG);
    if(error & 0x13) // BufferOvfl ParityErr ProtocolErr
        return PCD_Status_Code::ERROR;

    CPU::Reg8 aux_valid_bits = 0;

    // Get data back from MFRC522 when requested.
    if(back_data && back_len) {
        CPU::Reg8 n = read_reg(PCD_Register::FIFO_LEVEL);
        if(n > *back_len)
            return PCD_Status_Code::NO_ROOM;

        *back_len = n;
        read_reg(PCD_Register::FIFO_DATA, n, back_data, rx_align); // Get data from FIFO
        aux_valid_bits = read_reg(PCD_Register::CONTROL) & 0x07; // Number of valid bits in the last received byte. 0b000 -> byte valid.
        if(valid_bits)
            *valid_bits = aux_valid_bits;
    }

    if(error & 0x08) // CollErr
        return PCD_Status_Code::COLLISION;

    // Perform CRC_A validation when requested.
    if(back_data && back_len && check_crc) {
        if((*back_len == 1) && (aux_valid_bits == 4))
            return PCD_Status_Code::MIFARE_NACK;

        if((*back_len < 2) || (aux_valid_bits != 0))
            return PCD_Status_Code::CRC_WRONG;

        CPU::Reg8 control_buffer[2];
        PCD_Status_Code status = crc(&back_data[0], *back_len - 2, &control_buffer[0]);
        if(status != PCD_Status_Code::OK)
            return status;

        if((back_data[*back_len - 2] != control_buffer[0]) || (back_data[*back_len - 1] != control_buffer[1]))
            return PCD_Status_Code::CRC_WRONG;
    }

    return PCD_Status_Code::OK;
}

MFRC522::PCD_Status_Code MFRC522::mifare_transceive(const CPU::Reg8* data, CPU::Reg8 length, bool accept_timeout)
{
    if((data == 0) || (length > 16))
        return PCD_Status_Code::INVALID;

    CPU::Reg8 buffer[18]; // 16 bytes of data and 2 bytes of CRC_A
    // Copy data[] to buffer[]
    memcpy(buffer, data, length);

    // Add CRC_A to buffer[]
    PCD_Status_Code result = crc(buffer, length, &buffer[length]);
    if(result != PCD_Status_Code::OK)
        return result;
    length += 2;

    // transceive data and store reply in buffer[]
    CPU::Reg8 wait_irq = 0x30; // RxIRq and IdleIRq
    CPU::Reg8 bufferSize = sizeof(buffer);
    CPU::Reg8 validBits = 0;
    result = communicate_with_picc(
        PCD_Command::TRANSCEIVE, wait_irq, buffer,
        length, buffer, &bufferSize, &validBits
    );

    if(accept_timeout && (result == PCD_Status_Code::TIMEOUT))
        return PCD_Status_Code::OK;
    else if(result != PCD_Status_Code::OK)
        return result;

    // PICC must reply an ACK with 4 bits
    if((bufferSize != 1) || (validBits != 4))
        return PCD_Status_Code::ERROR;
    else if(buffer[0] != MIFARE::MIFARE_Misc::MF_ACK)
        return PCD_Status_Code::MIFARE_NACK;

    return PCD_Status_Code::OK;
}

CPU::Reg8 MFRC522::read_reg(PCD_Register reg)
{
    _spi->enable();
    _select->clear();
    _spi->put_data(0x80 | (reg & 0xfe));  // MSB == 1 -> read  // LSB is always 0
    _spi->get_data();
    _spi->put_data(0);
    CPU::Reg8 data = _spi->get_data();
    _select->set();
    _spi->disable();
    return data;
}

void MFRC522::read_reg(PCD_Register reg, CPU::Reg8 count, CPU::Reg8* values, CPU::Reg8 rx_align)
{
    if(count == 0)
        return;

    CPU::Reg8 address = 0x80 | (reg & 0xfe); // MSB == 1 -> read  // LSB is always 0
    unsigned int index = 0;
    count--;
    _spi->enable();
    _select->clear();
    _spi->put_data(address);
    _spi->get_data();
    while (index < count) {
        if(index == 0 && rx_align) {  // Only update bit positions rx_align..7 in values[0]
            CPU::Reg8 mask = 0;
            for(CPU::Reg8 i = rx_align; i <= 7; i++)
                mask |= (1 << i);

            _spi->put_data(address);
            CPU::Reg8 value = _spi->get_data();
            // Apply mask to current value of values[0] and the new data.
            values[0] = (values[index] & ~mask) | (value & mask);
        } else {
            _spi->put_data(address);
            values[index] = _spi->get_data();
        }
        index++;
    }
    _spi->put_data(0);
    values[index] = _spi->get_data();
    _select->set();
    _spi->disable();
}

void MFRC522::write_reg(PCD_Register reg, CPU::Reg8 value)
{
    _spi->enable();
    _select->clear();
    _spi->put_data((reg & 0x7e)); // MSB == 0 -> write  // LSB is always 0
    _spi->get_data();
    _spi->put_data(value);
    _spi->get_data();
    _select->set();
    _spi->disable();
}

void MFRC522::write_reg(PCD_Register reg, unsigned int count, const void * values)
{
    _spi->enable();
    _select->clear();
    _spi->put_data(reg & 0x7E); // MSB == 0 -> write  // LSB is always 0
    _spi->get_data();
    for(unsigned int i = 0; i < count; i++) {
        _spi->put_data(reinterpret_cast<const char*>(values)[i]);
        _spi->get_data();
    }
    _select->set();
    _spi->disable();
}


// W400 Wiegand Reader from Khomp

// Class attributes
W400 * W400::_instance;

// Methods
W400::~W400()
{
    _input0->int_disable();
    _input1->int_disable();
    if(_instance == this)
        _instance = 0;
}

void W400::reset()
{
    int_disable();
    reset_package();
    _input0->direction(GPIO::IN);
    _input1->direction(GPIO::IN);
    _input0->handler(&RFID_Reader::input0_handler, GPIO::FALLING);
    _input1->handler(&RFID_Reader::input1_handler, GPIO::RISING);
    int_enable();
}

void W400::int_enable()
{
    _input0->int_enable();
    _input1->int_enable();
}

void W400::int_disable()
{
    _input0->int_disable();
    _input1->int_disable();
}

__END_SYS

#endif
