
/**********************
 *      C Version     *
 **********************/


#define U8_TO_U32_LE(r, x, i)        \
{                                    \
  r  = ( uint )( x[ i + 0 ] ) <<  0; \
  r |= ( uint )( x[ i + 1 ] ) <<  8; \
  r |= ( uint )( x[ i + 2 ] ) << 16; \
  r |= ( uint )( x[ i + 3 ] ) << 24; \
}


#define U32_TO_U8_LE(r, x, i)       \
{                                   \
  r[ i + 0 ] = ( x >>  0 ) & 0xFF;  \
  r[ i + 1 ] = ( x >>  8 ) & 0xFF;  \
  r[ i + 2 ] = ( x >> 16 ) & 0xFF;  \
  r[ i + 3 ] = ( x >> 24 ) & 0xFF;  \
}


#define ROUND1(a, b, c, d)  \
{                           \
  t0 = t0 ^ RK[ a ];        \
  t1 = t1 ^ RK[ b ];        \
  t2 = t2 ^ RK[ c ];        \
  t3 = t3 ^ RK[ d ];        \
}


#define ROUND2(a, b, c, d)                          \
{                                                   \
    t4 = ( T0[ ( t0 >>  0 ) & 0xFF ] ) ^            \
         ( T1[ ( t1 >>  8 ) & 0xFF ] ) ^            \
         ( T2[ ( t2 >> 16 ) & 0xFF ] ) ^            \
         ( T3[ ( t3 >> 24 ) & 0xFF ] ) ^ RK[ a ];   \
    t5 = ( T0[ ( t1 >>  0 ) & 0xFF ] ) ^            \
         ( T1[ ( t2 >>  8 ) & 0xFF ] ) ^            \
         ( T2[ ( t3 >> 16 ) & 0xFF ] ) ^            \
         ( T3[ ( t0 >> 24 ) & 0xFF ] ) ^ RK[ b ];   \
    t6 = ( T0[ ( t2 >>  0 ) & 0xFF ] ) ^            \
         ( T1[ ( t3 >>  8 ) & 0xFF ] ) ^            \
         ( T2[ ( t0 >> 16 ) & 0xFF ] ) ^            \
         ( T3[ ( t1 >> 24 ) & 0xFF ] ) ^ RK[ c ];   \
    t7 = ( T0[ ( t3 >>  0 ) & 0xFF ] ) ^            \
         ( T1[ ( t0 >>  8 ) & 0xFF ] ) ^            \
         ( T2[ ( t1 >> 16 ) & 0xFF ] ) ^            \
         ( T3[ ( t2 >> 24 ) & 0xFF ] ) ^ RK[ d ];   \
                                                    \
    t0 = t4;                                        \
    t1 = t5;                                        \
    t2 = t6;                                        \
    t3 = t7;                                        \
}


#define ROUND2_INV(a, b, c, d)          \
{                                       \
  t4 = ( U0[ ( t0 >>  0 ) & 0xFF ] ) ^  \
       ( U1[ ( t3 >>  8 ) & 0xFF ] ) ^  \
       ( U2[ ( t2 >> 16 ) & 0xFF ] ) ^  \
       ( U3[ ( t1 >> 24 ) & 0xFF ] );   \
  t5 = ( U0[ ( t1 >>  0 ) & 0xFF ] ) ^  \
       ( U1[ ( t0 >>  8 ) & 0xFF ] ) ^  \
       ( U2[ ( t3 >> 16 ) & 0xFF ] ) ^  \
       ( U3[ ( t2 >> 24 ) & 0xFF ] );   \
  t6 = ( U0[ ( t2 >>  0 ) & 0xFF ] ) ^  \
       ( U1[ ( t1 >>  8 ) & 0xFF ] ) ^  \
       ( U2[ ( t0 >> 16 ) & 0xFF ] ) ^  \
       ( U3[ ( t3 >> 24 ) & 0xFF ] );   \
  t7 = ( U0[ ( t3 >>  0 ) & 0xFF ] ) ^  \
       ( U1[ ( t2 >>  8 ) & 0xFF ] ) ^  \
       ( U2[ ( t1 >> 16 ) & 0xFF ] ) ^  \
       ( U3[ ( t0 >> 24 ) & 0xFF ] );   \
                                        \
  t0 = t4 ^ RK[a];                      \
  t1 = t5 ^ RK[b];                      \
  t2 = t6 ^ RK[c];                      \
  t3 = t7 ^ RK[d];                      \
}


