#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <fmt/core.h>

// 1. Función para leer el archivo de texto
std::vector<int> read_file()
{
    std::fstream fs("path/to/datos.txt", std::ios::in);
    std::string line;
    std::vector<int> ret;

    while (std::getline(fs, line))
    {
        ret.push_back(std::stoi(line));
    }
    fs.close();
    return ret;
}

// 2. Funciones de cálculo local directas (Fáciles de recordar)
int calcular_maximo(const std::vector<int> &Datos)
{
    int max = Datos[0];
    for (const auto &num : Datos) {
        if (num > max) max = num;
    }
    return max;
}

int calcular_minimo(const std::vector<int> &Datos)
{
    int min = Datos[0];
    for (const auto &num : Datos) {
        if (num < min) min = num;
    }
    return min;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Variables globales
    std::vector<int> Datos;
    int longitud = 0;

    // Solo el Rank 0 lee el archivo físico
    if (rank == 0) {
        Datos = read_file();
        longitud = Datos.size();
    }

    // Compartimos la longitud real con todos de un solo golpe
    MPI_Bcast(&longitud, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // ---DIVISIÓN DIRECTA (Sin redondear ni rellenar) ---
    int elementos_por_rank = longitud / nprocs; 

    // Cada rank prepara su porción exacta
    std::vector<int> Datos_local(elementos_por_rank);

    // Reparto Colectivo de datos planos
    MPI_Scatter(Datos.data(), elementos_por_rank, MPI_INT,
                Datos_local.data(), elementos_por_rank, MPI_INT, 
                0, MPI_COMM_WORLD);

    // --- PROCESAMIENTO LOCAL (Cada uno con sus datos) ---
    int maximo_local = calcular_maximo(Datos_local);
    int minimo_local = calcular_minimo(Datos_local);

    // Para el promedio, cada uno suma su parte local
    long long suma_local = 0;
    for (int num : Datos_local) {
        suma_local += num;
    }

    // Para frecuencias, cada uno cuenta su vector local (del 0 al 100)
    std::vector<int> frec_local(101, 0);
    for (int num : Datos_local) {
        frec_local[num]++;
    }

    // --- REDUCCIONES COLECTIVAS DE RESULTADOS ---
    int maximo, minimo;
    long long suma_total;
    std::vector<int> frec_global(101, 0);

    // Reducciones directas (con el símbolo '&' para variables individuales)
    MPI_Reduce(&maximo_local, &maximo, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&minimo_local, &minimo, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&suma_local, &suma_total, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Reducción del vector de frecuencias de tamaño 101
    MPI_Reduce(frec_local.data(), frec_global.data(), 101, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // ---  IMPRESIÓN (Solo el Rank 0 calcula finales y muestra) ---
    if (rank == 0) {
        double promedio = suma_total * 1.0 / longitud;

        fmt::println("Máximo: {}", maximo);
        fmt::println("Mínimo: {}", minimo);
        fmt::println("Promedio: {:.6f}", promedio);

        fmt::println("\nTabla de frecuencias:");
        for (int i = 0; i <= 100; i++) {
            if (frec_global[i] > 0) {
                fmt::println("{}: {}", i, frec_global[i]);
            }
        }
    }

    MPI_Finalize();
    return 0;
}