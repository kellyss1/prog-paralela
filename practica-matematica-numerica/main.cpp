#include "producto_escalar.h"
#include "suma_vector_peso.h"

#include <iostream>
#include <vector>

int main()
{
    // tamaño de los vectores
    int n = 9;

    // crear vectores
    std::vector<double> x = {1.5, 2.0, 3.5, 4.0, 5.1, 6.2, 7.3, 8.4, 8.8};
    std::vector<double> y = {2.0, 1.5, 0.5, 3.0, 2.0, 1.0, 4.0, 2.5, 1.1};

    std::cout << "- Prueba producto escalar -" << "\n";
    double resultado_serial = producto_escalar_serial(x.data(), y.data(), n);

    double resultado_openmp = producto_escalar_openmp(x.data(), y.data(), n);

    double resultado_simd = producto_escalar_simd(x.data(), y.data(), n);

    std::cout << "Resultado serial: " << resultado_serial << "\n";
    std::cout << "Resultado OpenMP: " << resultado_openmp << "\n";
    std::cout << "Resultado SIMD:   " << resultado_simd << "\n";

    std::cout << "- Prueba vector con peso -" << "\n";

    std::cout << "Resultado Vector Peso OpenMP: ";
    double *resultado_vector_openmp = suma_vector_peso_openmp(x.data(), y.data(), n);
    for (int i = 0; i < n; i++)
    {
        std::cout << resultado_vector_openmp[i] << " ";
    }
    std::cout << "\n";

    std::cout << "Resultado Vector Peso Simd: ";
    double *resultado_vector_simd = suma_vector_peso_simd(x.data(), y.data(), n);
    for (int i = 0; i < n; i++)
    {
        std::cout << resultado_vector_simd[i] << " ";
    }
    std::cout << "\n";

    // Liberar la memoria
    delete[] resultado_vector_openmp;
    delete[] resultado_vector_simd;

    return 0;
}
