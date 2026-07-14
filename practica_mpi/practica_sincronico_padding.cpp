#include <mpi.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <fmt/core.h>

#define M 32

void multiplicar_matriz_vector(const std::vector<double> &A,
                               const std::vector<double> &b,
                               std::vector<double> &X, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        double temp = 0.0;
        for (int j = 0; j < cols; j++)
        {
            temp += A[i * cols + j] * b[j];
        }
        X[i] = temp;
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int filas_por_rank = std::ceil(M * 1.0 / nprocs);
    int total_filas_padding = filas_por_rank * M;

    std::vector<double> A_global(total_filas_padding * M, 0.0);
    std::vector<double> b_global(M, 1.0);
    std::vector<double> X_global(total_filas_padding);

    int numero_elementos = filas_por_rank * M;

    std::vector<double> A_local(numero_elementos);
    std::vector<double> X_local(filas_por_rank);

    // Reparto Sincrono
    if (rank == 0)
    {
        // Inicializar la matriz
        for (int i = 0; i < M; i++)
        {
            for (int j = 0; j < M; j++)
            {
                A_global[i * M + j] = i;
            }
        }

        // Rank 0 copia su propia porcion
        for (int i = 0; i < numero_elementos; i++)
        {
            A_local[i] = A_global[i];
        }

        // Envia cada porcion a cada rank excluyendose
        for (int i = 1; i < nprocs; i++)
        {
            int offset = i * numero_elementos;
            MPI_Send(&A_global[offset], numero_elementos, MPI_DOUBLE,
                     i, 0, MPI_COMM_WORLD);

            MPI_Send(b_global.data(), M, MPI_DOUBLE,
                     i, 1, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(A_local.data(), numero_elementos, MPI_DOUBLE,
                 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Recv(b_global.data(), M, MPI_DOUBLE,
                 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Recoleccion Sincrona
    if (rank == 0)
    {
        // Multiplica su propia porcion
        multiplicar_matriz_vector(A_local, b_global, X_local, filas_por_rank, M);

        // Escribe su parte en X_global
        for (int i = 0; i < filas_por_rank; i++)
        {
            X_global[i] = X_local[i];
        }

        // Recibe las x de los ranks
        for (int i = 1; i < nprocs; i++)
        {
            int offset = i * filas_por_rank;

            MPI_Recv(&X_global[offset], filas_por_rank, MPI_DOUBLE, i, 2,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        fmt::println("Resultado Sincronico: ");
        for (int i = 0; i < M; i++)
        {
            fmt::print("{} ", X_global[i]);
        }
    }
    else
    {
        // Multiplica en el rank
        multiplicar_matriz_vector(A_local, b_global, X_local, filas_por_rank, M);

        MPI_Send(X_local.data(), filas_por_rank, MPI_DOUBLE,
                 0, 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}