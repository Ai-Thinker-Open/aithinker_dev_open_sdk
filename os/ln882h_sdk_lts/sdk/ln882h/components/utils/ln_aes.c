
/**
 * @file     ln_aes.c
 * @author   LightningSemi
 * @brief    This file provides ADC function.
 * @version  1.0.0
 * @date     2018-05-01
 * 
 * @copyright Copyright (c) 2018-2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_aes.h"

#define RIJNDAEL_BLOCKSIZE 16 // 128 bits block size

#define ROTBYTE(x) (((x) >> 8) | (((x) & 0xFF) << 24))
#define ROTRBYTE(x) (((x) << 8) | (((x) >> 24) & 0xFF))
#define SUBBYTE(x, box) (((box)[((x) & 0xFF)]) | \
                        ((box)[(((x) >> 8) & 0xFF)] << 8) | \
                        ((box)[(((x) >> 16) & 0xFF)] << 16) | \
                        ((box)[(((x) >> 24) & 0xFF)] << 24))

#if (!(LN_SW_AES_USING_ROM_CODE))
/* These tables combine both the S-boxes and the mixcolumn transformation,   */
/* so that we can perform a round's encryption or by means of four table     */
/* lookups and four XOR's per column of state.                               */
static const uint32_t g_dtbl[] = {
  0xA56363C6, 0x847C7CF8, 0x997777EE, 0x8D7B7BF6,
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
  0xCBB0B07B, 0xFC5454A8, 0xD6BBBB6D, 0x3A16162C,
};