#define ROUND3(a,b,c,d)                                               \
{                                                                     \
    t4 = ( aes_sbox[ ( t0 >>  0 ) & 0xFF ] & 0x000000FF ) ^           \
         ( aes_sbox[ ( t1 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^           \
         ( aes_sbox[ ( t2 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^           \
         ( aes_sbox[ ( t3 >> 24 ) & 0xFF ] & 0xFF000000 ) ^ RK[ a ];  \
    t5 = ( aes_sbox[ ( t1 >>  0 ) & 0xFF ] & 0x000000FF ) ^           \
         ( aes_sbox[ ( t2 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^           \
         ( aes_sbox[ ( t3 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^           \
         ( aes_sbox[ ( t0 >> 24 ) & 0xFF ] & 0xFF000000 ) ^ RK[ b ];  \
    t6 = ( aes_sbox[ ( t2 >>  0 ) & 0xFF ] & 0x000000FF ) ^           \
         ( aes_sbox[ ( t3 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^           \
         ( aes_sbox[ ( t0 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^           \
         ( aes_sbox[ ( t1 >> 24 ) & 0xFF ] & 0xFF000000 ) ^ RK[ c ];  \
    t7 = ( aes_sbox[ ( t3 >>  0 ) & 0xFF ] & 0x000000FF ) ^           \
         ( aes_sbox[ ( t0 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^           \
         ( aes_sbox[ ( t1 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^           \
         ( aes_sbox[ ( t2 >> 24 ) & 0xFF ] & 0xFF000000 ) ^ RK[ d ];  \
}


#define ROUND3_INV(a, b, c, d)                                           \
{                                                                        \
    t4 = ( aes_inv_sbox[ ( t0 >>  0 ) & 0xFF ] & 0x000000FF ) ^          \
         ( aes_inv_sbox[ ( t3 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^          \
         ( aes_inv_sbox[ ( t2 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^          \
         ( aes_inv_sbox[ ( t1 >> 24 ) & 0xFF ] & 0xFF000000 ) ^ RK[ a ]; \
    t5 = ( aes_inv_sbox[ ( t1 >>  0 ) & 0xFF ] & 0x000000FF ) ^          \
         ( aes_inv_sbox[ ( t0 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^          \
         ( aes_inv_sbox[ ( t3 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^          \
         ( aes_inv_sbox[ ( t2 >> 24 ) & 0xFF ] & 0xFF000000 ) ^ RK[ b ]; \
    t6 = ( aes_inv_sbox[ ( t2 >>  0 ) & 0xFF ] & 0x000000FF ) ^          \
         ( aes_inv_sbox[ ( t1 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^          \
         ( aes_inv_sbox[ ( t0 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^          \
         ( aes_inv_sbox[ ( t3 >> 24 ) & 0xFF ] & 0xFF000000 ) ^ RK[ c ]; \
    t7 = ( aes_inv_sbox[ ( t3 >>  0 ) & 0xFF ] & 0x000000FF ) ^          \
         ( aes_inv_sbox[ ( t2 >>  8 ) & 0xFF ] & 0x0000FF00 ) ^          \
         ( aes_inv_sbox[ ( t1 >> 16 ) & 0xFF ] & 0x00FF0000 ) ^          \
         ( aes_inv_sbox[ ( t0 >> 24 ) & 0xFF ] & 0xFF000000 ) ^ RK[ d ]; \
}


uint T0[] ={ 0xA56363C6, 0x847C7CF8, 0x997777EE, 0x8D7B7BF6,
             0x0DF2F2FF, 0xBD6B6BD6, 0xB16F6FDE, 0x54C5C591,
             0x50303060, 0x03010102, 0xA96767CE, 0x7D2B2B56,
             0x19FEFEE7, 0x62D7D7B5, 0xE6ABAB4D, 0x9A7676EC,
             0x45CACA8F, 0x9D82821F, 0x40C9C989, 0x877D7DFA,
             0x15FAFAEF, 0xEB5959B2, 0xC947478E, 0x0BF0F0FB,
             0xECADAD41, 0x67D4D4B3, 0xFDA2A25F, 0xEAAFAF45,
             0xBF9C9C23, 0xF7A4A453, 0x967272E4, 0x5BC0C09B,
             0xC2B7B775, 0x1CFDFDE1, 0xAE93933D, 0x6A26264C,
             0x5A36366C, 0x413F3F7E, 0x02F7F7F5, 0x4FCCCC83,
             0x5C343468, 0xF4A5A551, 0x34E5E5D1, 0x08F1F1F9,
             0x937171E2, 0x73D8D8AB, 0x53313162, 0x3F15152A,
             0x0C040408, 0x52C7C795, 0x65232346, 0x5EC3C39D,
             0x28181830, 0xA1969637, 0x0F05050A, 0xB59A9A2F,
             0x0907070E, 0x36121224, 0x9B80801B, 0x3DE2E2DF,
             0x26EBEBCD, 0x6927274E, 0xCDB2B27F, 0x9F7575EA,
             0x1B090912, 0x9E83831D, 0x742C2C58, 0x2E1A1A34,
             0x2D1B1B36, 0xB26E6EDC, 0xEE5A5AB4, 0xFBA0A05B,
             0xF65252A4, 0x4D3B3B76, 0x61D6D6B7, 0xCEB3B37D,
             0x7B292952, 0x3EE3E3DD, 0x712F2F5E, 0x97848413,
             0xF55353A6, 0x68D1D1B9, 0x00000000, 0x2CEDEDC1,
             0x60202040, 0x1FFCFCE3, 0xC8B1B179, 0xED5B5BB6,
             0xBE6A6AD4, 0x46CBCB8D, 0xD9BEBE67, 0x4B393972,
             0xDE4A4A94, 0xD44C4C98, 0xE85858B0, 0x4ACFCF85,
             0x6BD0D0BB, 0x2AEFEFC5, 0xE5AAAA4F, 0x16FBFBED,
             0xC5434386, 0xD74D4D9A, 0x55333366, 0x94858511,
             0xCF45458A, 0x10F9F9E9, 0x06020204, 0x817F7FFE,
             0xF05050A0, 0x443C3C78, 0xBA9F9F25, 0xE3A8A84B,
             0xF35151A2, 0xFEA3A35D, 0xC0404080, 0x8A8F8F05,
             0xAD92923F, 0xBC9D9D21, 0x48383870, 0x04F5F5F1,
             0xDFBCBC63, 0xC1B6B677, 0x75DADAAF, 0x63212142,
             0x30101020, 0x1AFFFFE5, 0x0EF3F3FD, 0x6DD2D2BF,
             0x4CCDCD81, 0x140C0C18, 0x35131326, 0x2FECECC3,
             0xE15F5FBE, 0xA2979735, 0xCC444488, 0x3917172E,
             0x57C4C493, 0xF2A7A755, 0x827E7EFC, 0x473D3D7A,
             0xAC6464C8, 0xE75D5DBA, 0x2B191932, 0x957373E6,
             0xA06060C0, 0x98818119, 0xD14F4F9E, 0x7FDCDCA3,
             0x66222244, 0x7E2A2A54, 0xAB90903B, 0x8388880B,
             0xCA46468C, 0x29EEEEC7, 0xD3B8B86B, 0x3C141428,
             0x79DEDEA7, 0xE25E5EBC, 0x1D0B0B16, 0x76DBDBAD,
             0x3BE0E0DB, 0x56323264, 0x4E3A3A74, 0x1E0A0A14,
             0xDB494992, 0x0A06060C, 0x6C242448, 0xE45C5CB8,
             0x5DC2C29F, 0x6ED3D3BD, 0xEFACAC43, 0xA66262C4,
             0xA8919139, 0xA4959531, 0x37E4E4D3, 0x8B7979F2,
             0x32E7E7D5, 0x43C8C88B, 0x5937376E, 0xB76D6DDA,
             0x8C8D8D01, 0x64D5D5B1, 0xD24E4E9C, 0xE0A9A949,
             0xB46C6CD8, 0xFA5656AC, 0x07F4F4F3, 0x25EAEACF,
             0xAF6565CA, 0x8E7A7AF4, 0xE9AEAE47, 0x18080810,
             0xD5BABA6F, 0x887878F0, 0x6F25254A, 0x722E2E5C,
             0x241C1C38, 0xF1A6A657, 0xC7B4B473, 0x51C6C697,
             0x23E8E8CB, 0x7CDDDDA1, 0x9C7474E8, 0x211F1F3E,
             0xDD4B4B96, 0xDCBDBD61, 0x868B8B0D, 0x858A8A0F,
             0x907070E0, 0x423E3E7C, 0xC4B5B571, 0xAA6666CC,
             0xD8484890, 0x05030306, 0x01F6F6F7, 0x120E0E1C,
             0xA36161C2, 0x5F35356A, 0xF95757AE, 0xD0B9B969,
             0x91868617, 0x58C1C199, 0x271D1D3A, 0xB99E9E27,
             0x38E1E1D9, 0x13F8F8EB, 0xB398982B, 0x33111122,
             0xBB6969D2, 0x70D9D9A9, 0x898E8E07, 0xA7949433,
             0xB69B9B2D, 0x221E1E3C, 0x92878715, 0x20E9E9C9,
             0x49CECE87, 0xFF5555AA, 0x78282850, 0x7ADFDFA5,
             0x8F8C8C03, 0xF8A1A159, 0x80898909, 0x170D0D1A,
             0xDABFBF65, 0x31E6E6D7, 0xC6424284, 0xB86868D0,
             0xC3414182, 0xB0999929, 0x772D2D5A, 0x110F0F1E,
             0xCBB0B07B, 0xFC5454A8, 0xD6BBBB6D, 0x3A16162C };

uint T1[] ={ 0x6363C6A5, 0x7C7CF884, 0x7777EE99, 0x7B7BF68D,
             0xF2F2FF0D, 0x6B6BD6BD, 0x6F6FDEB1, 0xC5C59154,
             0x30306050, 0x01010203, 0x6767CEA9, 0x2B2B567D,
             0xFEFEE719, 0xD7D7B562, 0xABAB4DE6, 0x7676EC9A,
             0xCACA8F45, 0x82821F9D, 0xC9C98940, 0x7D7DFA87,
             0xFAFAEF15, 0x5959B2EB, 0x47478EC9, 0xF0F0FB0B,
             0xADAD41EC, 0xD4D4B367, 0xA2A25FFD, 0xAFAF45EA,
             0x9C9C23BF, 0xA4A453F7, 0x7272E496, 0xC0C09B5B,
             0xB7B775C2, 0xFDFDE11C, 0x93933DAE, 0x26264C6A,
             0x36366C5A, 0x3F3F7E41, 0xF7F7F502, 0xCCCC834F,
             0x3434685C, 0xA5A551F4, 0xE5E5D134, 0xF1F1F908,
             0x7171E293, 0xD8D8AB73, 0x31316253, 0x15152A3F,
             0x0404080C, 0xC7C79552, 0x23234665, 0xC3C39D5E,
             0x18183028, 0x969637A1, 0x05050A0F, 0x9A9A2FB5,
             0x07070E09, 0x12122436, 0x80801B9B, 0xE2E2DF3D,
             0xEBEBCD26, 0x27274E69, 0xB2B27FCD, 0x7575EA9F,
             0x0909121B, 0x83831D9E, 0x2C2C5874, 0x1A1A342E,
             0x1B1B362D, 0x6E6EDCB2, 0x5A5AB4EE, 0xA0A05BFB,
             0x5252A4F6, 0x3B3B764D, 0xD6D6B761, 0xB3B37DCE,
             0x2929527B, 0xE3E3DD3E, 0x2F2F5E71, 0x84841397,
             0x5353A6F5, 0xD1D1B968, 0x00000000, 0xEDEDC12C,
             0x20204060, 0xFCFCE31F, 0xB1B179C8, 0x5B5BB6ED,
             0x6A6AD4BE, 0xCBCB8D46, 0xBEBE67D9, 0x3939724B,
             0x4A4A94DE, 0x4C4C98D4, 0x5858B0E8, 0xCFCF854A,
             0xD0D0BB6B, 0xEFEFC52A, 0xAAAA4FE5, 0xFBFBED16,
             0x434386C5, 0x4D4D9AD7, 0x33336655, 0x85851194,
             0x45458ACF, 0xF9F9E910, 0x02020406, 0x7F7FFE81,
             0x5050A0F0, 0x3C3C7844, 0x9F9F25BA, 0xA8A84BE3,
             0x5151A2F3, 0xA3A35DFE, 0x404080C0, 0x8F8F058A,
             0x92923FAD, 0x9D9D21BC, 0x38387048, 0xF5F5F104,
             0xBCBC63DF, 0xB6B677C1, 0xDADAAF75, 0x21214263,
             0x10102030, 0xFFFFE51A, 0xF3F3FD0E, 0xD2D2BF6D,
             0xCDCD814C, 0x0C0C1814, 0x13132635, 0xECECC32F,
             0x5F5FBEE1, 0x979735A2, 0x444488CC, 0x17172E39,
             0xC4C49357, 0xA7A755F2, 0x7E7EFC82, 0x3D3D7A47,
             0x6464C8AC, 0x5D5DBAE7, 0x1919322B, 0x7373E695,
             0x6060C0A0, 0x81811998, 0x4F4F9ED1, 0xDCDCA37F,
             0x22224466, 0x2A2A547E, 0x90903BAB, 0x88880B83,
             0x46468CCA, 0xEEEEC729, 0xB8B86BD3, 0x1414283C,
             0xDEDEA779, 0x5E5EBCE2, 0x0B0B161D, 0xDBDBAD76,
             0xE0E0DB3B, 0x32326456, 0x3A3A744E, 0x0A0A141E,
             0x494992DB, 0x06060C0A, 0x2424486C, 0x5C5CB8E4,
             0xC2C29F5D, 0xD3D3BD6E, 0xACAC43EF, 0x6262C4A6,
             0x919139A8, 0x959531A4, 0xE4E4D337, 0x7979F28B,
             0xE7E7D532, 0xC8C88B43, 0x37376E59, 0x6D6DDAB7,
             0x8D8D018C, 0xD5D5B164, 0x4E4E9CD2, 0xA9A949E0,
             0x6C6CD8B4, 0x5656ACFA, 0xF4F4F307, 0xEAEACF25,
             0x6565CAAF, 0x7A7AF48E, 0xAEAE47E9, 0x08081018,
             0xBABA6FD5, 0x7878F088, 0x25254A6F, 0x2E2E5C72,
             0x1C1C3824, 0xA6A657F1, 0xB4B473C7, 0xC6C69751,
             0xE8E8CB23, 0xDDDDA17C, 0x7474E89C, 0x1F1F3E21,
             0x4B4B96DD, 0xBDBD61DC, 0x8B8B0D86, 0x8A8A0F85,
             0x7070E090, 0x3E3E7C42, 0xB5B571C4, 0x6666CCAA,
             0x484890D8, 0x03030605, 0xF6F6F701, 0x0E0E1C12,
             0x6161C2A3, 0x35356A5F, 0x5757AEF9, 0xB9B969D0,
             0x86861791, 0xC1C19958, 0x1D1D3A27, 0x9E9E27B9,
             0xE1E1D938, 0xF8F8EB13, 0x98982BB3, 0x11112233,
             0x6969D2BB, 0xD9D9A970, 0x8E8E0789, 0x949433A7,
             0x9B9B2DB6, 0x1E1E3C22, 0x87871592, 0xE9E9C920,
             0xCECE8749, 0x5555AAFF, 0x28285078, 0xDFDFA57A,
             0x8C8C038F, 0xA1A159F8, 0x89890980, 0x0D0D1A17,
             0xBFBF65DA, 0xE6E6D731, 0x424284C6, 0x6868D0B8,
             0x414182C3, 0x999929B0, 0x2D2D5A77, 0x0F0F1E11,
             0xB0B07BCB, 0x5454A8FC, 0xBBBB6DD6, 0x16162C3A };

uint T2[] ={ 0x63C6A563, 0x7CF8847C, 0x77EE9977, 0x7BF68D7B,
             0xF2FF0DF2, 0x6BD6BD6B, 0x6FDEB16F, 0xC59154C5,
             0x30605030, 0x01020301, 0x67CEA967, 0x2B567D2B,
             0xFEE719FE, 0xD7B562D7, 0xAB4DE6AB, 0x76EC9A76,
             0xCA8F45CA, 0x821F9D82, 0xC98940C9, 0x7DFA877D,
             0xFAEF15FA, 0x59B2EB59, 0x478EC947, 0xF0FB0BF0,
             0xAD41ECAD, 0xD4B367D4, 0xA25FFDA2, 0xAF45EAAF,
             0x9C23BF9C, 0xA453F7A4, 0x72E49672, 0xC09B5BC0,
             0xB775C2B7, 0xFDE11CFD, 0x933DAE93, 0x264C6A26,
             0x366C5A36, 0x3F7E413F, 0xF7F502F7, 0xCC834FCC,
             0x34685C34, 0xA551F4A5, 0xE5D134E5, 0xF1F908F1,
             0x71E29371, 0xD8AB73D8, 0x31625331, 0x152A3F15,
             0x04080C04, 0xC79552C7, 0x23466523, 0xC39D5EC3,
             0x18302818, 0x9637A196, 0x050A0F05, 0x9A2FB59A,
             0x070E0907, 0x12243612, 0x801B9B80, 0xE2DF3DE2,
             0xEBCD26EB, 0x274E6927, 0xB27FCDB2, 0x75EA9F75,
             0x09121B09, 0x831D9E83, 0x2C58742C, 0x1A342E1A,
             0x1B362D1B, 0x6EDCB26E, 0x5AB4EE5A, 0xA05BFBA0,
             0x52A4F652, 0x3B764D3B, 0xD6B761D6, 0xB37DCEB3,
             0x29527B29, 0xE3DD3EE3, 0x2F5E712F, 0x84139784,
             0x53A6F553, 0xD1B968D1, 0x00000000, 0xEDC12CED,
             0x20406020, 0xFCE31FFC, 0xB179C8B1, 0x5BB6ED5B,
             0x6AD4BE6A, 0xCB8D46CB, 0xBE67D9BE, 0x39724B39,
             0x4A94DE4A, 0x4C98D44C, 0x58B0E858, 0xCF854ACF,
             0xD0BB6BD0, 0xEFC52AEF, 0xAA4FE5AA, 0xFBED16FB,
             0x4386C543, 0x4D9AD74D, 0x33665533, 0x85119485,
             0x458ACF45, 0xF9E910F9, 0x02040602, 0x7FFE817F,
             0x50A0F050, 0x3C78443C, 0x9F25BA9F, 0xA84BE3A8,
             0x51A2F351, 0xA35DFEA3, 0x4080C040, 0x8F058A8F,
             0x923FAD92, 0x9D21BC9D, 0x38704838, 0xF5F104F5,
             0xBC63DFBC, 0xB677C1B6, 0xDAAF75DA, 0x21426321,
             0x10203010, 0xFFE51AFF, 0xF3FD0EF3, 0xD2BF6DD2,
             0xCD814CCD, 0x0C18140C, 0x13263513, 0xECC32FEC,
             0x5FBEE15F, 0x9735A297, 0x4488CC44, 0x172E3917,
             0xC49357C4, 0xA755F2A7, 0x7EFC827E, 0x3D7A473D,
             0x64C8AC64, 0x5DBAE75D, 0x19322B19, 0x73E69573,
             0x60C0A060, 0x81199881, 0x4F9ED14F, 0xDCA37FDC,
             0x22446622, 0x2A547E2A, 0x903BAB90, 0x880B8388,
             0x468CCA46, 0xEEC729EE, 0xB86BD3B8, 0x14283C14,
             0xDEA779DE, 0x5EBCE25E, 0x0B161D0B, 0xDBAD76DB,
             0xE0DB3BE0, 0x32645632, 0x3A744E3A, 0x0A141E0A,
             0x4992DB49, 0x060C0A06, 0x24486C24, 0x5CB8E45C,
             0xC29F5DC2, 0xD3BD6ED3, 0xAC43EFAC, 0x62C4A662,
             0x9139A891, 0x9531A495, 0xE4D337E4, 0x79F28B79,
             0xE7D532E7, 0xC88B43C8, 0x376E5937, 0x6DDAB76D,
             0x8D018C8D, 0xD5B164D5, 0x4E9CD24E, 0xA949E0A9,
             0x6CD8B46C, 0x56ACFA56, 0xF4F307F4, 0xEACF25EA,
             0x65CAAF65, 0x7AF48E7A, 0xAE47E9AE, 0x08101808,
             0xBA6FD5BA, 0x78F08878, 0x254A6F25, 0x2E5C722E,
             0x1C38241C, 0xA657F1A6, 0xB473C7B4, 0xC69751C6,
             0xE8CB23E8, 0xDDA17CDD, 0x74E89C74, 0x1F3E211F,
             0x4B96DD4B, 0xBD61DCBD, 0x8B0D868B, 0x8A0F858A,
             0x70E09070, 0x3E7C423E, 0xB571C4B5, 0x66CCAA66,
             0x4890D848, 0x03060503, 0xF6F701F6, 0x0E1C120E,
             0x61C2A361, 0x356A5F35, 0x57AEF957, 0xB969D0B9,
             0x86179186, 0xC19958C1, 0x1D3A271D, 0x9E27B99E,
             0xE1D938E1, 0xF8EB13F8, 0x982BB398, 0x11223311,
             0x69D2BB69, 0xD9A970D9, 0x8E07898E, 0x9433A794,
             0x9B2DB69B, 0x1E3C221E, 0x87159287, 0xE9C920E9,
             0xCE8749CE, 0x55AAFF55, 0x28507828, 0xDFA57ADF,
             0x8C038F8C, 0xA159F8A1, 0x89098089, 0x0D1A170D,
             0xBF65DABF, 0xE6D731E6, 0x4284C642, 0x68D0B868,
             0x4182C341, 0x9929B099, 0x2D5A772D, 0x0F1E110F,
             0xB07BCBB0, 0x54A8FC54, 0xBB6DD6BB, 0x162C3A16 };

uint T3[] ={ 0xC6A56363, 0xF8847C7C, 0xEE997777, 0xF68D7B7B,
             0xFF0DF2F2, 0xD6BD6B6B, 0xDEB16F6F, 0x9154C5C5,
             0x60503030, 0x02030101, 0xCEA96767, 0x567D2B2B,
             0xE719FEFE, 0xB562D7D7, 0x4DE6ABAB, 0xEC9A7676,
             0x8F45CACA, 0x1F9D8282, 0x8940C9C9, 0xFA877D7D,
             0xEF15FAFA, 0xB2EB5959, 0x8EC94747, 0xFB0BF0F0,
             0x41ECADAD, 0xB367D4D4, 0x5FFDA2A2, 0x45EAAFAF,
             0x23BF9C9C, 0x53F7A4A4, 0xE4967272, 0x9B5BC0C0,
             0x75C2B7B7, 0xE11CFDFD, 0x3DAE9393, 0x4C6A2626,
             0x6C5A3636, 0x7E413F3F, 0xF502F7F7, 0x834FCCCC,
             0x685C3434, 0x51F4A5A5, 0xD134E5E5, 0xF908F1F1,
             0xE2937171, 0xAB73D8D8, 0x62533131, 0x2A3F1515,
             0x080C0404, 0x9552C7C7, 0x46652323, 0x9D5EC3C3,
             0x30281818, 0x37A19696, 0x0A0F0505, 0x2FB59A9A,
             0x0E090707, 0x24361212, 0x1B9B8080, 0xDF3DE2E2,
             0xCD26EBEB, 0x4E692727, 0x7FCDB2B2, 0xEA9F7575,
             0x121B0909, 0x1D9E8383, 0x58742C2C, 0x342E1A1A,
             0x362D1B1B, 0xDCB26E6E, 0xB4EE5A5A, 0x5BFBA0A0,
             0xA4F65252, 0x764D3B3B, 0xB761D6D6, 0x7DCEB3B3,
             0x527B2929, 0xDD3EE3E3, 0x5E712F2F, 0x13978484,
             0xA6F55353, 0xB968D1D1, 0x00000000, 0xC12CEDED,
             0x40602020, 0xE31FFCFC, 0x79C8B1B1, 0xB6ED5B5B,
             0xD4BE6A6A, 0x8D46CBCB, 0x67D9BEBE, 0x724B3939,
             0x94DE4A4A, 0x98D44C4C, 0xB0E85858, 0x854ACFCF,
             0xBB6BD0D0, 0xC52AEFEF, 0x4FE5AAAA, 0xED16FBFB,
             0x86C54343, 0x9AD74D4D, 0x66553333, 0x11948585,
             0x8ACF4545, 0xE910F9F9, 0x04060202, 0xFE817F7F,
             0xA0F05050, 0x78443C3C, 0x25BA9F9F, 0x4BE3A8A8,
             0xA2F35151, 0x5DFEA3A3, 0x80C04040, 0x058A8F8F,
             0x3FAD9292, 0x21BC9D9D, 0x70483838, 0xF104F5F5,
             0x63DFBCBC, 0x77C1B6B6, 0xAF75DADA, 0x42632121,
             0x20301010, 0xE51AFFFF, 0xFD0EF3F3, 0xBF6DD2D2,
             0x814CCDCD, 0x18140C0C, 0x26351313, 0xC32FECEC,
             0xBEE15F5F, 0x35A29797, 0x88CC4444, 0x2E391717,
             0x9357C4C4, 0x55F2A7A7, 0xFC827E7E, 0x7A473D3D,
             0xC8AC6464, 0xBAE75D5D, 0x322B1919, 0xE6957373,
             0xC0A06060, 0x19988181, 0x9ED14F4F, 0xA37FDCDC,
             0x44662222, 0x547E2A2A, 0x3BAB9090, 0x0B838888,
             0x8CCA4646, 0xC729EEEE, 0x6BD3B8B8, 0x283C1414,
             0xA779DEDE, 0xBCE25E5E, 0x161D0B0B, 0xAD76DBDB,
             0xDB3BE0E0, 0x64563232, 0x744E3A3A, 0x141E0A0A,
             0x92DB4949, 0x0C0A0606, 0x486C2424, 0xB8E45C5C,
             0x9F5DC2C2, 0xBD6ED3D3, 0x43EFACAC, 0xC4A66262,
             0x39A89191, 0x31A49595, 0xD337E4E4, 0xF28B7979,
             0xD532E7E7, 0x8B43C8C8, 0x6E593737, 0xDAB76D6D,
             0x018C8D8D, 0xB164D5D5, 0x9CD24E4E, 0x49E0A9A9,
             0xD8B46C6C, 0xACFA5656, 0xF307F4F4, 0xCF25EAEA,
             0xCAAF6565, 0xF48E7A7A, 0x47E9AEAE, 0x10180808,
             0x6FD5BABA, 0xF0887878, 0x4A6F2525, 0x5C722E2E,
             0x38241C1C, 0x57F1A6A6, 0x73C7B4B4, 0x9751C6C6,
             0xCB23E8E8, 0xA17CDDDD, 0xE89C7474, 0x3E211F1F,
             0x96DD4B4B, 0x61DCBDBD, 0x0D868B8B, 0x0F858A8A,
             0xE0907070, 0x7C423E3E, 0x71C4B5B5, 0xCCAA6666,
             0x90D84848, 0x06050303, 0xF701F6F6, 0x1C120E0E,
             0xC2A36161, 0x6A5F3535, 0xAEF95757, 0x69D0B9B9,
             0x17918686, 0x9958C1C1, 0x3A271D1D, 0x27B99E9E,
             0xD938E1E1, 0xEB13F8F8, 0x2BB39898, 0x22331111,
             0xD2BB6969, 0xA970D9D9, 0x07898E8E, 0x33A79494,
             0x2DB69B9B, 0x3C221E1E, 0x15928787, 0xC920E9E9,
             0x8749CECE, 0xAAFF5555, 0x50782828, 0xA57ADFDF,
             0x038F8C8C, 0x59F8A1A1, 0x09808989, 0x1A170D0D,
             0x65DABFBF, 0xD731E6E6, 0x84C64242, 0xD0B86868,
             0x82C34141, 0x29B09999, 0x5A772D2D, 0x1E110F0F,
             0x7BCBB0B0, 0xA8FC5454, 0x6DD6BBBB, 0x2C3A1616 };

uint U0[] = {0x50a7f451, 0x5365417e, 0xc3a4171a, 0x965e273a, 
             0xcb6bab3b, 0xf1459d1f, 0xab58faac, 0x9303e34b, 
             0x55fa3020, 0xf66d76ad, 0x9176cc88, 0x254c02f5, 
             0xfcd7e54f, 0xd7cb2ac5, 0x80443526, 0x8fa362b5, 
             0x495ab1de, 0x671bba25, 0x980eea45, 0xe1c0fe5d, 
             0x2752fc3, 0x12f04c81, 0xa397468d, 0xc6f9d36b, 
             0xe75f8f03, 0x959c9215, 0xeb7a6dbf, 0xda595295, 
             0x2d83bed4, 0xd3217458, 0x2969e049, 0x44c8c98e, 
             0x6a89c275, 0x78798ef4, 0x6b3e5899, 0xdd71b927, 
             0xb64fe1be, 0x17ad88f0, 0x66ac20c9, 0xb43ace7d, 
             0x184adf63, 0x82311ae5, 0x60335197, 0x457f5362, 
             0xe07764b1, 0x84ae6bbb, 0x1ca081fe, 0x942b08f9, 
             0x58684870, 0x19fd458f, 0x876cde94, 0xb7f87b52, 
             0x23d373ab, 0xe2024b72, 0x578f1fe3, 0x2aab5566, 
             0x728ebb2, 0x3c2b52f, 0x9a7bc586, 0xa50837d3, 
             0xf2872830, 0xb2a5bf23, 0xba6a0302, 0x5c8216ed, 
             0x2b1ccf8a, 0x92b479a7, 0xf0f207f3, 0xa1e2694e, 
             0xcdf4da65, 0xd5be0506, 0x1f6234d1, 0x8afea6c4, 
             0x9d532e34, 0xa055f3a2, 0x32e18a05, 0x75ebf6a4, 
             0x39ec830b, 0xaaef6040, 0x69f715e, 0x51106ebd, 
             0xf98a213e, 0x3d06dd96, 0xae053edd, 0x46bde64d, 
             0xb58d5491, 0x55dc471, 0x6fd40604, 0xff155060, 
             0x24fb9819, 0x97e9bdd6, 0xcc434089, 0x779ed967, 
             0xbd42e8b0, 0x888b8907, 0x385b19e7, 0xdbeec879, 
             0x470a7ca1, 0xe90f427c, 0xc91e84f8, 0x0, 
             0x83868009, 0x48ed2b32, 0xac70111e, 0x4e725a6c, 
             0xfbff0efd, 0x5638850f, 0x1ed5ae3d, 0x27392d36, 
             0x64d90f0a, 0x21a65c68, 0xd1545b9b, 0x3a2e3624, 
             0xb1670a0c, 0xfe75793, 0xd296eeb4, 0x9e919b1b, 
             0x4fc5c080, 0xa220dc61, 0x694b775a, 0x161a121c, 
             0xaba93e2, 0xe52aa0c0, 0x43e0223c, 0x1d171b12, 
             0xb0d090e, 0xadc78bf2, 0xb9a8b62d, 0xc8a91e14, 
             0x8519f157, 0x4c0775af, 0xbbdd99ee, 0xfd607fa3, 
             0x9f2601f7, 0xbcf5725c, 0xc53b6644, 0x347efb5b, 
             0x7629438b, 0xdcc623cb, 0x68fcedb6, 0x63f1e4b8, 
             0xcadc31d7, 0x10856342, 0x40229713, 0x2011c684, 
             0x7d244a85, 0xf83dbbd2, 0x1132f9ae, 0x6da129c7, 
             0x4b2f9e1d, 0xf330b2dc, 0xec52860d, 0xd0e3c177, 
             0x6c16b32b, 0x99b970a9, 0xfa489411, 0x2264e947, 
             0xc48cfca8, 0x1a3ff0a0, 0xd82c7d56, 0xef903322, 
             0xc74e4987, 0xc1d138d9, 0xfea2ca8c, 0x360bd498, 
             0xcf81f5a6, 0x28de7aa5, 0x268eb7da, 0xa4bfad3f, 
             0xe49d3a2c, 0xd927850, 0x9bcc5f6a, 0x62467e54, 
             0xc2138df6, 0xe8b8d890, 0x5ef7392e, 0xf5afc382, 
             0xbe805d9f, 0x7c93d069, 0xa92dd56f, 0xb31225cf, 
             0x3b99acc8, 0xa77d1810, 0x6e639ce8, 0x7bbb3bdb, 
             0x97826cd, 0xf418596e, 0x1b79aec, 0xa89a4f83, 
             0x656e95e6, 0x7ee6ffaa, 0x8cfbc21, 0xe6e815ef, 
             0xd99be7ba, 0xce366f4a, 0xd4099fea, 0xd67cb029, 
             0xafb2a431, 0x31233f2a, 0x3094a5c6, 0xc066a235, 
             0x37bc4e74, 0xa6ca82fc, 0xb0d090e0, 0x15d8a733, 
             0x4a9804f1, 0xf7daec41, 0xe50cd7f, 0x2ff69117, 
             0x8dd64d76, 0x4db0ef43, 0x544daacc, 0xdf0496e4, 
             0xe3b5d19e, 0x1b886a4c, 0xb81f2cc1, 0x7f516546, 
             0x4ea5e9d, 0x5d358c01, 0x737487fa, 0x2e410bfb, 
             0x5a1d67b3, 0x52d2db92, 0x335610e9, 0x1347d66d, 
             0x8c61d79a, 0x7a0ca137, 0x8e14f859, 0x893c13eb, 
             0xee27a9ce, 0x35c961b7, 0xede51ce1, 0x3cb1477a, 
             0x59dfd29c, 0x3f73f255, 0x79ce1418, 0xbf37c773, 
             0xeacdf753, 0x5baafd5f, 0x146f3ddf, 0x86db4478, 
             0x81f3afca, 0x3ec468b9, 0x2c342438, 0x5f40a3c2, 
             0x72c31d16, 0xc25e2bc, 0x8b493c28, 0x41950dff, 
             0x7101a839, 0xdeb30c08, 0x9ce4b4d8, 0x90c15664, 
             0x6184cb7b, 0x70b632d5, 0x745c6c48, 0x4257b8d0};

uint U1[] = {0xa7f45150, 0x65417e53, 0xa4171ac3, 0x5e273a96, 
             0x6bab3bcb, 0x459d1ff1, 0x58faacab, 0x3e34b93, 
             0xfa302055, 0x6d76adf6, 0x76cc8891, 0x4c02f525, 
             0xd7e54ffc, 0xcb2ac5d7, 0x44352680, 0xa362b58f, 
             0x5ab1de49, 0x1bba2567, 0xeea4598, 0xc0fe5de1, 
             0x752fc302, 0xf04c8112, 0x97468da3, 0xf9d36bc6, 
             0x5f8f03e7, 0x9c921595, 0x7a6dbfeb, 0x595295da, 
             0x83bed42d, 0x217458d3, 0x69e04929, 0xc8c98e44, 
             0x89c2756a, 0x798ef478, 0x3e58996b, 0x71b927dd, 
             0x4fe1beb6, 0xad88f017, 0xac20c966, 0x3ace7db4, 
             0x4adf6318, 0x311ae582, 0x33519760, 0x7f536245, 
             0x7764b1e0, 0xae6bbb84, 0xa081fe1c, 0x2b08f994, 
             0x68487058, 0xfd458f19, 0x6cde9487, 0xf87b52b7, 
             0xd373ab23, 0x24b72e2, 0x8f1fe357, 0xab55662a, 
             0x28ebb207, 0xc2b52f03, 0x7bc5869a, 0x837d3a5, 
             0x872830f2, 0xa5bf23b2, 0x6a0302ba, 0x8216ed5c, 
             0x1ccf8a2b, 0xb479a792, 0xf207f3f0, 0xe2694ea1, 
             0xf4da65cd, 0xbe0506d5, 0x6234d11f, 0xfea6c48a, 
             0x532e349d, 0x55f3a2a0, 0xe18a0532, 0xebf6a475, 
             0xec830b39, 0xef6040aa, 0x9f715e06, 0x106ebd51, 
             0x8a213ef9, 0x6dd963d, 0x53eddae, 0xbde64d46, 
             0x8d5491b5, 0x5dc47105, 0xd406046f, 0x155060ff, 
             0xfb981924, 0xe9bdd697, 0x434089cc, 0x9ed96777, 
             0x42e8b0bd, 0x8b890788, 0x5b19e738, 0xeec879db, 
             0xa7ca147, 0xf427ce9, 0x1e84f8c9, 0x0, 
             0x86800983, 0xed2b3248, 0x70111eac, 0x725a6c4e, 
             0xff0efdfb, 0x38850f56, 0xd5ae3d1e, 0x392d3627, 
             0xd90f0a64, 0xa65c6821, 0x545b9bd1, 0x2e36243a, 
             0x670a0cb1, 0xe757930f, 0x96eeb4d2, 0x919b1b9e, 
             0xc5c0804f, 0x20dc61a2, 0x4b775a69, 0x1a121c16, 
             0xba93e20a, 0x2aa0c0e5, 0xe0223c43, 0x171b121d, 
             0xd090e0b, 0xc78bf2ad, 0xa8b62db9, 0xa91e14c8, 
             0x19f15785, 0x775af4c, 0xdd99eebb, 0x607fa3fd, 
             0x2601f79f, 0xf5725cbc, 0x3b6644c5, 0x7efb5b34, 
             0x29438b76, 0xc623cbdc, 0xfcedb668, 0xf1e4b863, 
             0xdc31d7ca, 0x85634210, 0x22971340, 0x11c68420, 
             0x244a857d, 0x3dbbd2f8, 0x32f9ae11, 0xa129c76d, 
             0x2f9e1d4b, 0x30b2dcf3, 0x52860dec, 0xe3c177d0, 
             0x16b32b6c, 0xb970a999, 0x489411fa, 0x64e94722, 
             0x8cfca8c4, 0x3ff0a01a, 0x2c7d56d8, 0x903322ef, 
             0x4e4987c7, 0xd138d9c1, 0xa2ca8cfe, 0xbd49836, 
             0x81f5a6cf, 0xde7aa528, 0x8eb7da26, 0xbfad3fa4, 
             0x9d3a2ce4, 0x9278500d, 0xcc5f6a9b, 0x467e5462, 
             0x138df6c2, 0xb8d890e8, 0xf7392e5e, 0xafc382f5, 
             0x805d9fbe, 0x93d0697c, 0x2dd56fa9, 0x1225cfb3, 
             0x99acc83b, 0x7d1810a7, 0x639ce86e, 0xbb3bdb7b, 
             0x7826cd09, 0x18596ef4, 0xb79aec01, 0x9a4f83a8, 
             0x6e95e665, 0xe6ffaa7e, 0xcfbc2108, 0xe815efe6, 
             0x9be7bad9, 0x366f4ace, 0x99fead4, 0x7cb029d6, 
             0xb2a431af, 0x233f2a31, 0x94a5c630, 0x66a235c0, 
             0xbc4e7437, 0xca82fca6, 0xd090e0b0, 0xd8a73315, 
             0x9804f14a, 0xdaec41f7, 0x50cd7f0e, 0xf691172f, 
             0xd64d768d, 0xb0ef434d, 0x4daacc54, 0x496e4df, 
             0xb5d19ee3, 0x886a4c1b, 0x1f2cc1b8, 0x5165467f, 
             0xea5e9d04, 0x358c015d, 0x7487fa73, 0x410bfb2e, 
             0x1d67b35a, 0xd2db9252, 0x5610e933, 0x47d66d13, 
             0x61d79a8c, 0xca1377a, 0x14f8598e, 0x3c13eb89, 
             0x27a9ceee, 0xc961b735, 0xe51ce1ed, 0xb1477a3c, 
             0xdfd29c59, 0x73f2553f, 0xce141879, 0x37c773bf, 
             0xcdf753ea, 0xaafd5f5b, 0x6f3ddf14, 0xdb447886, 
             0xf3afca81, 0xc468b93e, 0x3424382c, 0x40a3c25f, 
             0xc31d1672, 0x25e2bc0c, 0x493c288b, 0x950dff41, 
             0x1a83971, 0xb30c08de, 0xe4b4d89c, 0xc1566490, 
             0x84cb7b61, 0xb632d570, 0x5c6c4874, 0x57b8d042};

uint U2[] = {0xf45150a7, 0x417e5365, 0x171ac3a4, 0x273a965e, 
             0xab3bcb6b, 0x9d1ff145, 0xfaacab58, 0xe34b9303, 
             0x302055fa, 0x76adf66d, 0xcc889176, 0x2f5254c, 
             0xe54ffcd7, 0x2ac5d7cb, 0x35268044, 0x62b58fa3, 
             0xb1de495a, 0xba25671b, 0xea45980e, 0xfe5de1c0, 
             0x2fc30275, 0x4c8112f0, 0x468da397, 0xd36bc6f9, 
             0x8f03e75f, 0x9215959c, 0x6dbfeb7a, 0x5295da59, 
             0xbed42d83, 0x7458d321, 0xe0492969, 0xc98e44c8, 
             0xc2756a89, 0x8ef47879, 0x58996b3e, 0xb927dd71, 
             0xe1beb64f, 0x88f017ad, 0x20c966ac, 0xce7db43a, 
             0xdf63184a, 0x1ae58231, 0x51976033, 0x5362457f, 
             0x64b1e077, 0x6bbb84ae, 0x81fe1ca0, 0x8f9942b, 
             0x48705868, 0x458f19fd, 0xde94876c, 0x7b52b7f8, 
             0x73ab23d3, 0x4b72e202, 0x1fe3578f, 0x55662aab, 
             0xebb20728, 0xb52f03c2, 0xc5869a7b, 0x37d3a508, 
             0x2830f287, 0xbf23b2a5, 0x302ba6a, 0x16ed5c82, 
             0xcf8a2b1c, 0x79a792b4, 0x7f3f0f2, 0x694ea1e2, 
             0xda65cdf4, 0x506d5be, 0x34d11f62, 0xa6c48afe, 
             0x2e349d53, 0xf3a2a055, 0x8a0532e1, 0xf6a475eb, 
             0x830b39ec, 0x6040aaef, 0x715e069f, 0x6ebd5110, 
             0x213ef98a, 0xdd963d06, 0x3eddae05, 0xe64d46bd, 
             0x5491b58d, 0xc471055d, 0x6046fd4, 0x5060ff15, 
             0x981924fb, 0xbdd697e9, 0x4089cc43, 0xd967779e, 
             0xe8b0bd42, 0x8907888b, 0x19e7385b, 0xc879dbee, 
             0x7ca1470a, 0x427ce90f, 0x84f8c91e, 0x0, 
             0x80098386, 0x2b3248ed, 0x111eac70, 0x5a6c4e72, 
             0xefdfbff, 0x850f5638, 0xae3d1ed5, 0x2d362739, 
             0xf0a64d9, 0x5c6821a6, 0x5b9bd154, 0x36243a2e, 
             0xa0cb167, 0x57930fe7, 0xeeb4d296, 0x9b1b9e91, 
             0xc0804fc5, 0xdc61a220, 0x775a694b, 0x121c161a, 
             0x93e20aba, 0xa0c0e52a, 0x223c43e0, 0x1b121d17, 
             0x90e0b0d, 0x8bf2adc7, 0xb62db9a8, 0x1e14c8a9, 
             0xf1578519, 0x75af4c07, 0x99eebbdd, 0x7fa3fd60, 
             0x1f79f26, 0x725cbcf5, 0x6644c53b, 0xfb5b347e, 
             0x438b7629, 0x23cbdcc6, 0xedb668fc, 0xe4b863f1, 
             0x31d7cadc, 0x63421085, 0x97134022, 0xc6842011, 
             0x4a857d24, 0xbbd2f83d, 0xf9ae1132, 0x29c76da1, 
             0x9e1d4b2f, 0xb2dcf330, 0x860dec52, 0xc177d0e3, 
             0xb32b6c16, 0x70a999b9, 0x9411fa48, 0xe9472264, 
             0xfca8c48c, 0xf0a01a3f, 0x7d56d82c, 0x3322ef90, 
             0x4987c74e, 0x38d9c1d1, 0xca8cfea2, 0xd498360b, 
             0xf5a6cf81, 0x7aa528de, 0xb7da268e, 0xad3fa4bf, 
             0x3a2ce49d, 0x78500d92, 0x5f6a9bcc, 0x7e546246, 
             0x8df6c213, 0xd890e8b8, 0x392e5ef7, 0xc382f5af, 
             0x5d9fbe80, 0xd0697c93, 0xd56fa92d, 0x25cfb312, 
             0xacc83b99, 0x1810a77d, 0x9ce86e63, 0x3bdb7bbb, 
             0x26cd0978, 0x596ef418, 0x9aec01b7, 0x4f83a89a, 
             0x95e6656e, 0xffaa7ee6, 0xbc2108cf, 0x15efe6e8, 
             0xe7bad99b, 0x6f4ace36, 0x9fead409, 0xb029d67c, 
             0xa431afb2, 0x3f2a3123, 0xa5c63094, 0xa235c066, 
             0x4e7437bc, 0x82fca6ca, 0x90e0b0d0, 0xa73315d8, 
             0x4f14a98, 0xec41f7da, 0xcd7f0e50, 0x91172ff6, 
             0x4d768dd6, 0xef434db0, 0xaacc544d, 0x96e4df04, 
             0xd19ee3b5, 0x6a4c1b88, 0x2cc1b81f, 0x65467f51, 
             0x5e9d04ea, 0x8c015d35, 0x87fa7374, 0xbfb2e41, 
             0x67b35a1d, 0xdb9252d2, 0x10e93356, 0xd66d1347, 
             0xd79a8c61, 0xa1377a0c, 0xf8598e14, 0x13eb893c, 
             0xa9ceee27, 0x61b735c9, 0x1ce1ede5, 0x477a3cb1, 
             0xd29c59df, 0xf2553f73, 0x141879ce, 0xc773bf37, 
             0xf753eacd, 0xfd5f5baa, 0x3ddf146f, 0x447886db, 
             0xafca81f3, 0x68b93ec4, 0x24382c34, 0xa3c25f40, 
             0x1d1672c3, 0xe2bc0c25, 0x3c288b49, 0xdff4195, 
             0xa8397101, 0xc08deb3, 0xb4d89ce4, 0x566490c1, 
             0xcb7b6184, 0x32d570b6, 0x6c48745c, 0xb8d04257};

uint U3[] = {0x5150a7f4, 0x7e536541, 0x1ac3a417, 0x3a965e27, 
             0x3bcb6bab, 0x1ff1459d, 0xacab58fa, 0x4b9303e3, 
             0x2055fa30, 0xadf66d76, 0x889176cc, 0xf5254c02, 
             0x4ffcd7e5, 0xc5d7cb2a, 0x26804435, 0xb58fa362, 
             0xde495ab1, 0x25671bba, 0x45980eea, 0x5de1c0fe, 
             0xc302752f, 0x8112f04c, 0x8da39746, 0x6bc6f9d3, 
             0x3e75f8f, 0x15959c92, 0xbfeb7a6d, 0x95da5952, 
             0xd42d83be, 0x58d32174, 0x492969e0, 0x8e44c8c9, 
             0x756a89c2, 0xf478798e, 0x996b3e58, 0x27dd71b9, 
             0xbeb64fe1, 0xf017ad88, 0xc966ac20, 0x7db43ace, 
             0x63184adf, 0xe582311a, 0x97603351, 0x62457f53, 
             0xb1e07764, 0xbb84ae6b, 0xfe1ca081, 0xf9942b08, 
             0x70586848, 0x8f19fd45, 0x94876cde, 0x52b7f87b, 
             0xab23d373, 0x72e2024b, 0xe3578f1f, 0x662aab55, 
             0xb20728eb, 0x2f03c2b5, 0x869a7bc5, 0xd3a50837, 
             0x30f28728, 0x23b2a5bf, 0x2ba6a03, 0xed5c8216, 
             0x8a2b1ccf, 0xa792b479, 0xf3f0f207, 0x4ea1e269, 
             0x65cdf4da, 0x6d5be05, 0xd11f6234, 0xc48afea6, 
             0x349d532e, 0xa2a055f3, 0x532e18a, 0xa475ebf6, 
             0xb39ec83, 0x40aaef60, 0x5e069f71, 0xbd51106e, 
             0x3ef98a21, 0x963d06dd, 0xddae053e, 0x4d46bde6, 
             0x91b58d54, 0x71055dc4, 0x46fd406, 0x60ff1550, 
             0x1924fb98, 0xd697e9bd, 0x89cc4340, 0x67779ed9, 
             0xb0bd42e8, 0x7888b89, 0xe7385b19, 0x79dbeec8, 
             0xa1470a7c, 0x7ce90f42, 0xf8c91e84, 0x0, 
             0x9838680, 0x3248ed2b, 0x1eac7011, 0x6c4e725a, 
             0xfdfbff0e, 0xf563885, 0x3d1ed5ae, 0x3627392d, 
             0xa64d90f, 0x6821a65c, 0x9bd1545b, 0x243a2e36, 
             0xcb1670a, 0x930fe757, 0xb4d296ee, 0x1b9e919b, 
             0x804fc5c0, 0x61a220dc, 0x5a694b77, 0x1c161a12, 
             0xe20aba93, 0xc0e52aa0, 0x3c43e022, 0x121d171b, 
             0xe0b0d09, 0xf2adc78b, 0x2db9a8b6, 0x14c8a91e, 
             0x578519f1, 0xaf4c0775, 0xeebbdd99, 0xa3fd607f, 
             0xf79f2601, 0x5cbcf572, 0x44c53b66, 0x5b347efb, 
             0x8b762943, 0xcbdcc623, 0xb668fced, 0xb863f1e4, 
             0xd7cadc31, 0x42108563, 0x13402297, 0x842011c6, 
             0x857d244a, 0xd2f83dbb, 0xae1132f9, 0xc76da129, 
             0x1d4b2f9e, 0xdcf330b2, 0xdec5286, 0x77d0e3c1, 
             0x2b6c16b3, 0xa999b970, 0x11fa4894, 0x472264e9, 
             0xa8c48cfc, 0xa01a3ff0, 0x56d82c7d, 0x22ef9033, 
             0x87c74e49, 0xd9c1d138, 0x8cfea2ca, 0x98360bd4, 
             0xa6cf81f5, 0xa528de7a, 0xda268eb7, 0x3fa4bfad, 
             0x2ce49d3a, 0x500d9278, 0x6a9bcc5f, 0x5462467e, 
             0xf6c2138d, 0x90e8b8d8, 0x2e5ef739, 0x82f5afc3, 
             0x9fbe805d, 0x697c93d0, 0x6fa92dd5, 0xcfb31225, 
             0xc83b99ac, 0x10a77d18, 0xe86e639c, 0xdb7bbb3b, 
             0xcd097826, 0x6ef41859, 0xec01b79a, 0x83a89a4f, 
             0xe6656e95, 0xaa7ee6ff, 0x2108cfbc, 0xefe6e815, 
             0xbad99be7, 0x4ace366f, 0xead4099f, 0x29d67cb0, 
             0x31afb2a4, 0x2a31233f, 0xc63094a5, 0x35c066a2, 
             0x7437bc4e, 0xfca6ca82, 0xe0b0d090, 0x3315d8a7, 
             0xf14a9804, 0x41f7daec, 0x7f0e50cd, 0x172ff691, 
             0x768dd64d, 0x434db0ef, 0xcc544daa, 0xe4df0496, 
             0x9ee3b5d1, 0x4c1b886a, 0xc1b81f2c, 0x467f5165, 
             0x9d04ea5e, 0x15d358c, 0xfa737487, 0xfb2e410b, 
             0xb35a1d67, 0x9252d2db, 0xe9335610, 0x6d1347d6, 
             0x9a8c61d7, 0x377a0ca1, 0x598e14f8, 0xeb893c13, 
             0xceee27a9, 0xb735c961, 0xe1ede51c, 0x7a3cb147, 
             0x9c59dfd2, 0x553f73f2, 0x1879ce14, 0x73bf37c7, 
             0x53eacdf7, 0x5f5baafd, 0xdf146f3d, 0x7886db44, 
             0xca81f3af, 0xb93ec468, 0x382c3424, 0xc25f40a3, 
             0x1672c31d, 0xbc0c25e2, 0x288b493c, 0xff41950d, 
             0x397101a8, 0x8deb30c, 0xd89ce4b4, 0x6490c156, 
             0x7b6184cb, 0xd570b632, 0x48745c6c, 0xd04257b8 };

uint aes_sbox[] = {
    0x63636363, 0x7C7C7C7C, 0x77777777, 0x7B7B7B7B, 0xF2F2F2F2, 0x6B6B6B6B, 0x6F6F6F6F, 0xC5C5C5C5,
    0x30303030, 0x01010101, 0x67676767, 0x2B2B2B2B, 0xFEFEFEFE, 0xD7D7D7D7, 0xABABABAB, 0x76767676,
    0xCACACACA, 0x82828282, 0xC9C9C9C9, 0x7D7D7D7D, 0xFAFAFAFA, 0x59595959, 0x47474747, 0xF0F0F0F0,
    0xADADADAD, 0xD4D4D4D4, 0xA2A2A2A2, 0xAFAFAFAF, 0x9C9C9C9C, 0xA4A4A4A4, 0x72727272, 0xC0C0C0C0,
    0xB7B7B7B7, 0xFDFDFDFD, 0x93939393, 0x26262626, 0x36363636, 0x3F3F3F3F, 0xF7F7F7F7, 0xCCCCCCCC,
    0x34343434, 0xA5A5A5A5, 0xE5E5E5E5, 0xF1F1F1F1, 0x71717171, 0xD8D8D8D8, 0x31313131, 0x15151515,
    0x04040404, 0xC7C7C7C7, 0x23232323, 0xC3C3C3C3, 0x18181818, 0x96969696, 0x05050505, 0x9A9A9A9A,
    0x07070707, 0x12121212, 0x80808080, 0xE2E2E2E2, 0xEBEBEBEB, 0x27272727, 0xB2B2B2B2, 0x75757575,
    0x09090909, 0x83838383, 0x2C2C2C2C, 0x1A1A1A1A, 0x1B1B1B1B, 0x6E6E6E6E, 0x5A5A5A5A, 0xA0A0A0A0,
    0x52525252, 0x3B3B3B3B, 0xD6D6D6D6, 0xB3B3B3B3, 0x29292929, 0xE3E3E3E3, 0x2F2F2F2F, 0x84848484,
    0x53535353, 0xD1D1D1D1, 0x00000000, 0xEDEDEDED, 0x20202020, 0xFCFCFCFC, 0xB1B1B1B1, 0x5B5B5B5B,
    0x6A6A6A6A, 0xCBCBCBCB, 0xBEBEBEBE, 0x39393939, 0x4A4A4A4A, 0x4C4C4C4C, 0x58585858, 0xCFCFCFCF,
    0xD0D0D0D0, 0xEFEFEFEF, 0xAAAAAAAA, 0xFBFBFBFB, 0x43434343, 0x4D4D4D4D, 0x33333333, 0x85858585,
    0x45454545, 0xF9F9F9F9, 0x02020202, 0x7F7F7F7F, 0x50505050, 0x3C3C3C3C, 0x9F9F9F9F, 0xA8A8A8A8,
    0x51515151, 0xA3A3A3A3, 0x40404040, 0x8F8F8F8F, 0x92929292, 0x9D9D9D9D, 0x38383838, 0xF5F5F5F5,
    0xBCBCBCBC, 0xB6B6B6B6, 0xDADADADA, 0x21212121, 0x10101010, 0xFFFFFFFF, 0xF3F3F3F3, 0xD2D2D2D2,
    0xCDCDCDCD, 0x0C0C0C0C, 0x13131313, 0xECECECEC, 0x5F5F5F5F, 0x97979797, 0x44444444, 0x17171717,
    0xC4C4C4C4, 0xA7A7A7A7, 0x7E7E7E7E, 0x3D3D3D3D, 0x64646464, 0x5D5D5D5D, 0x19191919, 0x73737373,
    0x60606060, 0x81818181, 0x4F4F4F4F, 0xDCDCDCDC, 0x22222222, 0x2A2A2A2A, 0x90909090, 0x88888888,
    0x46464646, 0xEEEEEEEE, 0xB8B8B8B8, 0x14141414, 0xDEDEDEDE, 0x5E5E5E5E, 0x0B0B0B0B, 0xDBDBDBDB,
    0xE0E0E0E0, 0x32323232, 0x3A3A3A3A, 0x0A0A0A0A, 0x49494949, 0x06060606, 0x24242424, 0x5C5C5C5C,
    0xC2C2C2C2, 0xD3D3D3D3, 0xACACACAC, 0x62626262, 0x91919191, 0x95959595, 0xE4E4E4E4, 0x79797979,
    0xE7E7E7E7, 0xC8C8C8C8, 0x37373737, 0x6D6D6D6D, 0x8D8D8D8D, 0xD5D5D5D5, 0x4E4E4E4E, 0xA9A9A9A9,
    0x6C6C6C6C, 0x56565656, 0xF4F4F4F4, 0xEAEAEAEA, 0x65656565, 0x7A7A7A7A, 0xAEAEAEAE, 0x08080808,
    0xBABABABA, 0x78787878, 0x25252525, 0x2E2E2E2E, 0x1C1C1C1C, 0xA6A6A6A6, 0xB4B4B4B4, 0xC6C6C6C6,
    0xE8E8E8E8, 0xDDDDDDDD, 0x74747474, 0x1F1F1F1F, 0x4B4B4B4B, 0xBDBDBDBD, 0x8B8B8B8B, 0x8A8A8A8A,
    0x70707070, 0x3E3E3E3E, 0xB5B5B5B5, 0x66666666, 0x48484848, 0x03030303, 0xF6F6F6F6, 0x0E0E0E0E,
    0x61616161, 0x35353535, 0x57575757, 0xB9B9B9B9, 0x86868686, 0xC1C1C1C1, 0x1D1D1D1D, 0x9E9E9E9E,
    0xE1E1E1E1, 0xF8F8F8F8, 0x98989898, 0x11111111, 0x69696969, 0xD9D9D9D9, 0x8E8E8E8E, 0x94949494,
    0x9B9B9B9B, 0x1E1E1E1E, 0x87878787, 0xE9E9E9E9, 0xCECECECE, 0x55555555, 0x28282828, 0xDFDFDFDF,
    0x8C8C8C8C, 0xA1A1A1A1, 0x89898989, 0x0D0D0D0D, 0xBFBFBFBF, 0xE6E6E6E6, 0x42424242, 0x68686868,
    0x41414141, 0x99999999, 0x2D2D2D2D, 0x0F0F0F0F, 0xB0B0B0B0, 0x54545454, 0xBBBBBBBB, 0x16161616 };

uint aes_inv_sbox[256] = {
  0x52525252, 0x09090909, 0x6a6a6a6a, 0xd5d5d5d5, 0x30303030, 0x36363636, 0xa5a5a5a5, 0x38383838, 
  0xbfbfbfbf, 0x40404040, 0xa3a3a3a3, 0x9e9e9e9e, 0x81818181, 0xf3f3f3f3, 0xd7d7d7d7, 0xfbfbfbfb,
  0x7c7c7c7c, 0xe3e3e3e3, 0x39393939, 0x82828282, 0x9b9b9b9b, 0x2f2f2f2f, 0xffffffff, 0x87878787,
  0x34343434, 0x8e8e8e8e, 0x43434343, 0x44444444, 0xc4c4c4c4, 0xdededede, 0xe9e9e9e9, 0xcbcbcbcb,
  0x54545454, 0x7b7b7b7b, 0x94949494, 0x32323232, 0xa6a6a6a6, 0xc2c2c2c2, 0x23232323, 0x3d3d3d3d, 
  0xeeeeeeee, 0x4c4c4c4c, 0x95959595, 0x0b0b0b0b, 0x42424242, 0xfafafafa, 0xc3c3c3c3, 0x4e4e4e4e,
  0x08080808, 0x2e2e2e2e, 0xa1a1a1a1, 0x66666666, 0x28282828, 0xd9d9d9d9, 0x24242424, 0xb2b2b2b2,
  0x76767676, 0x5b5b5b5b, 0xa2a2a2a2, 0x49494949, 0x6d6d6d6d, 0x8b8b8b8b, 0xd1d1d1d1, 0x25252525,
  0x72727272, 0xf8f8f8f8, 0xf6f6f6f6, 0x64646464, 0x86868686, 0x68686868, 0x98989898, 0x16161616, 
  0xd4d4d4d4, 0xa4a4a4a4, 0x5c5c5c5c, 0xcccccccc, 0x5d5d5d5d, 0x65656565, 0xb6b6b6b6, 0x92929292,
  0x6c6c6c6c, 0x70707070, 0x48484848, 0x50505050, 0xfdfdfdfd, 0xedededed, 0xb9b9b9b9, 0xdadadada, 
  0x5e5e5e5e, 0x15151515, 0x46464646, 0x57575757, 0xa7a7a7a7, 0x8d8d8d8d, 0x9d9d9d9d, 0x84848484,
  0x90909090, 0xd8d8d8d8, 0xabababab, 0x00000000, 0x8c8c8c8c, 0xbcbcbcbc, 0xd3d3d3d3, 0x0a0a0a0a, 
  0xf7f7f7f7, 0xe4e4e4e4, 0x58585858, 0x05050505, 0xb8b8b8b8, 0xb3b3b3b3, 0x45454545, 0x06060606,
  0xd0d0d0d0, 0x2c2c2c2c, 0x1e1e1e1e, 0x8f8f8f8f, 0xcacacaca, 0x3f3f3f3f, 0x0f0f0f0f, 0x02020202, 
  0xc1c1c1c1, 0xafafafaf, 0xbdbdbdbd, 0x03030303, 0x01010101, 0x13131313, 0x8a8a8a8a, 0x6b6b6b6b,
  0x3a3a3a3a, 0x91919191, 0x11111111, 0x41414141, 0x4f4f4f4f, 0x67676767, 0xdcdcdcdc, 0xeaeaeaea, 
  0x97979797, 0xf2f2f2f2, 0xcfcfcfcf, 0xcececece, 0xf0f0f0f0, 0xb4b4b4b4, 0xe6e6e6e6, 0x73737373,
  0x96969696, 0xacacacac, 0x74747474, 0x22222222, 0xe7e7e7e7, 0xadadadad, 0x35353535, 0x85858585, 
  0xe2e2e2e2, 0xf9f9f9f9, 0x37373737, 0xe8e8e8e8, 0x1c1c1c1c, 0x75757575, 0xdfdfdfdf, 0x6e6e6e6e,
  0x47474747, 0xf1f1f1f1, 0x1a1a1a1a, 0x71717171, 0x1d1d1d1d, 0x29292929, 0xc5c5c5c5, 0x89898989, 
  0x6f6f6f6f, 0xb7b7b7b7, 0x62626262, 0x0e0e0e0e, 0xaaaaaaaa, 0x18181818, 0xbebebebe, 0x1b1b1b1b,
  0xfcfcfcfc, 0x56565656, 0x3e3e3e3e, 0x4b4b4b4b, 0xc6c6c6c6, 0xd2d2d2d2, 0x79797979, 0x20202020, 
  0x9a9a9a9a, 0xdbdbdbdb, 0xc0c0c0c0, 0xfefefefe, 0x78787878, 0xcdcdcdcd, 0x5a5a5a5a, 0xf4f4f4f4,
  0x1f1f1f1f, 0xdddddddd, 0xa8a8a8a8, 0x33333333, 0x88888888, 0x07070707, 0xc7c7c7c7, 0x31313131, 
  0xb1b1b1b1, 0x12121212, 0x10101010, 0x59595959, 0x27272727, 0x80808080, 0xecececec, 0x5f5f5f5f,
  0x60606060, 0x51515151, 0x7f7f7f7f, 0xa9a9a9a9, 0x19191919, 0xb5b5b5b5, 0x4a4a4a4a, 0x0d0d0d0d, 
  0x2d2d2d2d, 0xe5e5e5e5, 0x7a7a7a7a, 0x9f9f9f9f, 0x93939393, 0xc9c9c9c9, 0x9c9c9c9c, 0xefefefef,
  0xa0a0a0a0, 0xe0e0e0e0, 0x3b3b3b3b, 0x4d4d4d4d, 0xaeaeaeae, 0x2a2a2a2a, 0xf5f5f5f5, 0xb0b0b0b0, 
  0xc8c8c8c8, 0xebebebeb, 0xbbbbbbbb, 0x3c3c3c3c, 0x83838383, 0x53535353, 0x99999999, 0x61616161,
  0x17171717, 0x2b2b2b2b, 0x04040404, 0x7e7e7e7e, 0xbabababa, 0x77777777, 0xd6d6d6d6, 0x26262626, 
  0xe1e1e1e1, 0x69696969, 0x14141414, 0x63636363, 0x55555555, 0x21212121, 0x0c0c0c0c, 0x7d7d7d7d
};

uint RC[] ={ 0x00000001, 0x00000002, 0x00000004, 0x00000008,
             0x00000010, 0x00000020, 0x00000040, 0x00000080,
             0x0000001B, 0x00000036                         };



void aes_schedule( int nb, int nr, unsigned char* k, uint* RK ) 
{
    int i;
    for( int i = 0; i < (nb); i++ ) 
    {
        U8_TO_U32_LE( RK[ i ], k, 4*i );
    }

    for( int i = nb, j = 0; i < ( 4 * ( nr + 1 ) ); i++ ) 
    {
        uint t = RK[ i -  1 ];
        uint p = RK[ i - nb ];

        if( ( ( i % nb ) == 0 ) )
        {
          t = RC[ j++ ] ^ ( aes_sbox[ ( t >>  8 ) & 0xFF ] & 0x000000FF ) ^
                          ( aes_sbox[ ( t >> 16 ) & 0xFF ] & 0x0000FF00 ) ^
                          ( aes_sbox[ ( t >> 24 ) & 0xFF ] & 0x00FF0000 ) ^
                          ( aes_sbox[ ( t >>  0 ) & 0xFF ] & 0xFF000000 ) ;
        }
        else if( ( ( i % nb ) == 4 ) && ( nb == 8 ) ) {
            t = ( aes_sbox[ ( t >>  0 ) & 0xFF ] & 0x000000FF ) ^
                ( aes_sbox[ ( t >>  8 ) & 0xFF ] & 0x0000FF00 ) ^
                ( aes_sbox[ ( t >> 16 ) & 0xFF ] & 0x00FF0000 ) ^
                ( aes_sbox[ ( t >> 24 ) & 0xFF ] & 0xFF000000 ) ;
        }
        RK[ i ] = t ^ p;
    }
}


void rotate(unsigned char *in)
{
    unsigned char a, c;
    a = in[0];

    for(c = 0; c < 3; c++) 
        in[c] = in[c + 1];

    in[3] = a;
    return;
}

/* This is the core key expansion, which, given a 4-byte value,
 * does some scrambling */
void schedule_core(unsigned char *in, unsigned char i)
{
    char a;
    /* Rotate the input 8 bits to the left */
    rotate(in);

    /* Apply Rijndael's s-box on all 4 bytes */
    for(a = 0; a < 4; a++) 
        in[a] = aes_sbox[in[a]];

    /* On just the first byte, add 2^i to the byte */
    in[0] ^= RC[i];
}

void expand_key(unsigned char *in)
{
    unsigned char t[4];
    /* c is 16 because the first sub-key is the user-supplied key */
    unsigned char c = 16;
    unsigned char i = 0;
        unsigned char a;

    /* We need 11 sets of sixteen bytes each for 128-bit mode */
    while(c < 176)
    {
        /* Copy the temporary variable over from the last 4-byte
         * block */
        for(a = 0; a < 4; a++)
            t[a] = in[a + c - 4];

        /* Every four blocks (of four bytes), 
         * do a complex calculation */
        if(c % 16 == 0)
        {
            schedule_core(t, i);
            i++;
        }

        for(a = 0; a < 4; a++)
        {
                in[c] = in[c - 16] ^ t[a];
                c++;
        }
    }
}

uint *convertCharStringToUINT(unsigned char *in)
{
    uint *toReturn = new uint[44];
    int i;

    for(i = 0; i < 44; i ++)
        U8_TO_U32_LE(toReturn[i], in, 4 * i);

    return toReturn;
}

uint *getUintKeySchedule(unsigned char *key)
{
    unsigned char *tempExpKey = new unsigned char[44*4];
    strncpy( (char*)tempExpKey, (char*)key, 16);

    expand_key(tempExpKey);

    uint *toReturn = convertCharStringToUINT(tempExpKey);

    return toReturn;
}

// Takes an EXPANDED key and reverses order/applies InvMixColumns
uint *decryptionKeySchedule_128(uint *expandedEncKey)
{
    uint *decRK = new uint[44];
    int i, j;

    for(i = 0; i < 41; i += 4)
    {
        decRK[i + 0] = expandedEncKey[40 - i + 0];
        decRK[i + 1] = expandedEncKey[40 - i + 1];
        decRK[i + 2] = expandedEncKey[40 - i + 2];
        decRK[i + 3] = expandedEncKey[40 - i + 3];
    }

    for(i = 4; i < 40; i ++)
    {
        decRK[i] =  U0[aes_sbox[ (decRK[i] >>  0) & 0xff ] & 0xff] ^
                    U1[aes_sbox[ (decRK[i] >>  8) & 0xff ] & 0xff] ^
                    U2[aes_sbox[ (decRK[i] >> 16) & 0xff ] & 0xff] ^
                    U3[aes_sbox[ (decRK[i] >> 24) & 0xff ] & 0xff];
    }

    return decRK;
}



void aes_128_encrypt( unsigned char *C, unsigned char *M, uint *RK )
{
    uint t0, t1, t2, t3, t4, t5, t6, t7;

    U8_TO_U32_LE( t0, M,  0 ); U8_TO_U32_LE( t1, M,  4 );
    U8_TO_U32_LE( t2, M,  8 ); U8_TO_U32_LE( t3, M, 12 );

    ROUND1(  0,  1,  2,  3 );
    ROUND2(  4,  5,  6,  7 ); ROUND2(  8,  9, 10, 11 ); ROUND2( 12, 13, 14, 15 );
    ROUND2( 16, 17, 18, 19 ); ROUND2( 20, 21, 22, 23 ); ROUND2( 24, 25, 26, 27 );
    ROUND2( 28, 29, 30, 31 ); ROUND2( 32, 33, 34, 35 ); ROUND2( 36, 37, 38, 39 );
    ROUND3( 40, 41, 42, 43 );

    U32_TO_U8_LE( C, t4,  0 ); U32_TO_U8_LE( C, t5,  4 );
    U32_TO_U8_LE( C, t6,  8 ); U32_TO_U8_LE( C, t7, 12 );
}

void aes_128_decrypt(unsigned char *C, unsigned char *M, uint *RK)
{
    uint t0, t1, t2, t3, t4, t5, t6, t7;

    U8_TO_U32_LE( t0, C, 0 ); U8_TO_U32_LE( t1, C,  4 );
    U8_TO_U32_LE( t2, C, 8 ); U8_TO_U32_LE( t3, C, 12 );

    ROUND1(  0,  1,  2,  3 );
    ROUND2_INV(  4,  5,  6,  7 ); ROUND2_INV(  8,  9, 10, 11 ); ROUND2_INV( 12, 13, 14, 15 );
    ROUND2_INV( 16, 17, 18, 19 ); ROUND2_INV( 20, 21, 22, 23 ); ROUND2_INV( 24, 25, 26, 27 );
    ROUND2_INV( 28, 29, 30, 31 ); ROUND2_INV( 32, 33, 34, 35 ); ROUND2_INV( 36, 37, 38, 39 );
    ROUND3_INV( 40, 41, 42, 43 );
    
    U32_TO_U8_LE( M, t4,  0 ); U32_TO_U8_LE( M, t5,  4 );
    U32_TO_U8_LE( M, t6,  8 ); U32_TO_U8_LE( M, t7, 12 );
}


void aes_192_encrypt( unsigned char* C, unsigned char* M, uint* RK )
{
    uint t0, t1, t2, t3, t4, t5, t6, t7;

    U8_TO_U32_LE( t0, M,  0 ); U8_TO_U32_LE( t1, M,  4 );
    U8_TO_U32_LE( t2, M,  8 ); U8_TO_U32_LE( t3, M, 12 );

    ROUND1(  0,  1,  2,  3 );
    ROUND2(  4,  5,  6,  7 ); ROUND2(  8,  9, 10, 11 ); ROUND2( 12, 13, 14, 15 );
    ROUND2( 16, 17, 18, 19 ); ROUND2( 20, 21, 22, 23 ); ROUND2( 24, 25, 26, 27 );
    ROUND2( 28, 29, 30, 31 ); ROUND2( 32, 33, 34, 35 ); ROUND2( 36, 37, 38, 39 );
    ROUND2( 40, 41, 42, 43 ); ROUND2( 44, 45, 46, 47 ); 
    ROUND3( 48, 49, 50, 51 );

    U32_TO_U8_LE( C, t4, 0 ); U32_TO_U8_LE( C, t5,  4 );
    U32_TO_U8_LE( C, t6, 8 ); U32_TO_U8_LE( C, t7, 12 );
}


void aes_256_encrypt( unsigned char* C, unsigned char* M, uint* RK )
{
    uint t0, t1, t2, t3, t4, t5, t6, t7;

    U8_TO_U32_LE( t0, M,  0 ); U8_TO_U32_LE( t1, M,  4 );
    U8_TO_U32_LE( t2, M,  8 ); U8_TO_U32_LE( t3, M, 12 );

    ROUND1(  0,  1,  2,  3 );
    ROUND2(  4,  5,  6,  7 ); ROUND2(  8,  9, 10, 11 ); ROUND2( 12, 13, 14, 15 );
    ROUND2( 16, 17, 18, 19 ); ROUND2( 20, 21, 22, 23 ); ROUND2( 24, 25, 26, 27 );
    ROUND2( 28, 29, 30, 31 ); ROUND2( 32, 33, 34, 35 ); ROUND2( 36, 37, 38, 39 );
    ROUND2( 40, 41, 42, 43 ); ROUND2( 44, 45, 46, 47 ); ROUND2( 48, 49, 50, 51 ); 
    ROUND2( 52, 53, 54, 55 ); 
    ROUND3( 56, 57, 58, 59 );

    U32_TO_U8_LE( C, t4,  0 ); U32_TO_U8_LE( C, t5,  4 );
    U32_TO_U8_LE( C, t6,  8 ); U32_TO_U8_LE( C, t7, 12 );
}




/**********************
 *    M-Code Version  *
 **********************/

 /*

#define cpuid(func,ax,bx,cx,dx)\
    __asm__ __volatile__("cpuid": "=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func)); 


int Check_CPU_support_AES() 
{
    unsigned int a, b, c, d; 
    cpuid(1, a, b, c, d); 
    return (c & 0x2000000); 
}



inline __m128i AES_128_ASSIST (__m128i temp1, __m128i temp2)
{
    __m128i temp3; temp2 = _mm_shuffle_epi32 (temp2 ,0xff);
    temp3 = _mm_slli_si128 (temp1, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp3 = _mm_slli_si128 (temp3, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp3 = _mm_slli_si128 (temp3, 0x4);
    temp1 = _mm_xor_si128 (temp1, temp3);
    temp1 = _mm_xor_si128 (temp1, temp2);
    return temp1;
}


void aes_128_schedule( unsigned char* key, const unsigned char* userkey )
{
    __m128i temp1, temp2; 
    __m128i *Key_Schedule = (__m128i*)key; 
    temp1 = _mm_loadu_si128((__m128i*)userkey); 
    Key_Schedule[0] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x1);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[1] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x2);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[2] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x4);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[3] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x8);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[4] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x10);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[5] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x20);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[6] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x40);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[7] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x80);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[8] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x1b);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[9] = temp1;
    temp2 = _mm_aeskeygenassist_si128 (temp1, 0x36);
    temp1 = AES_128_ASSIST(temp1, temp2);
    Key_Schedule[10] = temp1; 
}


inline void KEY_192_ASSIST(__m128i* temp1, __m128i * temp2, __m128i * temp3)
{ 
    __m128i temp4;
    *temp2 = _mm_shuffle_epi32 (*temp2, 0x55);
    temp4 = _mm_slli_si128 (*temp1, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    *temp1 = _mm_xor_si128 (*temp1, *temp2);
    *temp2 = _mm_shuffle_epi32(*temp1, 0xff);
    temp4 = _mm_slli_si128 (*temp3, 0x4);
    *temp3 = _mm_xor_si128 (*temp3, temp4);
    *temp3 = _mm_xor_si128 (*temp3, *temp2);
}


void aes_192_schedule( unsigned char* key, const unsigned char* userkey )
{ 
    __m128i temp1, temp2, temp3;
    __m128i *Key_Schedule = (__m128i*)key;
    temp1 = _mm_loadu_si128((__m128i*)userkey);
    temp3 = _mm_loadu_si128((__m128i*)(userkey+16));
    Key_Schedule[0]=temp1;
    Key_Schedule[1]=temp3;
    temp2=_mm_aeskeygenassist_si128 (temp3,0x1);
    KEY_192_ASSIST(&temp1, &temp2, &temp3);
    Key_Schedule[1] = (__m128i)_mm_shuffle_pd((__m128d)Key_Schedule[1],(__m128d)temp1,0);
    Key_Schedule[2] = (__m128i)_mm_shuffle_pd((__m128d)temp1,(__m128d)temp3,1);
    temp2=_mm_aeskeygenassist_si128 (temp3,0x2);
    KEY_192_ASSIST(&temp1, &temp2, &temp3);
    Key_Schedule[3]=temp1;
    Key_Schedule[4]=temp3;
    temp2=_mm_aeskeygenassist_si128 (temp3,0x4);
    KEY_192_ASSIST(&temp1, &temp2, &temp3);
    Key_Schedule[4] = (__m128i)_mm_shuffle_pd((__m128d)Key_Schedule[4],(__m128d)temp1,0);
    Key_Schedule[5] = (__m128i)_mm_shuffle_pd((__m128d)temp1,(__m128d)temp3,1);
    temp2=_mm_aeskeygenassist_si128 (temp3,0x8);
    KEY_192_ASSIST(&temp1, &temp2, &temp3);
    Key_Schedule[6]=temp1;
    Key_Schedule[7]=temp3;
    temp2=_mm_aeskeygenassist_si128 (temp3,0x10);
    KEY_192_ASSIST(&temp1, &temp2, &temp3);
    Key_Schedule[7] = (__m128i)_mm_shuffle_pd((__m128d)Key_Schedule[7],(__m128d)temp1,0);
    Key_Schedule[8] = (__m128i)_mm_shuffle_pd((__m128d)temp1,(__m128d)temp3,1);
    temp2=_mm_aeskeygenassist_si128 (temp3,0x20);
    KEY_192_ASSIST(&temp1, &temp2, &temp3);
    Key_Schedule[9]=temp1;
    Key_Schedule[10]=temp3;
    temp2=_mm_aeskeygenassist_si128 (temp3,0x40);
    KEY_192_ASSIST(&temp1, &temp2, &temp3);
    Key_Schedule[10] = (__m128i)_mm_shuffle_pd((__m128d)Key_Schedule[10],(__m128d)temp1,0);
    Key_Schedule[11] = (__m128i)_mm_shuffle_pd((__m128d)temp1,(__m128d)temp3,1);
    temp2=_mm_aeskeygenassist_si128 (temp3,0x80);
    KEY_192_ASSIST(&temp1, &temp2, &temp3);
    Key_Schedule[12]=temp1; 
}


inline void KEY_256_ASSIST_1(__m128i* temp1, __m128i * temp2)
{
    __m128i temp4;
    *temp2 = _mm_shuffle_epi32(*temp2, 0xff);
    temp4 = _mm_slli_si128 (*temp1, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp1 = _mm_xor_si128 (*temp1, temp4);
    *temp1 = _mm_xor_si128 (*temp1, *temp2);
}


inline void KEY_256_ASSIST_2(__m128i* temp1, __m128i * temp3)
{ 
    __m128i temp2,temp4;
    temp4 = _mm_aeskeygenassist_si128 (*temp1, 0x0);
    temp2 = _mm_shuffle_epi32(temp4, 0xaa);
    temp4 = _mm_slli_si128 (*temp3, 0x4);
    *temp3 = _mm_xor_si128 (*temp3, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp3 = _mm_xor_si128 (*temp3, temp4);
    temp4 = _mm_slli_si128 (temp4, 0x4);
    *temp3 = _mm_xor_si128 (*temp3, temp4);
    *temp3 = _mm_xor_si128 (*temp3, temp2);
}


void aes_256_schedule( unsigned char* key, const unsigned char* userkey )
{
    __m128i temp1, temp2, temp3;
    __m128i *Key_Schedule = (__m128i*)key;
    temp1 = _mm_loadu_si128((__m128i*)userkey);
    temp3 = _mm_loadu_si128((__m128i*)(userkey+16));
    Key_Schedule[0] = temp1;
    Key_Schedule[1] = temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x01);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[2] = temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[3] = temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x02);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[4] = temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[5] = temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x04);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[6] = temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[7] = temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x08);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[8] = temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[9] = temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x10);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[10] = temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[11] = temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x20);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[12] = temp1;
    KEY_256_ASSIST_2(&temp1, &temp3);
    Key_Schedule[13] = temp3;
    temp2 = _mm_aeskeygenassist_si128 (temp3,0x40);
    KEY_256_ASSIST_1(&temp1, &temp2);
    Key_Schedule[14] = temp1;
}


void aes_128_encrypt(unsigned char* out, const unsigned char* in, const unsigned char* key)
{ 
    __m128i tmp; 
    tmp = _mm_loadu_si128 (&((__m128i*)in)[0]); 
    tmp = _mm_xor_si128 (tmp,((__m128i*)key)[0]);

    int j;
    for(j = 1; j < 10; j++)
    {
        tmp = _mm_aesenc_si128 (tmp,((__m128i*)key)[j]);
    }

    tmp = _mm_aesenclast_si128 (tmp,((__m128i*)key)[j]); 
    _mm_storeu_si128 (&((__m128i*)out)[0],tmp); 
}


void aes_192_encrypt(unsigned char* out, const unsigned char* in, const unsigned char* key)
{
    __m128i tmp; 
    tmp = _mm_loadu_si128 (&((__m128i*)in)[0]); 
    tmp = _mm_xor_si128 (tmp,((__m128i*)key)[0]); 
    
    int j;
    for(j = 1; j < 12; j++)
    {
        tmp = _mm_aesenc_si128 (tmp,((__m128i*)key)[j]);
    }
    
    tmp = _mm_aesenclast_si128 (tmp,((__m128i*)key)[j]); 
    _mm_storeu_si128 (&((__m128i*)out)[0],tmp); 
}


void aes_256_encrypt(unsigned char* out, const unsigned char* in, const unsigned char* key)
{
    __m128i tmp;
    tmp = _mm_loadu_si128 (&((__m128i*)in)[0]);
    tmp = _mm_xor_si128 (tmp,((__m128i*)key)[0]);

    int j;
    for(j = 1; j < 14; j++)
    {
        tmp = _mm_aesenc_si128 (tmp,((__m128i*)key)[j]);
    }

    tmp = _mm_aesenclast_si128 (tmp,((__m128i*)key)[j]); 
    _mm_storeu_si128 (&((__m128i*)out)[0],tmp); 
}

*/