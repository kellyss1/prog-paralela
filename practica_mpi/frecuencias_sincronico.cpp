#include <mpi.h>
#include <cmath>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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

int calcular_maximo(const std::vector<int> &Datos)
{
    int max = 0;
    for (const auto &num : Datos)
    {
        if (num > max)
        {
            max = num;
        }
    }
    return max;
}

int calcular_minimo(const std::vector<int> &Datos)
{
    int min = 100;
    for (const auto &num : Datos)
    {
        if (num < min)
        {
            min = num;
        }
    }
    return min;
}

std::vector<int> frecuencias(const std::vector<int> &Datos) {

}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<int> Datos = read_file();
    int longitud = Datos.size();

    int elementos_por_rank = std::ceil(longitud * 1.0 / nprocs);
    int total_elementos_padding = elementos_por_rank * nprocs;

    std::vector<int> Datos_local(elementos_por_rank);
    std::vector<int> maximos(nprocs, 0);
    std::vector<int> minimos(nprocs, 0);

    int maximo_local, minimo_local;

    //Repartir sincronico
    if (rank == 0)
    {
        Datos.resize(total_elementos_padding);

        //Datos a su propio rank 0
        for (int  i = 0; i < elementos_por_rank; i++)
        {
            Datos_local[i] = Datos[i];
        }

        //Enviar porcion a cada rank
        for (int i = 1; i < nprocs; i++)
        {
            int offset = i * elementos_por_rank;
            MPI_Send(&Datos[offset], elementos_por_rank, MPI_INT,
                    i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(Datos_local.data(), elementos_por_rank, MPI_INT,
                0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    //Recoleccion Sincrona
    if(rank == 0) {
        //Calcular sus datos
        maximos[0] = calcular_maximo(Datos_local);
        minimos[0] = calcular_minimo(Datos_local);

        //Recibir datos de ranks
        for (int i = 1; i < nprocs; i++)
        {
            MPI_Recv(&maximo_local, i, MPI_INT,
                    1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            maximos[i] = maximo_local;

            MPI_Recv(&minimo_local, i, MPI_INT,
                    1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            minimos[i] = minimo_local;
        }

        int maximo, minimo;
        maximo = calcular_maximo(maximos);
        minimo = calcular_minimo(minimos);

    } else {
        //Enviar datos
        maximo_local = calcular_maximo(Datos_local);
        minimo_local = calcular_minimo(Datos_local);

        MPI_Send(&maximo_local, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&minimo_local, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
