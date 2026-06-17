#ifndef PRODUCTO_ESCALAR_H
#define PRODUCTO_ESCALAR_H

#include <cstdint>

double producto_escalar_openmp(double* x, double* y, int n);
double producto_escalar_serial(double* x, double* y, int n);
double producto_escalar_simd(double* x, double* y, int n);

#endif // PRODUCTO_ESCALAR_H