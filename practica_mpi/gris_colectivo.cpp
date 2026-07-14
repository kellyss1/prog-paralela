#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Librería del docente para leer imágenes [cite: 51, 53]
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // Librería del docente para escribir imágenes [cite: 51, 55]

#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <fmt/core.h>

void convertir_bloque_a_gris(const std::vector<uint8_t> &rgb_local, 
                             std::vector<uint8_t> &gris_local, 
                             int num_pixeles) {
    for (int i = 0; i < num_pixeles; i++) {
        uint8_t r = rgb_local[3 * i];
        uint8_t g = rgb_local[3 * i + 1];
        uint8_t b = rgb_local[3 * i + 2];
        
        gris_local[i] = static_cast<uint8_t>(0.21 * r + 0.72 * g + 0.07 * b); // [cite: 42, 44]
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int width = 0, height = 0, channels = 0;
    uint8_t* rgb_pixels_raw = nullptr;

    if (rank == 0) {
        rgb_pixels_raw = stbi_load("imagen.jpg", &width, &height, &channels, STBI_rgb); // [cite: 56, 60]
        if (!rgb_pixels_raw) {
            fmt::println("ERROR: No se pudo cargar la imagen 'imagen.jpg'");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        channels = 3;
    }

    // 1. Transmitimos dimensiones iniciales
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // --- MATEMÁTICAS DE PADDING DE PÍXELES ---
    int total_pixeles_reales = width * height;
    int pixeles_por_rank = std::ceil(total_pixeles_reales * 1.0 / nprocs);
    int total_pixeles_padding = pixeles_por_rank * nprocs;

    // Contenedores globales en Rank 0
    std::vector<uint8_t> RGB_global;
    std::vector<uint8_t> Gris_global;

    if (rank == 0) {
        RGB_global.resize(total_pixeles_padding * 3, 0);
        Gris_global.resize(total_pixeles_padding, 0);

        for (int i = 0; i < total_pixeles_reales * 3; i++) {
            RGB_global[i] = rgb_pixels_raw[i];
        }
        stbi_image_free(rgb_pixels_raw);
    }

    // --- PREPARACIÓN DE LAS BANDEJAS LOCALES ---
    std::vector<uint8_t> RGB_local(pixeles_por_rank * 3);
    std::vector<uint8_t> Gris_local(pixeles_por_rank);

    // --- REPARTO SÍNCRONO PUNTO A PUNTO ---
    if (rank == 0) {
        // A. El Rank 0 copia su propia porción directamente
        for (int j = 0; j < pixeles_por_rank * 3; j++) {
            RGB_local[j] = RGB_global[j];
        }

        // B. Envía la porción correspondiente a cada esclavo
        for (int i = 1; i < nprocs; i++) {
            int offset_rgb = i * (pixeles_por_rank * 3);
            MPI_Send(&RGB_global[offset_rgb], pixeles_por_rank * 3, MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
    } else {
        // C. Los esclavos reciben su porción RGB
        MPI_Recv(RGB_local.data(), pixeles_por_rank * 3, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // --- PROCESAMIENTO PARALELO LOCAL ---
    convertir_bloque_a_gris(RGB_local, Gris_local, pixeles_por_rank);

    // --- RECOLECCIÓN SÍNCRONA PUNTO A PUNTO ---
    if (rank == 0) {
        // A. El Rank 0 guarda su propia parte procesada
        for (int j = 0; j < pixeles_por_rank; j++) {
            Gris_global[j] = Gris_local[j];
        }

        // B. Recibe secuencialmente los resultados de los esclavos
        for (int i = 1; i < nprocs; i++) {
            int offset_gris = i * pixeles_por_rank;
            MPI_Recv(&Gris_global[offset_gris], pixeles_por_rank, MPI_BYTE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        // C. Los esclavos envían su resultado procesado
        MPI_Send(Gris_local.data(), pixeles_por_rank, MPI_BYTE, 0, 1, MPI_COMM_WORLD);
    }

    // 2. Guardado en disco en Rank 0 [cite: 49]
    if (rank == 0) {
        std::vector<uint8_t> Gris_final(total_pixeles_reales);
        for (int i = 0; i < total_pixeles_reales; i++) {
            Gris_final[i] = Gris_global[i];
        }

        stbi_write_png("img-gris-sincrona.png", width, height, STBI_grey, Gris_final.data(), width); // [cite: 63]
        fmt::println("¡Imagen procesada con ÉXITO en modo SÍNCRONO!");
        fmt::println("Resultado guardado en 'img-gris-sincrona.png'");
    }

    MPI_Finalize();
    return 0;
}