static const uint32_t g_itbl[] = {
  0x50A7F451, 0x5365417E, 0xC3A4171A, 0x965E273A,
  0xCB6BAB3B, 0xF1459D1F, 0xAB58FAAC, 0x9303E34B,
  0x55FA3020, 0xF66D76AD, 0x9176CC88, 0x254C02F5,
  0xFCD7E54F, 0xD7CB2AC5, 0x80443526, 0x8FA362B5,
  0x495AB1DE, 0x671BBA25, 0x980EEA45, 0xE1C0FE5D,
  0x02752FC3, 0x12F04C81, 0xA397468D, 0xC6F9D36B,
  0xE75F8F03, 0x959C9215, 0xEB7A6DBF, 0xDA595295,
  0x2D83BED4, 0xD3217458, 0x2969E049, 0x44C8C98E,
  0x6A89C275, 0x78798EF4, 0x6B3E5899, 0xDD71B927,
  0xB64FE1BE, 0x17AD88F0, 0x66AC20C9, 0xB43ACE7D,
  0x184ADF63, 0x82311AE5, 0x60335197, 0x457F5362,
  0xE07764B1, 0x84AE6BBB, 0x1CA081FE, 0x942B08F9,
  0x58684870, 0x19FD458F, 0x876CDE94, 0xB7F87B52,
  0x23D373AB, 0xE2024B72, 0x578F1FE3, 0x2AAB5566,
  0x0728EBB2, 0x03C2B52F, 0x9A7BC586, 0xA50837D3,
  0xF2872830, 0xB2A5BF23, 0xBA6A0302, 0x5C8216ED,
  0x2B1CCF8A, 0x92B479A7, 0xF0F207F3, 0xA1E2694E,
  0xCDF4DA65, 0xD5BE0506, 0x1F6234D1, 0x8AFEA6C4,
  0x9D532E34, 0xA055F3A2, 0x32E18A05, 0x75EBF6A4,
  0x39EC830B, 0xAAEF6040, 0x069F715E, 0x51106EBD,
  0xF98A213E, 0x3D06DD96, 0xAE053EDD, 0x46BDE64D,
  0xB58D5491, 0x055DC471, 0x6FD40604, 0xFF155060,
  0x24FB9819, 0x97E9BDD6, 0xCC434089, 0x779ED967,
  0xBD42E8B0, 0x888B8907, 0x385B19E7, 0xDBEEC879,
  0x470A7CA1, 0xE90F427C, 0xC91E84F8, 0x00000000,
  0x83868009, 0x48ED2B32, 0xAC70111E, 0x4E725A6C,
  0xFBFF0EFD, 0x5638850F, 0x1ED5AE3D, 0x27392D36,
  0x64D90F0A, 0x21A65C68, 0xD1545B9B, 0x3A2E3624,
  0xB1670A0C, 0x0FE75793, 0xD296EEB4, 0x9E919B1B,
  0x4FC5C080, 0xA220DC61, 0x694B775A, 0x161A121C,
  0x0ABA93E2, 0xE52AA0C0, 0x43E0223C, 0x1D171B12,
  0x0B0D090E, 0xADC78BF2, 0xB9A8B62D, 0xC8A91E14,
  0x8519F157, 0x4C0775AF, 0xBBDD99EE, 0xFD607FA3,
  0x9F2601F7, 0xBCF5725C, 0xC53B6644, 0x347EFB5B,
  0x7629438B, 0xDCC623CB, 0x68FCEDB6, 0x63F1E4B8,
  0xCADC31D7, 0x10856342, 0x40229713, 0x2011C684,
  0x7D244A85, 0xF83DBBD2, 0x1132F9AE, 0x6DA129C7,
  0x4B2F9E1D, 0xF330B2DC, 0xEC52860D, 0xD0E3C177,
  0x6C16B32B, 0x99B970A9, 0xFA489411, 0x2264E947,
  0xC48CFCA8, 0x1A3FF0A0, 0xD82C7D56, 0xEF903322,
  0xC74E4987, 0xC1D138D9, 0xFEA2CA8C, 0x360BD498,
  0xCF81F5A6, 0x28DE7AA5, 0x268EB7DA, 0xA4BFAD3F,
  0xE49D3A2C, 0x0D927850, 0x9BCC5F6A, 0x62467E54,
  0xC2138DF6, 0xE8B8D890, 0x5EF7392E, 0xF5AFC382,
  0xBE805D9F, 0x7C93D069, 0xA92DD56F, 0xB31225CF,
  0x3B99ACC8, 0xA77D1810, 0x6E639CE8, 0x7BBB3BDB,
  0x097826CD, 0xF418596E, 0x01B79AEC, 0xA89A4F83,
  0x656E95E6, 0x7EE6FFAA, 0x08CFBC21, 0xE6E815EF,
  0xD99BE7BA, 0xCE366F4A, 0xD4099FEA, 0xD67CB029,
  0xAFB2A431, 0x31233F2A, 0x3094A5C6, 0xC066A235,
  0x37BC4E74, 0xA6CA82FC, 0xB0D090E0, 0x15D8A733,
  0x4A9804F1, 0xF7DAEC41, 0x0E50CD7F, 0x2FF69117,
  0x8DD64D76, 0x4DB0EF43, 0x544DAACC, 0xDF0496E4,
  0xE3B5D19E, 0x1B886A4C, 0xB81F2CC1, 0x7F516546,
  0x04EA5E9D, 0x5D358C01, 0x737487FA, 0x2E410BFB,
  0x5A1D67B3, 0x52D2DB92, 0x335610E9, 0x1347D66D,
  0x8C61D79A, 0x7A0CA137, 0x8E14F859, 0x893C13EB,
  0xEE27A9CE, 0x35C961B7, 0xEDE51CE1, 0x3CB1477A,
  0x59DFD29C, 0x3F73F255, 0x79CE1418, 0xBF37C773,
  0xEACDF753, 0x5BAAFD5F, 0x146F3DDF, 0x86DB4478,
  0x81F3AFCA, 0x3EC468B9, 0x2C342438, 0x5F40A3C2,
  0x72C31D16, 0x0C25E2BC, 0x8B493C28, 0x41950DFF,
  0x7101A839, 0xDEB30C08, 0x9CE4B4D8, 0x90C15664,
  0x6184CB7B, 0x70B632D5, 0x745C6C48, 0x4257B8D0,
};


