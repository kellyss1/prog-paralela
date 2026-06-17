#include <iostream>
#include <fmt/core.h>
#include <mpi.h>
#include <vector>
#define MATRIX_DIH 25

#include <vector>
#include <iostream>

void imprimir_matriz(const std::vector<double>& A_local,
                     int rows,
                     int matrix_dim) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < matrix_dim; j++) {
            std::cout << A_local[i * matrix_dim + j] << " ";
        }
        std::cout << std::endl;
    }
}

void imprimir_vector(const std::vector<double>& v) {
    for (int i = 0; i < v.size(); i++) {
        std::cout << v[i] << " ";
        }
        std::cout << std::endl;
    }
void multiplicar_matriz_vector(std::vector<double>& A,
                              std::vector<double>& b,
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

    // numero de filas para cada RANK (proceso)

    if (rank == 0)
    {
        std::vector<double> A(MATRIX_DIH * MATRIX_DIH);
        std::vector<double> B(MATRIX_DIH);
        std::vector<double> X(MATRIX_DIH);
        // inicializar la matriz A y el vector b
        for (int i = 0; i < MATRIX_DIH; i++)
        {
            for (int j = 0; j < MATRIX_DIH; j++)
            {
                int index = i * MATRIX_DIH + j;
                A[index] = i;
            }
        }
        for (int i = 0; i < MATRIX_DIH; i++)
        {
            B[i] = 1;
        }

        // numero de filas y columnas por procesos
        int rows_per_rank = std::ceil(MATRIX_DIH * 1.0 / nprocs);
        int padding = rows_per_rank * nprocs - MATRIX_DIH;
        //fmt::print("MATRIX_DIM{}, nprocs:{}, rows_per_rank:{},padding :{}\n", MATRIX_DIH, nprocs, rows_per_rank, padding);
        // enviar dimensiones y datos
        for (int i = 0; i < nprocs; i++)
        {
            int fila = rows_per_rank;
            if (nprocs - 1 == i)
            {
                fila = rows_per_rank - padding;
            }

            // enviar dimensiones
            std::vector<int> data = {MATRIX_DIH, fila};
            MPI_Send(
                data.data(),   // buffer de datos
                2,             // data.size(),   // cuanto
                MPI_INT,       // Tipo de dato
                i,             // Rank de destino
                0,             // TAG
                MPI_COMM_WORLD // Grupo
            );

            const double *Buffer = A.data();
            // enviar datos al rank i
            MPI_Send(
                &Buffer[i * rows_per_rank * MATRIX_DIH], // buffer de datos
                fila * MATRIX_DIH,                     // data.size(),   // cuanto
                MPI_DOUBLE,                            // Tipo de dato
                i,                                     // Rank de destino
                0,                                     // TAG
                MPI_COMM_WORLD                         // Grupo
            );

            //enviar vector b
                        MPI_Send(
                B.data(), // buffer de datos
                MATRIX_DIH,                            // data.size(),   // cuanto
                MPI_DOUBLE,                            // Tipo de dato
                i,                                     // Rank de destino
                0,                                     // TAG
                MPI_COMM_WORLD                         // Grupo
            );
        }

    //fmt::print("RANK_{}, {} x {}", rank, rows_per_rank, MATRIX_DIH);
                  
       
        multiplicar_matriz_vector(A,B,X,rows_per_rank,MATRIX_DIH);

        for (int i = 1; i < nprocs; i++)
        {
             int fila = rows_per_rank;
            if (nprocs - 1 == i)
            {
                fila = rows_per_rank - padding;
            }

            MPI_Recv(
            X.data()+i*rows_per_rank,
            fila,
            MPI_DOUBLE,
            i,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        }
              fmt::print("RANK_{}, resultado parcial:\n", rank);
          imprimir_vector(X);
    }

    else
    {
        std::vector<int> data_rec(2);
        std::vector<double> b_local(MATRIX_DIH);
        MPI_Recv(
            data_rec.data(),
            2,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
        int matrix_dim = data_rec[0];
        int rows = data_rec[1];
        //fmt::print("RANK_{}, {} x {}", rank, matrix_dim, rows);

        std::vector<double> A_local(rows * matrix_dim);
        MPI_Recv(
            A_local.data(),
            rows * matrix_dim,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        MPI_Recv(
            b_local.data(),
            MATRIX_DIH,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

          //if (rank==2)
          //{
           // imprimir_matriz(A_local,rows,matrix_dim);
            //imprimir_vector(b_local,matrix_dim);
         // }
          std::vector<double> X_local(rows);
          multiplicar_matriz_vector(A_local,b_local,X_local,rows,matrix_dim);

        MPI_Send(
            X_local.data(),
            rows,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}