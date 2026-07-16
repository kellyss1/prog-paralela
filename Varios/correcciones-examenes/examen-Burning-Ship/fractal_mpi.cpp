#include "fractal_mpi.h"
#include "palette.h"
#include <cstdint>
#include <cmath>
#include <algorithm>

extern int max_iteraciones;

// Kernel Burning Ship: z_{n+1} = (|Re(z_n)| + i|Im(z_n)|)^2 + c
uint32_t evaluar_burning_ship(double cx, double cy, int &out_iter)
{
    double zr = 0.0; // z_0 = 0
    double zi = 0.0;
    int iter = 0;

    // Condición de escape: zr^2 + zi^2 > 4 (equivalente a ||z|| > 2)
    while (iter < max_iteraciones && (zr * zr + zi * zi) <= 4.0)
    {
        // Aplicamos valor absoluto a los componentes real e imaginario
        double abs_zr = std::abs(zr);
        double abs_zi = std::abs(zi);

        // Elevamos al cuadrado el número complejo obtenido: (abs_zr + i*abs_zi)^2 + c
        double next_zr = abs_zr * abs_zr - abs_zi * abs_zi + cx;
        double next_zi = 2.0 * abs_zr * abs_zi + cy;

        zr = next_zr;
        zi = next_zi;
        iter++;
    }

    out_iter = iter;

    if (iter < max_iteraciones)
    {
        // El píxel escapó: asignamos color de la paleta de 16 tonos
        int index = iter % 16;
        return color_ramp[index];
    }

    return 0xFF000000; // Negro si no escapa (se queda atrapado)
}

void burning_ship_mpi(double x_min, double y_min, double x_max, double y_max, 
                      uint32_t width, uint32_t height, uint32_t row_start, uint32_t row_end, 
                      uint32_t *pixel_buffer, int *local_hist)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    // Inicializamos el histograma local de 16 bins en cero
    for (int b = 0; b < 16; b++) {
        local_hist[b] = 0;
    }

    for (uint32_t j = row_start; j < row_end; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            double x = x_min + i * dx;
            double y = y_max - j * dy; // Proyección del plano complejo

            int iter = 0;
            uint32_t color = evaluar_burning_ship(x, y, iter);
            pixel_buffer[(j - row_start) * width + i] = color;

            // Construcción del histograma local: solo por cada píxel que escapó
            if (iter < max_iteraciones)
            {
                int bin = (iter * 16) / max_iteraciones;
                if (bin >= 16) bin = 15;
                if (bin < 0)  bin = 0;
                local_hist[bin]++;
            }
        }
    }
}