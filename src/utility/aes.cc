//// EPOS Advanced Encryption Standard (AES) Utility Declarations
//// Adapted from https://github.com/kokke/tiny-AES128-C
//
//
///*
//    This code was taken from https://github.com/kokke/tiny-AES128-C
//    and adapted to EPOS
//
//    This is an implementation of the AES128 algorithm, specifically MODE_ECB and MODE_CBC mode.
//
//    The implementation is verified against the test vectors in:
//    National Institute of Standards and Technology Special Publication 800-38A 2001 ED
//
//    MODE_ECB-AES128
//    ----------
//
//    plain-text:
//    6bc1bee22e409f96e93d7e117393172a
//    ae2d8a571e03ac9c9eb76fac45af8e51
//    30c81c46a35ce411e5fbc1191a0a52ef
//    f69f2445df4f9b17ad2b417be66c3710
//
//    key:
//    2b7e151628aed2a6abf7158809cf4f3c
//
//    resulting cipher:
//    3ad77bb40d7a3660a89ecaf32466ef97
//    f5d3d58503b9699de785895a96fdbaaf
//    43b1cd7f598ece23881b00e3ed030688
//    7b0c785e27e8ad3f8223207104725dd4
//
//
//    NOTE: String length must be evenly divisible by 16 bytes (str_len % 16 == 0)
//    You should pad the end of the string with zeros if this is not the case.
// */
//
//#include <utility/string.h>
//#include <utility/aes.h>
//
//__BEGIN_UTIL
//
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::AES128_ECB_encrypt(const char* input, const char* key, char* output)
//{
//    // Copy input to output, and work in-memory on output
//    BlockCopy(output, input);
//    state = (state_t*)output;
//
//    Key = key;
//    KeyExpansion();
//
//    // The next function call encrypts the PlainText with the Key using AES algorithm.
//    _Cipher();
//}
//
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::AES128_ECB_decrypt(const char* input, const char* key, char *output)
//{
//    // Copy input to output, and work in-memory on output
//    BlockCopy(output, input);
//    state = (state_t*)output;
//
//    // The KeyExpansion routine must be called before encryption.
//    Key = key;
//    KeyExpansion();
//
//    Inv_Cipher();
//}
//
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::AES128_CBC_encrypt_buffer(char* output, const char* _input, int length, const char* key, const char* iv)
//{
//    char remainders = length % KEY_LENGTH; /* Remaining bytes in the last non-full block */
//
//    BlockCopy(output, _input);
//    state = (state_t*)output;
//
//    // Skip the key expansion if key is passed as 0
//    if(0 != key) {
//        Key = key;
//        KeyExpansion();
//    }
//
//    if(iv != 0)
//        Iv = (char*)iv;
//
//    int i;
//    for(i = 0; i < length; i += KEY_LENGTH) {
//        char input[KEY_LENGTH];
//        for(int j=0; (j<(int)KEY_LENGTH) && (j+i < length); j++)
//            input[j] = _input[j+i];
//        XorWithIv(input);
//        BlockCopy(output, input);
//        state = (state_t*)output;
//        _Cipher();
//        Iv = output;
//        output += KEY_LENGTH;
//    }
//
//    if(remainders) {
//        BlockCopy(output, _input+i);
//        memset(output + remainders, 0, KEY_LENGTH - remainders); /* add 0-padding */
//        state = (state_t*)output;
//        _Cipher();
//    }
//}
//
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::AES128_CBC_decrypt_buffer(char* output, const char* input, int length, const char* key, const char* iv)
//{
//    int i;
//    char remainders = length % KEY_LENGTH; /* Remaining bytes in the last non-full block */
//
//    BlockCopy(output, input);
//    state = (state_t*)output;
//
//    // Skip the key expansion if key is passed as 0
//    if(0 != key) {
//        Key = key;
//        KeyExpansion();
//    }
//
//    // If iv is passed as 0, we continue to encrypt without re-setting the Iv
//    if(iv != 0)
//        Iv = (char*)iv;
//
//    for(i = 0; i < length; i += KEY_LENGTH) {
//        BlockCopy(output, input);
//        state = (state_t*)output;
//        Inv_Cipher();
//        XorWithIv(output);
//        Iv = (char *)input;
//        input += KEY_LENGTH;
//        output += KEY_LENGTH;
//    }
//
//    if(remainders) {
//        BlockCopy(output, input);
//        memset(output+remainders, 0, KEY_LENGTH - remainders); /* add 0-padding */
//        state = (state_t*)output;
//        Inv_Cipher();
//    }
//}
//
//// This function produces Nb(Nr+1) round keys. The round keys are used in each round to decrypt the states.
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::KeyExpansion(void)
//{
//    unsigned int i, j, k;
//    char tempa[4]; // Used for the column/row operations
//
//    // The first round key is the key itself.
//    for(i = 0; i < Nk; ++i) {
//        RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
//        RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
//        RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
//        RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
//    }
//
//    // All other round keys are found from the previous round keys.
//    for(; (i < (Nb * (Nr + 1))); ++i) {
//        for(j = 0; j < 4; ++j) {
//            tempa[j]=RoundKey[(i-1) * 4 + j];
//        }
//        if (i % Nk == 0) {
//            // This function rotates the 4 bytes in a word to the left once.
//            // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]
//
//            // Function RotWord()
//            {
//                k = tempa[0];
//                tempa[0] = tempa[1];
//                tempa[1] = tempa[2];
//                tempa[2] = tempa[3];
//                tempa[3] = k;
//            }
//
//            // SubWord() is a function that takes a four-byte input word and
//            // applies the S-box to each of the four bytes to produce an output word.
//
//            // Function Subword()
//            {
//                tempa[0] = getSBoxValue(tempa[0]);
//                tempa[1] = getSBoxValue(tempa[1]);
//                tempa[2] = getSBoxValue(tempa[2]);
//                tempa[3] = getSBoxValue(tempa[3]);
//            }
//
//            tempa[0] =  tempa[0] ^ Rcon[i/Nk];
//        }
//        else if (Nk > 6 && i % Nk == 4) {
//            // Function Subword()
//            {
//                tempa[0] = getSBoxValue(tempa[0]);
//                tempa[1] = getSBoxValue(tempa[1]);
//                tempa[2] = getSBoxValue(tempa[2]);
//                tempa[3] = getSBoxValue(tempa[3]);
//            }
//        }
//        RoundKey[i * 4 + 0] = RoundKey[(i - Nk) * 4 + 0] ^ tempa[0];
//        RoundKey[i * 4 + 1] = RoundKey[(i - Nk) * 4 + 1] ^ tempa[1];
//        RoundKey[i * 4 + 2] = RoundKey[(i - Nk) * 4 + 2] ^ tempa[2];
//        RoundKey[i * 4 + 3] = RoundKey[(i - Nk) * 4 + 3] ^ tempa[3];
//    }
//}
//
//// This function adds the round key to state.
//// The round key is added to the state by an XOR function.
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::AddRoundKey(int round)
//{
//    int i,j;
//    for(i=0;i<4;++i) {
//        for(j = 0; j < 4; ++j) {
//            (*state)[i][j] ^= RoundKey[round * Nb * 4 + i * Nb + j];
//        }
//    }
//}
//
//// The SubBytes Function Substitutes the values in the
//// state matrix with values in an S-box.
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::SubBytes(void)
//{
//    int i, j;
//    for(i = 0; i < 4; ++i) {
//        for(j = 0; j < 4; ++j) {
//            (*state)[j][i] = getSBoxValue((*state)[j][i]);
//        }
//    }
//}
//
//// The ShiftRows() function shifts the rows in the state to the left.
//// Each row is shifted with different offset.
//// Offset = Row number. So the first row is not shifted.
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::ShiftRows(void)
//{
//    char temp;
//
//    // Rotate first row 1 columns to left
//    temp           = (*state)[0][1];
//    (*state)[0][1] = (*state)[1][1];
//    (*state)[1][1] = (*state)[2][1];
//    (*state)[2][1] = (*state)[3][1];
//    (*state)[3][1] = temp;
//
//    // Rotate second row 2 columns to left
//    temp           = (*state)[0][2];
//    (*state)[0][2] = (*state)[2][2];
//    (*state)[2][2] = temp;
//
//    temp       = (*state)[1][2];
//    (*state)[1][2] = (*state)[3][2];
//    (*state)[3][2] = temp;
//
//    // Rotate third row 3 columns to left
//    temp       = (*state)[0][3];
//    (*state)[0][3] = (*state)[3][3];
//    (*state)[3][3] = (*state)[2][3];
//    (*state)[2][3] = (*state)[1][3];
//    (*state)[1][3] = temp;
//}
//
//// MixColumns function mixes the columns of the state matrix
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::MixColumns(void)
//{
//    int i;
//    char Tmp,Tm,t;
//    for(i = 0; i < 4; ++i)
//    {
//        t   = (*state)[i][0];
//        Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
//        Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
//        Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
//        Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
//        Tm  = (*state)[i][3] ^ t ;        Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
//    }
//}
//
//// MixColumns function mixes the columns of the state matrix.
//// The method used to multiply may be difficult to understand for the inexperienced.
//// Please use the references to gain more information.
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::InvMixColumns(void)
//{
//    int i;
//    char a,b,c,d;
//    for(i=0;i<4;++i)
//    {
//        a = (*state)[i][0];
//        b = (*state)[i][1];
//        c = (*state)[i][2];
//        d = (*state)[i][3];
//
//        (*state)[i][0] = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
//        (*state)[i][1] = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
//        (*state)[i][2] = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
//        (*state)[i][3] = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
//    }
//}
//
//
//// The SubBytes Function Substitutes the values in the
//// state matrix with values in an S-box.
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::InvSubBytes(void)
//{
//    int i,j;
//    for(i=0;i<4;++i) {
//        for(j=0;j<4;++j) {
//            (*state)[j][i] = getSBoxInvert((*state)[j][i]);
//        }
//    }
//}
//
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::InvShiftRows(void)
//{
//    char temp;
//
//    // Rotate first row 1 columns to right
//    temp=(*state)[3][1];
//    (*state)[3][1]=(*state)[2][1];
//    (*state)[2][1]=(*state)[1][1];
//    (*state)[1][1]=(*state)[0][1];
//    (*state)[0][1]=temp;
//
//    // Rotate second row 2 columns to right
//    temp=(*state)[0][2];
//    (*state)[0][2]=(*state)[2][2];
//    (*state)[2][2]=temp;
//
//    temp=(*state)[1][2];
//    (*state)[1][2]=(*state)[3][2];
//    (*state)[3][2]=temp;
//
//    // Rotate third row 3 columns to right
//    temp=(*state)[0][3];
//    (*state)[0][3]=(*state)[1][3];
//    (*state)[1][3]=(*state)[2][3];
//    (*state)[2][3]=(*state)[3][3];
//    (*state)[3][3]=temp;
//}
//
//
//// _Cipher is the main function that encrypts the PlainText.
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::_Cipher(void)
//{
//    char round = 0;
//
//    // Add the First round key to the state before starting the rounds.
//    AddRoundKey(0);
//
//    // There will be Nr rounds.
//    // The first Nr-1 rounds are identical.
//    // These Nr-1 rounds are executed in the loop below.
//    for(round = 1; round < Nr; ++round)
//    {
//        SubBytes();
//        ShiftRows();
//        MixColumns();
//        AddRoundKey(round);
//    }
//
//    // The last round is given below.
//    // The MixColumns function is not here in the last round.
//    SubBytes();
//    ShiftRows();
//    AddRoundKey(Nr);
//}
//
//template<unsigned int KEY_LENGTH>
//void AES<KEY_LENGTH>::Inv_Cipher(void)
//{
//    char round=0;
//
//    // Add the First round key to the state before starting the rounds.
//    AddRoundKey(Nr);
//
//    // There will be Nr rounds.
//    // The first Nr-1 rounds are identical.
//    // These Nr-1 rounds are executed in the loop below.
//    for(round=Nr-1;round>0;round--)
//    {
//        InvShiftRows();
//        InvSubBytes();
//        AddRoundKey(round);
//        InvMixColumns();
//    }
//
//    // The last round is given below.
//    // The MixColumns function is not here in the last round.
//    InvShiftRows();
//    InvSubBytes();
//    AddRoundKey(0);
//}
//
//__END_UTIL
