#include "fractal_mpi.h"
#include "palette.h"
#include <cstdint>
#include <complex>
#include <cmath>

extern int max_iteraciones;

// Las 3 raíces cúbicas de la unidad
const std::complex<double> w0(1.0, 0.0);
const std::complex<double> w1(-0.5, 0.8660254037844386);  // -0.5 + i*sqrt(3)/2
const std::complex<double> w2(-0.5, -0.8660254037844386); // -0.5 - i*sqrt(3)/2

const double EPSILON = 1e-4;

// Kernel Newton-Raphson: z_{n+1} = z_n - (z_n^3 - 1) / (3 * z_n^2)
uint32_t evaluar_newton(double x, double y, unsigned long long &out_iters)
{
    std::complex<double> z(x, y);
    int iter = 0;

    while (iter < max_iteraciones)
    {
        // 1. Condición de escape: |z| > 2 (equivalente a |z|^2 > 4)
        if (std::norm(z) > 4.0)
        {
            out_iters += iter;
            return 0xFF000000; // Negro
        }

        // 2. Condición de convergencia: |z - w_k| < epsilon (equivalente a |z - w_k|^2 < epsilon^2)
        if (std::norm(z - w0) < EPSILON * EPSILON)
        {
            out_iters += iter;
            int index = (0 * 5 + iter) % 16;
            return color_ramp[index];
        }
        if (std::norm(z - w1) < EPSILON * EPSILON)
        {
            out_iters += iter;
            int index = (1 * 5 + iter) % 16;
            return color_ramp[index];
        }
        if (std::norm(z - w2) < EPSILON * EPSILON)
        {
            out_iters += iter;
            int index = (2 * 5 + iter) % 16;
            return color_ramp[index];
        }

        // 3. Newton-Raphson step
        std::complex<double> z2 = z * z;
        std::complex<double> den = 3.0 * z2;

        // Evitar división por cero
        if (std::norm(den) < 1e-18) 
        {
            break;
        }

        z = z - (z2 * z - 1.0) / den;
        iter++;
    }

    out_iters += iter;
    return 0xFF000000; // Negro si no converge
}

void newton_mpi(double x_min, double y_min, double x_max, double y_max, 
                uint32_t width, uint32_t height, uint32_t row_start, uint32_t row_end, 
                uint32_t *pixel_buffer, unsigned long long &total_iters)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;
    total_iters = 0;

    for (uint32_t j = row_start; j < row_end; j++)
    {
        for (uint32_t i = 0; i < width; i++)
        {
            double x = x_min + i * dx;
            double y = y_max - j * dy;

            uint32_t color = evaluar_newton(x, y, total_iters);
            pixel_buffer[(j - row_start) * width + i] = color;
        }
    }
}