#include "producto_escalar.h"

#include <omp.h>
#include <vector>

double producto_escalar_openmp(double *x, double *y, int n)
{

    int num_threads = omp_get_max_threads();
    std::vector<double> subtotales(num_threads, 0.0);

#pragma omp parallel
    {
        int id_thread = omp_get_thread_num();

        int bloque = n / num_threads;
        int inicio = id_thread * bloque;
        int final = inicio + bloque;

        if (id_thread == num_threads-1)
        {
            final = n;
        }

        for (int i = inicio; i < final; i++)
        {
            subtotales[id_thread] += x[i] * y[i];
        }
    }

    double resultado_final = 0;
    for (double val : subtotales)
    {
        resultado_final += val;
    }

    return resultado_final;
}