/* Needed only for the key schedule and for final rounds */
static const uint8_t g_sbox[256] = {
  99, 124, 119, 123, 242, 107, 111, 197,  48,   1, 103,  43, 254, 215, 171,
  118, 202, 130, 201, 125, 250,  89,  71, 240, 173, 212, 162, 175, 156, 164,
  114, 192, 183, 253, 147,  38,  54,  63, 247, 204,  52, 165, 229, 241, 113,
  216,  49,  21,  4, 199,  35, 195,  24, 150,   5, 154,   7,  18, 128, 226,
  235,  39, 178, 117,  9, 131,  44,  26,  27, 110,  90, 160,  82,  59, 214,
  179,  41, 227,  47, 132, 83, 209,   0, 237,  32, 252, 177,  91, 106, 203,
  190,  57,  74,  76,  88, 207, 208, 239, 170, 251,  67,  77,  51, 133,  69,
  249,   2, 127,  80,  60, 159, 168, 81, 163,  64, 143, 146, 157,  56, 245,
  188, 182, 218,  33,  16, 255, 243, 210, 205,  12,  19, 236,  95, 151,  68,
  23, 196, 167, 126,  61, 100,  93,  25, 115, 96, 129,  79, 220,  34,  42,
  144, 136,  70, 238, 184,  20, 222,  94,  11, 219, 224,  50,  58,  10,  73,
  6,  36,  92, 194, 211, 172,  98, 145, 149, 228, 121, 231, 200,  55, 109,
  141, 213,  78, 169, 108,  86, 244, 234, 101, 122, 174,   8, 186, 120,  37,
  46,  28, 166, 180, 198, 232, 221, 116,  31,  75, 189, 139, 138, 112,  62,
  181, 102,  72,   3, 246,  14,  97,  53,  87, 185, 134, 193,  29, 158, 225,
  248, 152,  17, 105, 217, 142, 148, 155,  30, 135, 233, 206,  85,  40, 223,
  140, 161, 137,  13, 191, 230,  66, 104,  65, 153,  45,  15, 176,  84, 187,
  22,
};

static const uint8_t g_isbox[256] = {
  82,   9, 106, 213,  48,  54, 165,  56, 191,  64, 163, 158, 129, 243, 215,
  251, 124, 227,  57, 130, 155,  47, 255, 135,  52, 142,  67,  68, 196, 222,
  233, 203, 84, 123, 148,  50, 166, 194,  35,  61, 238,  76, 149,  11,  66,
  250, 195,  78,  8,  46, 161, 102,  40, 217,  36, 178, 118,  91, 162,  73,
  109, 139, 209,  37, 114, 248, 246, 100, 134, 104, 152,  22, 212, 164,  92,
  204,  93, 101, 182, 146, 108, 112,  72,  80, 253, 237, 185, 218,  94,  21,
  70,  87, 167, 141, 157, 132, 144, 216, 171,   0, 140, 188, 211,  10, 247,
  228,  88,   5, 184, 179,  69,   6, 208,  44,  30, 143, 202,  63,  15,   2,
  193, 175, 189,   3,   1,  19, 138, 107,  58, 145,  17,  65,  79, 103, 220,
  234, 151, 242, 207, 206, 240, 180, 230, 115, 150, 172, 116,  34, 231, 173,
  53, 133, 226, 249,  55, 232,  28, 117, 223, 110,  71, 241,  26, 113,  29,
  41, 197, 137, 111, 183,  98,  14, 170,  24, 190,  27, 252,  86,  62,  75,
  198, 210, 121,  32, 154, 219, 192, 254, 120, 205,  90, 244,  31, 221, 168,
  51, 136,   7, 199,  49, 177,  18,  16,  89,  39, 128, 236,  95,  96,  81,
  127, 169,  25, 181,  74,  13,  45, 229, 122, 159, 147, 201, 156, 239, 160,
  224,  59,  77, 174,  42, 245, 176, 200, 235, 187,  60, 131,  83, 153,  97,
  23,  43,   4, 126, 186, 119, 214,  38, 225, 105,  20,  99,  85,  33,  12,
  125,
};

