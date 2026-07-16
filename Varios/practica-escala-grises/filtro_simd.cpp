#include <immintrin.h>
#include <stdint.h>

void filtro_simd(uint8_t* rgba_pixels, uint8_t* gray_pixels, int width, int height) {
    int total_pixeles = width * height;
    
    __m256d peso_R = _mm256_set1_pd(0.21);
    __m256d peso_G = _mm256_set1_pd(0.72);
    __m256d peso_B = _mm256_set1_pd(0.07);

    int i = 0;
    
    // Procesamiento en bloques de 4
    for (; i <= total_pixeles - 4; i += 4) {
        double temp_R[4] = { (double)rgba_pixels[i*4],     (double)rgba_pixels[(i+1)*4],     (double)rgba_pixels[(i+2)*4],     (double)rgba_pixels[(i+3)*4] };
        double temp_G[4] = { (double)rgba_pixels[i*4 + 1], (double)rgba_pixels[(i+1)*4 + 1], (double)rgba_pixels[(i+2)*4 + 1], (double)rgba_pixels[(i+3)*4 + 1] };
        double temp_B[4] = { (double)rgba_pixels[i*4 + 2], (double)rgba_pixels[(i+1)*4 + 2], (double)rgba_pixels[(i+2)*4 + 2], (double)rgba_pixels[(i+3)*4 + 2] };
        
        __m256d vec_R = _mm256_loadu_pd(temp_R);
        __m256d vec_G = _mm256_loadu_pd(temp_G);
        __m256d vec_B = _mm256_loadu_pd(temp_B);

        __m256d mul_R = _mm256_mul_pd(peso_R, vec_R);
        __m256d mul_G = _mm256_mul_pd(peso_G, vec_G);
        __m256d mul_B = _mm256_mul_pd(peso_B, vec_B);
        
        __m256d resultado_final = _mm256_add_pd(mul_R, _mm256_add_pd(mul_G, mul_B));

        double temp_gris[4];
        _mm256_storeu_pd(temp_gris, resultado_final);

        gray_pixels[i] = (uint8_t)temp_gris[0];
        gray_pixels[i + 1] = (uint8_t)temp_gris[1];
        gray_pixels[i + 2] = (uint8_t)temp_gris[2];
        gray_pixels[i + 3] = (uint8_t)temp_gris[3];
    }

    // Manejo de residuo para los píxeles sobrantes
    for (; i < total_pixeles; i++) {
        uint8_t R = rgba_pixels[i * 4];
        uint8_t G = rgba_pixels[i * 4 + 1];
        uint8_t B = rgba_pixels[i * 4 + 2];
        
        gray_pixels[i] = 0.21 * R + 0.72 * G + 0.07 * B;
    }
}