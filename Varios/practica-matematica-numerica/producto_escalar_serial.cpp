#include "producto_escalar.h"

double producto_escalar_serial(double* x, double* y, int n) {
    double suma = 0.0;
    for(int i = 0; i < n; i++) {
        suma += x[i] * y[i];
    }
    return suma;
}