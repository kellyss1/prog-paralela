#include "suma_vector_peso.h"

#include <immintrin.h>
#include <vector>

double *suma_vector_peso_simd(double *A, double *B, int n)
{
    double* vector_resultado = new double[n];

    __m256d peso_A = _mm256_set1_pd(0.5);
    __m256d peso_B = _mm256_set1_pd(0.3);

    int i=0;

    for(;i <= n-4; i+=4) {
        __m256d vec_A = _mm256_loadu_pd(&A[i]);
        __m256d vec_B = _mm256_loadu_pd(&B[i]);

        __m256d mult_A = _mm256_mul_pd(vec_A, peso_A);
        __m256d mult_B = _mm256_mul_pd(vec_B, peso_B);

        __m256d suma_resultados = _mm256_add_pd(mult_A, mult_B);

        _mm256_storeu_pd(&vector_resultado[i], suma_resultados);
    }

    for(; i < n; i++) {
        vector_resultado[i] = (A[i] * 0.5) + (B[i] * 0.3);
    }
    return vector_resultado;
}