/* Used only by the key schedule */
static const uint8_t g_logtable[256] = {
  0,   0,  25,   1,  50,   2,  26, 198,  75, 199,  27, 104,  51, 238, 223,  3,
  100,   4, 224,  14,  52, 141, 129, 239,  76, 113,   8, 200, 248, 105,  28,
  193, 125, 194,  29, 181, 249, 185,  39, 106,  77, 228, 166, 114, 154, 201,
  9, 120, 101,  47, 138,   5,  33,  15, 225,  36,  18, 240, 130,  69,  53,
  147, 218, 142, 150, 143, 219, 189,  54, 208, 206, 148,  19,  92, 210, 241,
  64,  70, 131,  56, 102, 221, 253,  48, 191,   6, 139,  98, 179,  37, 226,
  152,  34, 136, 145,  16, 126, 110,  72, 195, 163, 182,  30,  66,  58, 107,
  40,  84, 250, 133,  61, 186, 43, 121,  10,  21, 155, 159,  94, 202,  78,
  212, 172, 229, 243, 115, 167,  87, 175,  88, 168,  80, 244, 234, 214, 116,
  79, 174, 233, 213, 231, 230, 173, 232, 44, 215, 117, 122, 235,  22,  11,
  245,  89, 203,  95, 176, 156, 169,  81, 160, 127,  12, 246, 111,  23, 196,
  73, 236, 216,  67,  31,  45, 164, 118, 123, 183, 204, 187,  62,  90, 251,
  96, 177, 134,  59,  82, 161, 108, 170,  85,  41, 157, 151, 178, 135, 144,
  97, 190, 220, 252, 188, 149, 207, 205,  55,  63,  91, 209, 83,  57, 132,
  60,  65, 162, 109,  71,  20,  42, 158,  93,  86, 242, 211, 171, 68,  17,
  146, 217,  35,  32,  46, 137, 180, 124, 184,  38, 119, 153, 227, 165, 103,
  74, 237, 222, 197,  49, 254,  24,  13,  99, 140, 128, 192, 247, 112,   7,
};

static const uint8_t g_alogtable[256] = {
  1,   3,   5,  15,  17,  51,  85, 255,  26,  46, 114, 150, 161, 248,  19,
  53, 95, 225,  56,  72, 216, 115, 149, 164, 247,   2,   6,  10,  30,  34,
  102, 170, 229,  52,  92, 228,  55,  89, 235,  38, 106, 190, 217, 112, 144,
  171, 230,  49,  83, 245,   4,  12,  20,  60,  68, 204,  79, 209, 104, 184,
  211, 110, 178, 205,  76, 212, 103, 169, 224,  59,  77, 215,  98, 166, 241,
  8,  24,  40, 120, 136, 131, 158, 185, 208, 107, 189, 220, 127, 129, 152,
  179, 206,  73, 219, 118, 154, 181, 196,  87, 249,  16,  48,  80, 240,  11,
  29,  39, 105, 187, 214,  97, 163, 254,  25,  43, 125, 135, 146, 173, 236,
  47, 113, 147, 174, 233,  32,  96, 160, 251,  22,  58,  78, 210, 109, 183,
  194,  93, 231,  50,  86, 250,  21,  63,  65, 195,  94, 226,  61,  71, 201,
  64, 192,  91, 237,  44, 116, 156, 191, 218, 117, 159, 186, 213, 100, 172,
  239,  42, 126, 130, 157, 188, 223, 122, 142, 137, 128, 155, 182, 193,  88,
  232,  35, 101, 175, 234,  37, 111, 177, 200,  67, 197,  84, 252,  31,  33,
  99, 165, 244,   7,   9,  27,  45, 119, 153, 176, 203,  70, 202,  69, 207,
  74, 222, 121, 139, 134, 145, 168, 227,  62,  66, 198,  81, 243,  14,  18,
  54,  90, 238,  41, 123, 141, 140, 143, 138, 133, 148, 167, 242,  13,  23,
  57,  75, 221, 124, 132, 151, 162, 253,  28,  36, 108, 180, 199,  82, 246, 1,
};

