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

    // Cálculo uniforme del padding
    int rows_per_rank = std::ceil(MATRIX_DIH * 1.0 / nprocs);
    int total_rows_padded = rows_per_rank * nprocs;

    std::vector<double> A_local(rows_per_rank * MATRIX_DIH);
    std::vector<double> b_local(MATRIX_DIH);
    std::vector<double> X_local(rows_per_rank);

    if (rank == 0)
    {
        std::vector<double> A(total_rows_padded * MATRIX_DIH, 0.0);
        std::vector<double> B(MATRIX_DIH);
        std::vector<double> X(total_rows_padded, 0.0);

        // Inicializar la matriz y el vector
        for (int i = 0; i < MATRIX_DIH; i++) {
            for (int j = 0; j < MATRIX_DIH; j++) {
                A[i * MATRIX_DIH + j] = i;
            }
        }
        for (int i = 0; i < MATRIX_DIH; i++) {
            B[i] = 1;
        }

        // --- 1. ENVIAR SOLO A LOS ESCLAVOS REALES (i = 1 en adelante) ---
        for (int i = 1; i < nprocs; i++)
        {
            std::vector<int> dimensiones = {MATRIX_DIH, rows_per_rank};
            MPI_Send(dimensiones.data(), 2, MPI_INT, i, 100, MPI_COMM_WORLD);

            int offset_A = i * rows_per_rank * MATRIX_DIH;
            int cantidad_datos = rows_per_rank * MATRIX_DIH;

            MPI_Send(&A[offset_A], cantidad_datos, MPI_DOUBLE, i, 200, MPI_COMM_WORLD);
            MPI_Send(B.data(), MATRIX_DIH, MPI_DOUBLE, i, 300, MPI_COMM_WORLD);
        }

        // --- 2. EL RANK 0 TOMA SU PROPIA PARTE MANUALMENTE ---
        // Copiamos el primer bloque de la matriz A y el vector B completo
        for (int i = 0; i < rows_per_rank * MATRIX_DIH; i++) {
            A_local[i] = A[i];
        }
        b_local = B;
        
        // El Rank 0 ejecuta su multiplicación local en paralelo con los demás
        multiplicar_matriz_vector(A_local, b_local, X_local, rows_per_rank, MATRIX_DIH);

        // Guardamos su resultado local al inicio del vector global X
        for (int i = 0; i < rows_per_rank; i++) {
            X[i] = X_local[i];
        }

        // --- 3. RECOLECTAR RESULTADOS DE LOS ESCLAVOS REALES ---
        for (int i = 1; i < nprocs; i++)
        {
            int offset_X = i * rows_per_rank;
            MPI_Recv(&X[offset_X], rows_per_rank, MPI_DOUBLE, i, 400, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        fmt::print("RANK_{}, resultado final (filtrando el padding):\n", rank);
        imprimir_vector(X, MATRIX_DIH);
    }
    else
    {
        // --- CÓDIGO DE LOS ESCLAVOS (Ranks 1 en adelante) ---
        std::vector<int> data_rec(2);
        MPI_Recv(data_rec.data(), 2, MPI_INT, 0, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        int matrix_dim = data_rec[0];
        int rows = data_rec[1];

        MPI_Recv(A_local.data(), rows * matrix_dim, MPI_DOUBLE, 0, 200, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(b_local.data(), matrix_dim, MPI_DOUBLE, 0, 300, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Multiplicación local
        multiplicar_matriz_vector(A_local, b_local, X_local, rows, matrix_dim);

        // Devolución al maestro
        MPI_Send(X_local.data(), rows, MPI_DOUBLE, 0, 400, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}