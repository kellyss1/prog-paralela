#ifndef FILTROS
#define FILTROS

#include <cstdint>

void filtro_simd(uint8_t* rgba_pixels, uint8_t* gray_pixels, int width, int height);
void filtro_openmp(uint8_t* rgba_pixels, uint8_t* gray_pixels, int width, int height);

#endif // FILTROS