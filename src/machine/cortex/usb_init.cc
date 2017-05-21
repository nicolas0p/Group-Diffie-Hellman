// EPOS Cortex USB Mediator Initialization

#include <ic.h>
#include <usb.h>
#include <machine.h>

__USING_SYS

#ifdef __USB_H

volatile USB_2_0::STATE USB::_state;
volatile bool USB::_ready_to_put = false;
bool USB::_ready_to_put_next = false;
bool USB::_was_locked = false;

const USB_2_0::Descriptor::Device USB::_device_descriptor =
{
    sizeof(Descriptor::Device), // Descriptor length
    DESC_DEVICE,               // Descriptor type
    0x0200,                    // USB version 2.0
    0x02,                      // Device class (CDC)
    0x00,                      // Device subclass
    0x00,                      // Device protocol
    _max_packet_ep0,           // Max packet size for endpoint 0
    0x1d50,                    // Vendor ID (Openmoko)
    0x601c,                    // Product ID (Epos Mote II)
    0x03,                      // Device release number
    0x00,                      // Manufacturer string index
    0x00,                      // Product string index
    0x00,                      // Serial number string index
    0x01,                      // Number of possible configurations
};

const USB::Full_Config USB::_config =
{
    //_configuration_descriptor =
    {
        sizeof(Descriptor::Configuration), // Descriptor length
        DESC_CONFIGURATION,              // Descriptor type (DESC_CONFIGURATION)
        sizeof(Full_Config),         // Total length of data returned for this configuration
        0x02,                            // Number of interfaces supported by this configuration
        0x01,                            // Value to use as an argument to the SetConfiguration() request to select this configuration
        0x00,                            // Index of string descriptor describing this configuration
        0xA0,                            // Configuration characteristics (remote wakeup = 1)
        25                               // Maximum power consumption from the bus in this specific configuration. (50mA)
    },
    //_interface0_descriptor =
    {
        sizeof(Descriptor::Interface), // Descriptor length
        DESC_INTERFACE,               // Descriptor type (DESC_INTERFACE)
        0x00,                         // Number of this interface
        0x00,                         // Number Value used to select this alternate setting
        0x01,                         // Number of endpoints used by this interface (excluding endpoint zero)
        0x02,                         // Class code (CDC)
        0x02,                         // Subclass code (Abstract control model)
        0x01,                         // Protocol code (V25TER)
        0x00                          // Index of string descriptor describing this interface
    },
    //_cdc_header_descriptor =
    {
        sizeof(CDC::Functional_Descriptor::Header),
        DESC_CLASS_SPECIFIC_INTERFACE,
        CDC::DESC_HEADER,
        0x0110
    },
    //_cdc_acm_descriptor =
    {
        sizeof(CDC::Functional_Descriptor::Abstract_Control_Management),
        DESC_CLASS_SPECIFIC_INTERFACE,
        CDC::DESC_ABS_CTRL_MGMT,
        0x02
    },
    //_cdc_ui_descriptor =
    {
        sizeof(CDC::Functional_Descriptor::Union_Interface),
        DESC_CLASS_SPECIFIC_INTERFACE,
        CDC::DESC_UNION_IF,
        0x00,
        0x01
    },
    //_cdc_cm_descriptor =
    {
        sizeof(CDC::Functional_Descriptor::Call_Management),
        DESC_CLASS_SPECIFIC_INTERFACE,
        CDC::DESC_CALL_MGMT,
        0x00,
        0x01
    },
    //_endpoint0_descriptor =
    {
        sizeof(Descriptor::Endpoint),// Descriptor length
        DESC_ENDPOINT,               // Descriptor type (DESC_ENDPOINT)
        2 | (1 << 7),                // Encoded address: Endpoint 2, IN
        EP_ATTR_INT,                 // Endpoint attributes (Interrupt endpoint)
        _max_packet_ep2,             // Maximum packet size this endpoint is capable of sending or receiving at once
        0x40                         // Interval for polling endpoint (64 full-speed frames = 64 ms)
    },
    //_interface1_descriptor =
    {
        sizeof(Descriptor::Interface),
        DESC_INTERFACE,
        0x01,
        0x00,
        0x02,
        0x0A,
        0x00,
        0x00,
        0x00
    },
    //_endpoint1_descriptor =
    {
        sizeof(Descriptor::Endpoint),// Descriptor length
        DESC_ENDPOINT,               // Descriptor type (DESC_ENDPOINT)
        3 | (1 << 7),                // Encoded Address: Endpoint 3, IN
        EP_ATTR_BULK,                // Endpoint attributes (Bulk endpoint)
        32,                          // Maximum packet size this endpoint is capable of sending or receiving at once
        0x00                         // Interval (ignored for Bulk operation)
    },
    //_endpoint2_descriptor =
    {
        sizeof(Descriptor::Endpoint),// Descriptor length
        DESC_ENDPOINT,               // Descriptor type (DESC_ENDPOINT)
        4,                           // Encoded address: Endpoint 4, OUT
        EP_ATTR_BULK,                // Endpoint attributes (Bulk endpoint)
        _max_packet_ep4,             // Maximum packet size this endpoint is capable of sending or receiving at once
        0x00                         // Interval (ignored for Bulk operation)
    }
};

