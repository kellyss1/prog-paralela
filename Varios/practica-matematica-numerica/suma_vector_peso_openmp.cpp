#include "suma_vector_peso.h"

#include <omp.h>

double *suma_vector_peso_openmp(double *A, double *B, int n)
{
    double *vector_resultado = new double[n];
    
    #pragma omp parallel
    {

        int num_threads = omp_get_num_threads();
        int id_thread = omp_get_thread_num();

        int bloque = n / num_threads;
        int inicio = id_thread * bloque;
        int final = inicio + bloque;

        // arreglos impares
        if (id_thread == num_threads - 1)
        {
            final = n;
        }

        for (int i = inicio; i < final; i++)
        {
            vector_resultado[i] = (A[i] * 0.5) + (B[i] * 0.3);
        }
    }
    return vector_resultado;
}