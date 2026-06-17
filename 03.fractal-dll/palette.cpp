#include "palette.h"

/*		
#FFFFCC
#FFF5B5
#FFEC9D
#FEE187
#FED470
#FEBF5A
#FEAB49
#FD9740
#FD7C37
#FC5B2E
#F43D25
#E6211E
#D41020
#C00225
#A10026
#800026
*/

uint32_t bswap32(uint32_t a) {
    return 
    ((a & 0x000000ff) << 24) | 
    ((a & 0x0000ff00) << 8) |
    ((a & 0x00ff0000) >> 8) |
    ((a & 0xff000000) >> 24);  
    
}

std::vector<uint32_t> color_ramp = {
// Degradado verde -> negro (16 pasos). Los bytes en C++ están invertidos ABGR.
    bswap32(0xE6FFE6FF), //1
    bswap32(0xCCFFCCFF), //2
    bswap32(0xB3FFB3FF), //3
    bswap32(0x99FF99FF), //4
    bswap32(0x80FF80FF), //5
    bswap32(0x66FF66FF), //6
    bswap32(0x4DFF4DFF), //7
    bswap32(0x33FF33FF), //8
    bswap32(0x1AFF1AFF), //9
    bswap32(0x00FF00FF), //10
    bswap32(0x00E600FF), //11
    bswap32(0x00CC00FF), //12
    bswap32(0x009900FF), //13
    bswap32(0x006600FF), //14
    bswap32(0x003300FF), //15
    bswap32(0x000000FF)  //16
};