#include "producto_escalar.h"

#include <immintrin.h>

double producto_escalar_simd(double* x, double* y, int n) {
    __m256d suma_vec = _mm256_setzero_pd();

    int i=0;

    for (; i <= n-4; i+=4)
    {
        //cargar 4 datos
        __m256d vecA = _mm256_loadu_pd(&x[i]);
        __m256d vecB = _mm256_loadu_pd(&y[i]);

        //multiplicacion de los 4 pares
        __m256d multiplicacion = _mm256_mul_pd(vecA, vecB);

        //suma de los 4 pares
        suma_vec = _mm256_add_pd(suma_vec, multiplicacion);

    }

    //sacamos los subtotales
    double subtotales[4];
    _mm256_storeu_pd(subtotales, suma_vec);

    double suma_total = subtotales[0] + subtotales[1] + subtotales[2] + subtotales[3];

    //el int i afuera del for almaceno la posicion
    //va a seguir sumando si el numero de n terminos no es multiplo de 4
    for (; i < n; i++)
    {
        suma_total += x[i]*y[i];
    }
    
    return suma_total;
}
