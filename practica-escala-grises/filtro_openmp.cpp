#include "filtros.h"

#include <omp.h>

void filtro_openmp(uint8_t *rgba_pixels, uint8_t *gray_pixels, int width, int height)
{

#pragma omp parallel
    {
        int num_threads = omp_get_num_threads();
        int id_thread = omp_get_thread_num();

        int total_pixeles = width * height;
        int bloque = total_pixeles / num_threads;

        int inicio = id_thread * bloque;
        int final = inicio + bloque;

        // Para arreglos impares
        if(id_thread == num_threads -1) {
            final = total_pixeles;
        }

        for (int i = inicio; i < final; i++)
        {
            uint8_t R = rgba_pixels[i * 4];
            uint8_t G = rgba_pixels[i * 4 + 1];
            uint8_t B = rgba_pixels[i * 4 + 2];

            gray_pixels[i] = 0.21 * R + 0.72 * G + 0.07 * B;
        }
    }
}