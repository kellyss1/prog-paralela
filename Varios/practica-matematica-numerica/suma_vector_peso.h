#ifndef SUMA_VECTOR_PESO
#define SUMA_VECTOR_PESO

double* suma_vector_peso_openmp(double* A, double* B, int n);
double* suma_vector_peso_simd(double* A, double* B, int n);

#endif