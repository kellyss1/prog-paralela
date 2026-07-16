#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include "fractal_mpi.h"
#include "mpi.h"
#include "arial_ttf.h"
#include "draw_text.h"

std::string machine_name() {
    #ifdef _WIN32
    if (const char *name = std::getenv("COMPUTERNAME")) { return name; }
    #else
    if (const char *name = std::getenv("HOSTNAME")) { return name; }
    #endif
    return "unknown";
}

// Parámetros por defecto exigidos por el enunciado del examen
int max_iteraciones = 100;
double x_min = -1.8;
double x_max = -1.7;
double y_min = -0.1;
double y_max = 0.05;

uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;
int running = 1;
int delta; 
int rows_start;
int rows_end;
int nprocs, rank;

#define ANCHO 1600
#define ALTO 900
int padding;

// Histogramas locales y de recolección global
int local_hist[16] = {0};
int *gather_hist_buffer = nullptr; // Buffer dinámico de tamaño 16 * nprocs
int global_hist[16] = {0};         // Histograma consolidado final de 16 bins

void draw_rank_bottom_line(uint32_t *buffer, int y)
{
    if (!buffer || y < 0 || y >= ALTO) return;
    for (int x = 0; x < ANCHO; x++) {
        buffer[y * ANCHO + x] = 0xFFFFFFFF;
    }
}

void dibujar_texto(int rank)
{
    auto texto = fmt::format("RANK_{} - {}", rank, machine_name());
    if (rank == 0) {
        draw_text_to_texture((unsigned char *)texture_buffer, ANCHO, ALTO, texto.c_str(), 10, 50, 20);
    } else {
        int send_rows = rows_end - rows_start;
        draw_text_to_texture((unsigned char *)pixel_buffer, ANCHO, send_rows, texto.c_str(), 10, 50, 20);
    }
}

void setup_ui()
{
    texture_buffer = new uint32_t[ANCHO * ALTO];
    std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));

    gather_hist_buffer = new int[16 * nprocs];
    std::memset(gather_hist_buffer, 0, 16 * nprocs * sizeof(int));

    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Fractal Burning Ship - MPI");

    sf::Texture texture({ANCHO, ALTO});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    sf::Font font(arial_ttf::data, arial_ttf::data_len);
    
    // Configuración del Overlay superior
    sf::Text text(font, "", 18);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    // Leyenda de ayuda de teclado inferior
    std::string options = "Teclado: [Up/Down] max_iter | [Esc] Cerrar examen";
    sf::Text textoptions(font, options, 16);
    textoptions.setFillColor(sf::Color::Yellow);
    textoptions.setStyle(sf::Text::Bold);
    textoptions.setPosition({10, window.getView().getSize().y - 35});

    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                running = 0;
                window.close();
            }
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();
                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::Up:
                    max_iteraciones += 10;
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iteraciones -= 10;
                    if (max_iteraciones < 10) max_iteraciones = 10;
                    break;
                case sf::Keyboard::Scan::Escape:
                    running = 0;
                    window.close();
                    break;
                }
                std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));
            }
        }

        // 1. Comunicación Colectiva: MPI_Bcast de parámetros desde Rank 0
        double bcast_data[6] = {
            static_cast<double>(max_iteraciones),
            static_cast<double>(running),
            x_min, x_max, y_min, y_max
        };
        MPI_Bcast(bcast_data, 6, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if (running == 0) break;

        // Calcular porción propia de Rank 0
        burning_ship_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end, pixel_buffer, local_hist);
        std::memcpy(texture_buffer, pixel_buffer, ANCHO * delta * sizeof(uint32_t));

        // 2. Recepción P2P secuencial de imágenes de los esclavos (Rúbrica 4)
        for (int i = 1; i < nprocs; i++)
        {
            int new_delta = delta;
            if (i == nprocs - 1) { new_delta = delta - padding; }
            
            MPI_Status status;
            MPI_Recv(pixel_buffer, ANCHO * new_delta, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, &status);
            
            std::memcpy(texture_buffer + i * delta * ANCHO, pixel_buffer, ANCHO * new_delta * sizeof(uint32_t));

            int separator_y = i * delta - 1;
            draw_rank_bottom_line(texture_buffer, separator_y);
        }

        // 3. Comunicación Colectiva: MPI_Gather de Histogramas (Rúbrica 6)
        MPI_Gather(local_hist, 16, MPI_INT, gather_hist_buffer, 16, MPI_INT, 0, MPI_COMM_WORLD);

        // Consolidación de los bins recolectados de todos los procesos en un solo vector global
        std::memset(global_hist, 0, 16 * sizeof(int));
        for (int p = 0; p < nprocs; p++) {
            for (int b = 0; b < 16; b++) {
                global_hist[b] += gather_hist_buffer[p * 16 + b];
            }
        }

        dibujar_texto(0);
        texture.update((const uint8_t *)texture_buffer);

        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        // Construcción de la cadena del histograma para desplegar en el overlay
        std::string hist_str = "";
        for (int b = 0; b < 16; b++) {
            hist_str += fmt::format("{}:{} ", b, global_hist[b]);
        }

        // Renderizado del Overlay superior con las especificaciones del PDF
        auto msg = fmt::format(
            "RANKS Activos: {} | Iteraciones: {} | Dominio: X:[{:.2f},{:.2f}] Y:[{:.2f},{:.2f}]\nGlobal Hist: {}\nFPS: {}", 
            nprocs, max_iteraciones, x_min, x_max, y_min, y_max, hist_str, fps
        );
        text.setString(msg);

        window.clear();
        window.draw(sprite);
        window.draw(text);
        window.draw(textoptions);
        window.display();
    }

    delete[] gather_hist_buffer;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    init_freetype();    

    // Descomposición balanceada de ID de filas con cálculo preciso de padding
    delta = std::ceil(1.0 * ALTO / nprocs);
    rows_start = rank * delta;
    rows_end = rows_start + delta;
    padding = delta * nprocs - ALTO;

    if (rows_end > ALTO) { rows_end = ALTO; }

    pixel_buffer = new uint32_t[ANCHO * delta];
    std::memset(pixel_buffer, 0, ANCHO * delta * sizeof(uint32_t));
    fmt::print("Rank {}: procesando filas {} a la {}\n", rank, rows_start, rows_end);

    if (rank == 0)
    {
        setup_ui();
    }
    else
    {
        while (true)
        {
            // Escuchar el canal síncrono colectivo broadcast para actualizar parámetros de cálculo
            double bcast_data[6];
            MPI_Bcast(bcast_data, 6, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            
            max_iteraciones = static_cast<int>(bcast_data[0]);
            running = static_cast<int>(bcast_data[1]);
            x_min = bcast_data[2];
            x_max = bcast_data[3];
            y_min = bcast_data[4];
            y_max = bcast_data[5];

            if (running == 0)
            {
                fmt::print("Rank {}: Saliendo de forma controlada...\n", rank);
                break;
            }

            // Procesar el fragmento gráfico asignado y llenar el histograma local por píxel
            burning_ship_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end, pixel_buffer, local_hist);
            
            dibujar_texto(rank);

            int send_rows = rows_end - rows_start;
            // Envío punto a punto de la matriz de píxeles calculada hacia Rank 0
            MPI_Send(pixel_buffer, ANCHO * send_rows, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);

            // Sincronización colectiva obligatoria enviando el histograma local recolectado
            MPI_Gather(local_hist, 16, MPI_INT, nullptr, 16, MPI_INT, 0, MPI_COMM_WORLD);
        }
    }

    delete[] pixel_buffer;
    MPI_Finalize();
    return 0;
}