static const uint32_t g_idxe[4][4] = {
  { 0, 1, 2, 3 },
  { 1, 2, 3, 0 },
  { 2, 3, 0, 1 },
  { 3, 0, 1, 2 }
};

static const uint32_t g_idxd[4][4] = {
  { 0, 1, 2, 3 },
  { 3, 0, 1, 2 },
  { 2, 3, 0, 1 },
  { 1, 2, 3, 0 }
};

/*****************************************************************************/
/* Static Function Declarations                                              */
/*****************************************************************************/

static uint8_t xtime(uint8_t a);
static uint8_t mul(uint8_t a, uint8_t b);
static void   inv_mix_column(uint32_t *a, uint32_t *b);
static void   key_addition_8to32(uint8_t *txt, uint32_t *key, uint32_t *out);
static void   key_addition32(uint32_t *txt, uint32_t *key, uint32_t *out);
static void   key_addition32to8(uint32_t *txt, uint32_t *key, uint8_t *out);

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : aes_setup                                             */
/*                                                                           */
/*  Description      : This performs Rijndael's key scheduling algorithm.    */
/*                                                                           */
/*  Inputs           : 1) Key Context                                        */
/*                     2) Key Size                                           */
/*                     3) Key                                                */
/*                                                                           */
/*  Globals          : None                                                  */
/*  Processing       : None                                                  */
/*  Outputs          : None                                                  */
/*  Returns          : None                                                  */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

