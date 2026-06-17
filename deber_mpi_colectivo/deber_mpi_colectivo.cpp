#include <iostream>
#include <fmt/core.h>
#include <mpi.h>
#include <vector>
#include <cmath>

#define MATRIX_DIH 25

void imprimir_vector(const std::vector<double>& v, int elementos_a_mostrar) {
    for (int i = 0; i < elementos_a_mostrar; i++) {
        std::cout << v[i] << " ";
    }
    std::cout << std::endl;
}

void multiplicar_matriz_vector(const std::vector<double>& A,
                               const std::vector<double>& b,
                               std::vector<double>& X,
                               int rows,
                               int cols)
{
    for (int i = 0; i < rows; i++) {
        double sum = 0.0;
        for (int j = 0; j < cols; j++) {
            sum += A[i * cols + j] * b[j];
        }
        X[i] = sum;
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // --- LÓGICA DE PADDING (Idéntica para todos) ---
    int rows_per_rank = std::ceil(MATRIX_DIH * 1.0 / nprocs);
    int total_rows_padded = rows_per_rank * nprocs;

    // Estructuras locales simétricas
    std::vector<double> A_local(rows_per_rank * MATRIX_DIH);
    std::vector<double> b_local(MATRIX_DIH);
    std::vector<double> X_local(rows_per_rank);

    // Variables globales (Solo el Rank 0 las llenará de datos reales)
    std::vector<double> A_global;
    std::vector<double> B_global(MATRIX_DIH, 1.0); // Vector de unos listo para todos
    std::vector<double> X_global;

    if (rank == 0)
    {
        // El maestro inicializa los contenedores globales con el padding
        A_global.resize(total_rows_padded * MATRIX_DIH, 0.0);
        X_global.resize(total_rows_padded, 0.0);

        // Llenar la matriz con la lógica de tu clase
        for (int i = 0; i < MATRIX_DIH; i++) {
            for (int j = 0; j < MATRIX_DIH; j++) {
                A_global[i * MATRIX_DIH + j] = i;
            }
        }
    }

    // --- FASE COLECTIVA: REPARTO AUTOMÁTICO ---
    // 1. Scatter: Corta A_global en rebanadas iguales y le da una a cada A_local
    int elementos_por_bloque = rows_per_rank * MATRIX_DIH;
    MPI_Scatter(A_global.data(), elementos_por_bloque, MPI_DOUBLE, 
                A_local.data(), elementos_por_bloque, MPI_DOUBLE, 
                0, MPI_COMM_WORLD);

    // 2. Bcast: El Rank 0 le copia el vector B_global entero a los b_local de TODOS
    b_local = B_global; // El Rank 0 ya lo tiene, pero ejecutamos el Bcast para los demás
    MPI_Bcast(b_local.data(), MATRIX_DIH, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // --- CÓDIGO PARALELO HOMOGÉNEO ---
    // Todos los ranks corren la misma multiplicación al mismo tiempo sobre sus bloques locales
    multiplicar_matriz_vector(A_local, b_local, X_local, rows_per_rank, MATRIX_DIH);

    // --- FASE COLECTIVA: RECOLECCIÓN AUTOMÁTICA ---
    // 3. Gather: Toma los X_local de todos y los junta ordenadamente en X_global
    MPI_Gather(X_local.data(), rows_per_rank, MPI_DOUBLE, 
               X_global.data(), rows_per_rank, MPI_DOUBLE, 
               0, MPI_COMM_WORLD);

    // --- MOSTRAR RESULTADO (Solo el maestro) ---
    if (rank == 0)
    {
        fmt::print("RANK_{}, (COLECTIVO) Resultado final filtrando el padding:\n", rank);
        imprimir_vector(X_global, MATRIX_DIH);
    }

    MPI_Finalize();
    return 0;
}