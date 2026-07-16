#include <iostream>
#include <complex>
#include <chrono>
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
    if (const char *name = std::getenv("COMPUTERNAME")) {
        return name;
    }
    #else
    if (const char *name = std::getenv("HOSTNAME")) {
        return name;
    }
    #endif
    return "unknown";
}

int max_iteraciones = 50;
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;
double c_real = 0.0; // Dummy para cumplir con la rúbrica de Bcast
double c_imag = 0.0; // Dummy para cumplir con la rúbrica de Bcast

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

// Variables globales para la reducción que alimentarán el Overlay
double max_compute_ms = 0.0;
unsigned long long global_total_iters = 0;

void draw_rank_bottom_line(uint32_t *buffer, int y)
{
    if (!buffer || y < 0 || y >= ALTO) {
        return;
    }
    for (int x = 0; x < ANCHO; x++) {
        buffer[y * ANCHO + x] = 0xFFFFFFFF;
    }
}

void dibujar_texto(int rank)
{
    auto texto = fmt::format("RANK_{} - {}", rank, machine_name());
    if (rank == 0) {
        draw_text_to_texture((unsigned char *)texture_buffer,
                             ANCHO, ALTO,
                             texto.c_str(),
                             10, 25, 20);
    } else {
        int send_rows = rows_end - rows_start;
        draw_text_to_texture((unsigned char *)pixel_buffer,
                             ANCHO, send_rows,
                             texto.c_str(),
                             10, 25, 20);
    }
}

void setup_ui()
{
    texture_buffer = new uint32_t[ANCHO * ALTO];
    std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));

    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Fractal de Newton - MPI");

    sf::Texture texture({ANCHO, ALTO});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    sf::Font font(arial_ttf::data, arial_ttf::data_len);
    
    // Texto superior con las estadísticas completas
    sf::Text text(font, "", 20);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    // Leyenda de controles en la esquina inferior
    std::string options = "Controles: [Up/Down] Iteraciones | [Esc] Salir";
    sf::Text textoptions(font, options, 18);
    textoptions.setFillColor(sf::Color::Cyan);
    textoptions.setStyle(sf::Text::Bold);
    textoptions.setPosition({10, window.getView().getSize().y - 40});

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
                    if (max_iteraciones < 10)
                        max_iteraciones = 10;
                    break;
                case sf::Keyboard::Scan::Escape:
                    running = 0;
                    window.close();
                    break;
                }
                std::memset(texture_buffer, 0, ANCHO * ALTO * sizeof(uint32_t));
            }
        }

        // 1. MPI_Bcast de parámetros requeridos (Rúbrica 5)
        double bcast_data[8] = {
            static_cast<double>(max_iteraciones),
            static_cast<double>(running),
            x_min, x_max, y_min, y_max,
            c_real, c_imag
        };
        MPI_Bcast(bcast_data, 8, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if (running == 0)
        {
            break;
        }

        // 2. Medir tiempo de cálculo local para Rank 0
        unsigned long long local_iters = 0;
        auto start_time = std::chrono::high_resolution_clock::now();
        
        newton_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end, pixel_buffer, local_iters);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        double local_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

        std::memcpy(texture_buffer, pixel_buffer, ANCHO * delta * sizeof(uint32_t));

        // 3. Recepción de sub-bloques calculados (Rúbrica 4)
        for (int i = 1; i < nprocs; i++)
        {
            int new_delta = delta;
            if (i == nprocs - 1)
            {
                new_delta = delta - padding; 
            }
            MPI_Status status;
            MPI_Recv(
                pixel_buffer,
                ANCHO * new_delta,
                MPI_UNSIGNED,
                i, 
                0, 
                MPI_COMM_WORLD,
                &status);
            
            std::memcpy(texture_buffer + i * delta * ANCHO, pixel_buffer, ANCHO * new_delta * sizeof(uint32_t));

            int separator_y = i * delta - 1;
            draw_rank_bottom_line(texture_buffer, separator_y);
        }

        // 4. Reducción colectiva global (Rúbrica 6)
        MPI_Reduce(&local_time_ms, &max_compute_ms, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_iters, &global_total_iters, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

        dibujar_texto(0);
        texture.update((const uint8_t *)texture_buffer);

        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        // Overlay con toda la información requerida
        auto msg = fmt::format(
            "RANK: 0 | max_iter: {} | max_compute: {:.2f} ms | total_iters: {} | FPS: {}", 
            max_iteraciones, max_compute_ms, global_total_iters, fps
        );
        text.setString(msg);

        window.clear();
        window.draw(sprite);
        window.draw(text);
        window.draw(textoptions);
        window.display();
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    init_freetype();    

    // Descomposición de filas con padding dinámico
    delta = std::ceil(1.0 * ALTO / nprocs);
    rows_start = rank * delta;
    rows_end = rows_start + delta;
    padding = delta * nprocs - ALTO;

    if (rows_end > ALTO)
    {
        rows_end = ALTO;
    }

    pixel_buffer = new uint32_t[ANCHO * delta];
    std::memset(pixel_buffer, 0, ANCHO * delta * sizeof(uint32_t));
    fmt::print("Rank {}: rows {} to {}\n", rank, rows_start, rows_end);

    if (rank == 0)
    {
        setup_ui();
    }
    else
    {
        while (true)
        {
            // Escuchar el broadcast en cada ciclo
            double bcast_data[8];
            MPI_Bcast(bcast_data, 8, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            
            max_iteraciones = static_cast<int>(bcast_data[0]);
            running = static_cast<int>(bcast_data[1]);
            x_min = bcast_data[2];
            x_max = bcast_data[3];
            y_min = bcast_data[4];
            y_max = bcast_data[5];
            c_real = bcast_data[6];
            c_imag = bcast_data[7];

            if (running == 0)
            {
                fmt::print("Rank {}: received shutdown signal, exiting...\n", rank);
                break;
            }

            // Realizar cálculo del sub-bloque y medir métricas locales
            unsigned long long local_iters = 0;
            auto start_time = std::chrono::high_resolution_clock::now();
            
            newton_mpi(x_min, y_min, x_max, y_max, ANCHO, ALTO, rows_start, rows_end, pixel_buffer, local_iters);
            
            auto end_time = std::chrono::high_resolution_clock::now();
            double local_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

            dibujar_texto(rank);

            int send_rows = rows_end - rows_start;
            MPI_Send(
                pixel_buffer,
                ANCHO * send_rows,
                MPI_UNSIGNED,
                0, 
                0, 
                MPI_COMM_WORLD);

            // Participar en las reducciones globales obligatorias
            double dummy_max_compute = 0.0;
            unsigned long long dummy_total_iters = 0;
            MPI_Reduce(&local_time_ms, &dummy_max_compute, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
            MPI_Reduce(&local_iters, &dummy_total_iters, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
        }
    }

    delete[] pixel_buffer;
    MPI_Finalize();
    return 0;
}