void ln_aes_setup(ln_aes_context_t *ctx, uint8_t keysize, const uint8_t *key)
{
    uint32_t nk      = 0;
    uint32_t nr      = 0;
    uint32_t i       = 0;
    uint32_t lastkey = 0;
    uint32_t temp    = 0;
    uint32_t rcon    = 0;

    /* Truncate keysizes to the valid key sizes provided by Rijndael */
    if(keysize >= 32)
    {
        nk = 8;
        nr = 14;
    }
    else if (keysize >= 24)
    {
        nk = 6;
        nr = 12;
    }
    else
    {
        nk = 4;
        nr = 10;
    }

    lastkey = (RIJNDAEL_BLOCKSIZE/4) * (nr + 1);
    ctx->nrounds = nr;
    rcon = 1;

    for(i = 0; i < nk; i++)
    {
        ctx->keys[i] = key[i * 4] + (key[i * 4 + 1] << 8) +
                       (key[i * 4 + 2] << 16) + (key[i * 4 + 3] << 24);
    }

    for(i = nk; i < lastkey; i++)
    {
        temp = ctx->keys[i-1];
        if(i % nk == 0)
        {
            temp = SUBBYTE(ROTBYTE(temp), g_sbox) ^ rcon;
            rcon = (uint32_t)xtime((uint8_t)(rcon & 0xFF));
        }
        else if(nk > 6 && (i % nk) == 4)
        {
            temp = SUBBYTE(temp, g_sbox);
        }

        ctx->keys[i] = ctx->keys[i - nk] ^ temp;
    }

    /* Generate the inverse keys */
    for (i = 0; i < 4; i++)
    {
        ctx->ikeys[i] = ctx->keys[i];
        ctx->ikeys[lastkey - 4 + i] = ctx->keys[lastkey - 4 + i];
    }

    for (i = 4; i < lastkey - 4; i += 4)
        inv_mix_column(&(ctx->keys[i]), &(ctx->ikeys[i]));
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : aes_encrypt                                           */
/*                                                                           */
/*  Description      : This performs Rijndael encryption on a block of 16    */
/*                     bytes of data.                                        */
/*                                                                           */
/*  Inputs           : 1) Key Context                                        */
/*                     2) Pointer to plain text                              */
/*                     3) Pointer to cipher text                             */
/*                     Note: The memory areas of the plaintext and the       */
/*                     ciphertext can overlap.                               */
/*                                                                           */
/*  Globals          : None                                                  */
/*                                                                           */
/*  Processing       : Before this function can be used, aes_setup must be   */
/*                     used in order to initialize Rijndael's key schedule.  */
/*                     Encryption is done with the Rijndael algorithm.       */
/*                                                                           */
/*  Outputs          : None                                                  */
/*  Returns          : None                                                  */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

void ln_aes_encrypt(ln_aes_context_t *ctx, const uint8_t *plaintext, uint8_t *ciphertext)
{
    uint32_t r = 0;
    uint32_t j = 0;
    uint32_t e = 0;
    uint32_t wtxt[4];
    uint32_t t[4];

    key_addition_8to32((uint8_t *)plaintext, &(ctx->keys[0]), wtxt);

    for(r = 1; r < ctx->nrounds; r++)
    {
        for (j=0; j<4; j++)
        {
            t[j] = g_dtbl[wtxt[j] & 0xFF]
                 ^ ROTRBYTE(g_dtbl[(wtxt[g_idxe[1][j]] >> 8) & 0xFF]
                 ^ ROTRBYTE(g_dtbl[(wtxt[g_idxe[2][j]] >> 16) & 0xFF]
                 ^ ROTRBYTE(g_dtbl[(wtxt[g_idxe[3][j]] >> 24) & 0xFF])));
        }

        key_addition32(t, &(ctx->keys[r * 4]), wtxt);
    }

    /* Last round is special: there is no mixcolumn */
    for(j = 0; j < 4; j++)
    {
        e = wtxt[j] & 0xFF;
        e |= (wtxt[g_idxe[1][j]]) & ((uint32_t)0xFF << 8);
        e |= (wtxt[g_idxe[2][j]]) & ((uint32_t)0xFF << 16);
        e |= (wtxt[g_idxe[3][j]]) & ((uint32_t)0xFF << 24);
        t[j] = e;
    }

    for(j = 0; j < 4; j++)
        t[j] = SUBBYTE(t[j], g_sbox);

    key_addition32to8(t, &(ctx->keys[4*ctx->nrounds]), ciphertext);
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : aes_decrypt                                           */
/*                                                                           */
/*  Description      : This performs Rijndael decryption on a block of 16    */
/*                     bytes of data.                                        */
/*                                                                           */
/*  Inputs           : 1) Key Context                                        */
/*                     2) Pointer to plain text                              */
/*                     3) Pointer to cipher text                             */
/*                     Note: The memory areas of the plaintext and the       */
/*                     ciphertext can overlap.                               */
/*                                                                           */
/*  Globals          : None                                                  */
/*                                                                           */
/*  Processing       : Before this function can be used, aes_setup must be   */
/*                     used in order to initialize Rijndael's key schedule.  */
/*                     Decryption is done with the Rijndael algorithm.       */
/*                                                                           */
/*  Outputs          : None                                                  */
/*  Returns          : None                                                  */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

void ln_aes_decrypt(ln_aes_context_t *ctx, const uint8_t *ciphertext, uint8_t *plaintext)
{
    uint32_t r = 0;
    uint32_t j = 0;
    uint32_t e = 0;
    uint32_t wtxt[4];
    uint32_t t[4];

    key_addition_8to32((uint8_t *)ciphertext, &(ctx->ikeys[4*ctx->nrounds]), wtxt);
    for(r = ctx->nrounds - 1; r > 0;  r--)
    {
        for(j = 0; j < 4; j++)
        {
            t[j] = g_itbl[wtxt[j] & 0xFF]
                 ^ ROTRBYTE(g_itbl[(wtxt[g_idxd[1][j]] >> 8) & 0xFF]
                 ^ ROTRBYTE(g_itbl[(wtxt[g_idxd[2][j]] >> 16) & 0xFF]
                 ^ ROTRBYTE(g_itbl[(wtxt[g_idxd[3][j]] >> 24) & 0xFF])));
        }

        key_addition32(t, &(ctx->ikeys[r * 4]), wtxt);
    }

    /* Last round is special: there is no mixcolumn */
    for(j = 0; j < 4; j++)
    {
        e = wtxt[j] & 0xFF;
        e |= (wtxt[g_idxd[1][j]]) & ((uint32_t)0xFF << 8);
        e |= (wtxt[g_idxd[2][j]]) & ((uint32_t)0xFF << 16);
        e |= (wtxt[g_idxd[3][j]]) & ((uint32_t)0xFF << 24);
        t[j] = e;
    }

    for(j = 0; j < 4; j++)
        t[j] = SUBBYTE(t[j], g_isbox);

    key_addition32to8(t, &(ctx->ikeys[0]), plaintext);
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : xtime                                                 */
/*                                                                           */
/*  Description      : Masks the input value after required shifts           */
/*                                                                           */
/*  Inputs           : None                                                  */
/*  Globals          : None                                                  */
/*  Processing       : None                                                  */
/*  Outputs          : None                                                  */
/*                                                                           */
/*  Returns          : The shifted and the masked value                      */
/*                                                                           */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

static uint8_t xtime(uint8_t a)
{
  uint8_t b = 0;

  b = (a & 0x80) ? 0x1B : 0;
  a <<= 1;
  a ^= b;

  return a;
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : mul                                                   */
/*                                                                           */
/*  Description      : Performs the multiplication using the Log Table       */
/*                                                                           */
/*  Inputs           : 1) Multiplier 1                                       */
/*                     2) Multiplier 2                                       */
/*                                                                           */
/*  Globals          : None                                                  */
/*  Processing       : None                                                  */
/*  Outputs          : None                                                  */
/*  Returns          : The product of the two given numbers                  */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

static uint8_t mul(uint8_t a, uint8_t b)
{
  if (a && b)
      return g_alogtable[(g_logtable[a] + g_logtable[b])%255];

  return 0;
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : inv_mix_column                                        */
/*                                                                           */
/*  Description      : Performs the Inverse of the column after mixing it    */
/*                                                                           */
/*  Inputs           : 1) Input column                                       */
/*                     2) Output column                                      */
/*                                                                           */
/*  Globals          : None                                                  */
/*  Processing       : None                                                  */
/*  Outputs          : None                                                  */
/*  Returns          : None                                                  */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

static void inv_mix_column(uint32_t *a, uint32_t *b)
{
  uint32_t i = 0;
  uint32_t j = 0;
  uint8_t c[4][4];

  for(j = 0; j < 4; j++)
  {
      for(i = 0; i < 4; i++)
      {
          c[j][i] = mul(0xE, (uint8_t)((a[j] >> i * 8) & 0xFF))
                  ^ mul(0xB, (uint8_t)((a[j] >> ((i + 1) % 4) * 8) & 0xFF))
                  ^ mul(0xD, (uint8_t)((a[j] >> ((i + 2) % 4) * 8) & 0xFF))
                  ^ mul(0x9, (uint8_t)((a[j] >> ((i + 3) % 4) * 8) & 0xFF));
    }
  }

  for(i = 0; i < 4; i++)
  {
      b[i] = 0;

      for(j = 0; j < 4; j++)
          b[i] |= c[i][j] << (j * 8);
  }
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : key_addition_8to32                                    */
/*                                                                           */
/*  Description      : Adds the key and the text                             */
/*                                                                           */
/*  Inputs           : 1) Pointer to the text                                */
/*                     2) Pointer to the key                                 */
/*                     3) Pointer to the output Key                          */
/*                                                                           */
/*  Globals          : None                                                  */
/*  Processing       : None                                                  */
/*  Outputs          : None                                                  */
/*  Returns          : None                                                  */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

static void key_addition_8to32(uint8_t *txt, uint32_t *key, uint32_t *out)
{
  uint32_t i   = 0;
  uint32_t j   = 0;
  uint32_t val = 0;
  uint8_t *ptr = 0;

  ptr = txt;
  for(i = 0; i < 4; i++)
  {
    val = 0;

    for(j = 0; j < 4; j++)
        val |= (*ptr++ << 8 * j);

    out[i] = key[i] ^ val;
  }
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : key_addition32                                        */
/*                                                                           */
/*  Description      : Adds the key and the text                             */
/*                                                                           */
/*  Inputs           : 1) Pointer to the text                                */
/*                     2) Pointer to the key                                 */
/*                     3) Pointer to the output Key                          */
/*                                                                           */
/*  Globals          : None                                                  */
/*  Processing       : None                                                  */
/*  Outputs          : None                                                  */
/*  Returns          : None                                                  */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

static void key_addition32(uint32_t *txt, uint32_t *key, uint32_t *out)
{
  uint32_t i = 0;

  for(i = 0; i < 4; i++)
    out[i] = key[i] ^ txt[i];
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name    : key_addition32to8                                     */
/*                                                                           */
/*  Description      : Adds the key and the text                             */
/*                                                                           */
/*  Inputs           : 1) Pointer to the text                                */
/*                     2) Pointer to the key                                 */
/*                     3) Pointer to the output Key                          */
/*                                                                           */
/*  Globals          : None                                                  */
/*  Processing       : None                                                  */
/*  Outputs          : None                                                  */
/*  Returns          : None                                                  */
/*  Issues           : None                                                  */
/*                                                                           */
/*****************************************************************************/

static void key_addition32to8(uint32_t *txt, uint32_t *key,
                                         uint8_t *out)
{
  uint32_t i   = 0;
  uint32_t j   = 0;
  uint32_t val = 0;
  uint8_t *ptr = 0;

  ptr = out;

  for(i = 0; i < 4; i++)
  {
    val = txt[i] ^ key[i];

    for(j = 0; j < 4; j++)
      *ptr++ = (val >> 8 * j) & 0xFF;
  }
}
#else
#include "ln882h_rom_fun.h"

typedef void (*rom_func_aes_setup)(ln_aes_context_t *, uint8_t, const uint8_t *);
typedef void (*rom_func_aes_encrypt)(ln_aes_context_t *, const uint8_t *, uint8_t *);
typedef void (*rom_func_aes_decrypt)(ln_aes_context_t *, const uint8_t *, uint8_t *);

void ln_aes_setup(ln_aes_context_t *ctx, uint8_t keysize, const uint8_t *key)
{
    rom_func_aes_setup aes_setup = (rom_func_aes_setup)ROM_FUN_AES_SETUP;
    aes_setup(ctx, keysize, key);
}

void ln_aes_encrypt(ln_aes_context_t *ctxt, const uint8_t *ptext, uint8_t *ctext)
{
    rom_func_aes_encrypt aes_enc = (rom_func_aes_encrypt)ROM_FUN_AES_ENCRYPT;
    aes_enc(ctxt, ptext, ctext);
}

void ln_aes_decrypt(ln_aes_context_t *ctxt, const uint8_t *ctext, uint8_t *ptext)
{
    rom_func_aes_decrypt aes_dec = (rom_func_aes_decrypt)ROM_FUN_AES_DECRYPT;
    aes_dec(ctxt, ctext, ptext);
}
#endif /* !(LN_SW_AES_USING_ROM_CODE) */
