#include <iostream>
#include <fmt/core.h>
#include <mpi.h>
#include <vector>
#include <cmath>

#define MATRIX_ROWS 25
#define MATRIX_COLS 32

void imprimir_vector(const std::vector<double>& v, int limite) {
    for (int i = 0; i < limite; i++) {
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

    // Todos calculan el mismo tamaño de bloque
    int rows_per_rank = std::ceil(MATRIX_ROWS * 1.0 / nprocs);
    int total_rows_padded = rows_per_rank * nprocs;

    if (rank == 0)
    {
        // --- 1. INICIALIZACIÓN (Solo Rank 0) ---
        std::vector<double> A_padded(total_rows_padded * MATRIX_COLS, 0.0); 
        std::vector<double> B(MATRIX_COLS); 
        std::vector<double> X_final(total_rows_padded, 0.0); 

        for (int i = 0; i < MATRIX_ROWS; i++) {
            for (int j = 0; j < MATRIX_COLS; j++) {
                A_padded[i * MATRIX_COLS + j] = i; 
            }
        }
        for (int i = 0; i < MATRIX_COLS; i++) {
            B[i] = 1; 
        }

        // --- 2. REPARTIR A LOS ESCLAVOS REALES (Ranks 1 a nprocs-1) ---
        for (int i = 1; i < nprocs; i++)
        {
            int elementos_a_enviar = rows_per_rank * MATRIX_COLS;
            int offset_A = i * rows_per_rank * MATRIX_COLS;

            // Enviamos los bloques idénticos con tags diferenciados
            MPI_Send(&A_padded[offset_A], elementos_a_enviar, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(B.data(), MATRIX_COLS, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
        }

        // --- 3. EL RANK 0 HACE SU PROPIA PARTE (Sin usar la red) ---
        std::vector<double> A_local_rank0(A_padded.begin(), A_padded.begin() + (rows_per_rank * MATRIX_COLS));
        std::vector<double> X_local_rank0(rows_per_rank);

        multiplicar_matriz_vector(A_local_rank0, B, X_local_rank0, rows_per_rank, MATRIX_COLS);

        // Guardamos el resultado del Rank 0 al inicio de nuestro vector final
        for (int i = 0; i < rows_per_rank; i++) {
            X_final[i] = X_local_rank0[i];
        }

        // --- 4. RECOLECTAR DE LOS ESCLAVOS REALES ---
        for (int i = 1; i < nprocs; i++)
        {
            int offset_X = i * rows_per_rank;
            MPI_Recv(&X_final[offset_X], rows_per_rank, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // --- 5. MOSTRAR RESULTADOS ---
        fmt::print("RANK_{}, Resultado final de X (Filtrado):\n", rank);
        imprimir_vector(X_final, MATRIX_ROWS);
    }
    else
    {
        // --- CÓDIGO DE LOS ESCLAVOS REALES (Ranks 1 en adelante) ---
        std::vector<double> A_local(rows_per_rank * MATRIX_COLS);
        std::vector<double> b_local(MATRIX_COLS);
        std::vector<double> X_local(rows_per_rank);

        // Se quedan esperando las cartas del jefe (Rank 0)
        MPI_Recv(A_local.data(), rows_per_rank * MATRIX_COLS, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(b_local.data(), MATRIX_COLS, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Procesan su bloque relleno
        multiplicar_matriz_vector(A_local, b_local, X_local, rows_per_rank, MATRIX_COLS);

        // Envían el resultado de vuelta
        MPI_Send(X_local.data(), rows_per_rank, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}

// #include <iostream>
// #include <fmt/core.h>
// #include <mpi.h>
// #include <vector>

// #define MATRIX_ROWS 25
// #define MATRIX_COLS 32

// void imprimir_vector(const std::vector<double>& v) {
//     for (size_t i = 0; i < v.size(); i++) {
//         std::cout << v[i] << " ";
//     }
//     std::cout << std::endl;
// }

// void multiplicar_matriz_vector(const std::vector<double>& A,
//                                const std::vector<double>& b,
//                                std::vector<double>& X,
//                                int rows,
//                                int cols) 
// {
//     for (int i = 0; i < rows; i++) {
//         double sum = 0.0;
//         for (int j = 0; j < cols; j++) {
//             sum += A[i * cols + j] * b[j];
//         }
//         X[i] = sum;
//     }
// }

// int main(int argc, char **argv)
// {
//     MPI_Init(&argc, &argv);
//     int nprocs, rank;
//     MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);

//     // Lógica matemática de reparto (todos los procesos calculan esto para saber su tamaño)
//     int filas_base = MATRIX_ROWS / nprocs;
//     int residuo = MATRIX_ROWS % nprocs;

//     if (rank == 0)
//     {
//         std::vector<double> A(MATRIX_ROWS * MATRIX_COLS);
//         std::vector<double> B(MATRIX_COLS); 
//         std::vector<double> X(MATRIX_ROWS); 

//         // Inicializar la matriz A y el vector B
//         for (int i = 0; i < MATRIX_ROWS; i++) {
//             for (int j = 0; j < MATRIX_COLS; j++) {
//                 A[i * MATRIX_COLS + j] = i; 
//             }
//         }
//         for (int i = 0; i < MATRIX_COLS; i++) {
//             B[i] = 1; 
//         }

//         // --- REPARTIR TRABAJO A LOS ESCLAVOS (Ranks 1 a nprocs-1) ---
//         // El Rank 0 se queda con el primer bloque, así que el offset del Rank 1 empieza luego del bloque del Rank 0
//         int filas_rank0 = filas_base + (0 < residuo ? 1 : 0);
//         int offset = filas_rank0 * MATRIX_COLS; 

//         for (int i = 1; i < nprocs; i++)
//         {
//             int filas_a_enviar = filas_base + (i < residuo ? 1 : 0);

//             // 1. Enviar dimensiones
//             std::vector<int> dimensiones = {filas_a_enviar, MATRIX_COLS};
//             MPI_Send(dimensiones.data(), 2, MPI_INT, i, 0, MPI_COMM_WORLD);

//             // 2. Enviar el bloque de la matriz A
//             int elementos_a_enviar = filas_a_enviar * MATRIX_COLS;
//             MPI_Send(&A[offset], elementos_a_enviar, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);

//             // 3. Enviar el vector B completo
//             MPI_Send(B.data(), MATRIX_COLS, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);

//             offset += elementos_a_enviar; 
//         }

//         // --- EL RANK 0 HACE SU PROPIO TRABAJO ---
//         // Creamos un subvector temporal para la porción de la matriz del Rank 0
//         std::vector<double> A_local_rank0(A.begin(), A.begin() + (filas_rank0 * MATRIX_COLS));
//         std::vector<double> X_local_rank0(filas_rank0);
        
//         multiplicar_matriz_vector(A_local_rank0, B, X_local_rank0, filas_rank0, MATRIX_COLS);

//         // Copiar el resultado local del Rank 0 directamente en el vector definitivo X
//         for (int i = 0; i < filas_rank0; i++) {
//             X[i] = X_local_rank0[i];
//         }

//         // --- RECOLECTAR RESULTADOS DE LOS ESCLAVOS ---
//         int offset_X = filas_rank0; // Empezamos a escribir en X justo después de lo que calculó el Rank 0
//         for (int i = 1; i < nprocs; i++)
//         {
//             int filas_a_recibir = filas_base + (i < residuo ? 1 : 0);

//             MPI_Recv(&X[offset_X], filas_a_recibir, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
//             offset_X += filas_a_recibir; 
//         }

//         fmt::print("RANK_{}, Resultado final de X:\n", rank);
//         imprimir_vector(X);
//     }
//     else
//     {
//         // --- CÓDIGO DE LOS ESCLAVOS (Solo entran Ranks del 1 en adelante) ---
//         std::vector<int> dimensiones_rec(2);
//         MPI_Recv(dimensiones_rec.data(), 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
//         int filas_locales = dimensiones_rec[0];
//         int columnas_locales = dimensiones_rec[1];

//         std::vector<double> A_local(filas_locales * columnas_locales);
//         std::vector<double> b_local(columnas_locales);
//         std::vector<double> X_local(filas_locales);

//         MPI_Recv(A_local.data(), filas_locales * columnas_locales, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//         MPI_Recv(b_local.data(), columnas_locales, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

//         multiplicar_matriz_vector(A_local, b_local, X_local, filas_locales, columnas_locales);

//         // Enviar de vuelta al Rank 0
//         MPI_Send(X_local.data(), filas_locales, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
//     }

//     MPI_Finalize();
//     return 0;
// }