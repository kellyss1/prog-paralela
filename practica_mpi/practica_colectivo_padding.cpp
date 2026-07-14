#include <mpi.h>
#include <iostream>
#include <fmt/core.h>
#include <vector>
#include <math.h>

#define M 32

void multiplicar_matriz_vector(const std::vector<double> &A, 
                                const std::vector<double> &b,
                                std::vector<double> &X, int rows, int cols) {
    for (int i = 0; i < rows; i++)
    {
        double temp = 0.0;
        for (int j = 0; j < cols; j++)
        {
            temp += A[i*cols+j] * b[j];
        }
        X[i] = temp;
    }
}

int main(int argc, char** argv) {
    
    MPI_Init(&argc, &argv);

    int nprocs, rank;

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int filas_por_rank = std::ceil(M*1.0/nprocs); //redondea hacia arriba
    int total_filas_padding = filas_por_rank*nprocs; //nuevo numero de filas

    std::vector<double> A_global(total_filas_padding*M, 0.0);
    std::vector<double> b_global(M, 1.0);
    std::vector<double> X_global(total_filas_padding, 0.0);

    int numero_elementos = filas_por_rank * M;

    std::vector<double> A_local(numero_elementos);
    std::vector<double> X_local(filas_por_rank);

    if(rank == 0) {
        for (int i = 0; i < M; i++)
        {
            for (int j = 0; j < M; j++)
            {
                A_global[i*M+j] = i;
            }
            
        }
        
    }

    MPI_Scatter(
        A_global.data(), numero_elementos, MPI_DOUBLE,
        A_local.data(), numero_elementos, MPI_DOUBLE,
        0, MPI_COMM_WORLD
    );

    MPI_Bcast(
        b_global.data(), M, MPI_DOUBLE,
        0, MPI_COMM_WORLD
    );

    multiplicar_matriz_vector(A_local, b_global, X_local, filas_por_rank, M);

    MPI_Gather(
        X_local.data(), filas_por_rank,  MPI_DOUBLE,
        X_global.data(), filas_por_rank, MPI_DOUBLE,
        0, MPI_COMM_WORLD
    );

    if(rank == 0) {
        fmt::println("Resultado Final X_global: ");
        for (int i = 0; i < M; i++)
        {
            fmt::print("{} ", X_global[i]);
        }
    }

    MPI_Finalize();
    return 0;
}