// Configurations that need to be done at every USB reset
void USB::reset()
{
    _ready_to_put = false;
    _ready_to_put_next = false;
    _state = USB_2_0::STATE::DEFAULT;
    _send_buffer = reinterpret_cast<const char *>(0);
    _send_buffer_size = 0;

    // Set up endpoints
    output();
    // The two lines below make the USB automatically signal that a packet is ready every 32 characters.
    reg(MAXI) = 32 / 8; // Endpoint 3, IN.
    reg(MAXO) = 0;
    reg(CS0_CSIL) |= CSIL_CLRDATATOG; // From cc2538 User Guide: When a Bulk IN endpoint is first configured, USB_CSIL.CLRDATATOG should be set.
    // if there are any data packets in the FIFO, they should be flushed. It may be necessary to set this bit twice in succession if double buffering is enabled.
    reg(CS0_CSIL) |= CSIL_FLUSHPACKET;
    reg(CS0_CSIL) |= CSIL_FLUSHPACKET;

    input();
    reg(MAXI) = 0;
    reg(MAXO) = _max_packet_ep4 / 8; // Endpoint 4, OUT
    reg(CSOL) |= CSOL_CLRDATATOG; // From cc2538 User Guide: When a Bulk OUT endpoint is first configured, USB_CSOL.CLRDATATOG should be set.
    // if there are any data packets in the FIFO, they should be flushed
    reg(CSOL) |= CSOL_FLUSHPACKET;

    // Only enable IN interrupts for endpoints 0 and 2
    reg(IIE) = (1 << 0);// | (1 << 2);
    // Only enable OUT interrupts for endpoint 0
    reg(OIE) = (1 << 0);
    // Only enable RESET common interrupt (disable start-of-frame, resume and suspend)
    reg(CIE) = INT_RESET;
}

void USB::init()
{
    Machine_Model::power_usb(0, FULL);

    // Reset USB
    //reg(CTRL) = 0;

    // Enable USB controller
    reg(CTRL) |= USBEN;

    // Enable PLL and wait for it to initialize
    reg(CTRL) |= PLLEN;
    while(!(reg(CTRL) & PLLLOCKED));

    // Do not suspend the device when USB is idle
    reg(POW) &= ~SUSPENDEN;

    reset();

    _state = USB_2_0::STATE::POWERED;

    IC::int_vector(IC::INT_USB0, &int_handler);
    IC::enable(IC::INT_USB0);

    // FIXME
    bool a = CPU::int_disabled();
    if(a)
        CPU::int_enable();
    Machine::delay(2000000);
    if(a)
        CPU::int_enable();
}

#endif
