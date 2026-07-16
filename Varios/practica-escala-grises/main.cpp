#include <iostream>
#include <optional>

#include <SFML/Graphics.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "filtros.h"

// Metodo para preparar la textura
// Adapta la imagen para que SFML pueda dibujarla
void preparar_textura_gris(uint8_t *rgba_gris, uint8_t *rgba_buffer, int total_pixeles)
{
    for (int i = 0; i < total_pixeles; i++)
    {
        rgba_buffer[i * 4] = rgba_gris[i];     // R
        rgba_buffer[i * 4 + 1] = rgba_gris[i]; // G
        rgba_buffer[i * 4 + 2] = rgba_gris[i]; // B
        rgba_buffer[i * 4 + 3] = 255;          // Alpha (Opacidad total)
    }
}

int main()
{
    // Cargar Imagen
    int width, height, channels;
    uint8_t *rgba_pixels = stbi_load("img.jpg", &width, &height, &channels, STBI_rgb_alpha);

    // Preparar la memoria y la ventana
    int total_pixeles = width * height;
    uint8_t *gray_pixels = new uint8_t[total_pixeles];
    uint8_t *display_buffer = new uint8_t[total_pixeles * 4];

    sf::RenderWindow window(sf::VideoMode({(unsigned int)width, (unsigned int)height}), "Escala de Grises");
    sf::Texture texture({(unsigned int)width, (unsigned int)height});
    sf::Sprite sprite(texture);

    // Actualizar textura
    texture.update(rgba_pixels);

    int ultimo_filtro = 0;

    // Bucle principal del programa
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();
                if (!evt)
                {
                    continue;
                }
                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::Num1:
                    // Original
                    texture.update(rgba_pixels);
                    break;
                case sf::Keyboard::Scan::Num2:
                    // Aplicar Simd
                    filtro_simd(rgba_pixels, gray_pixels, width, height);
                    preparar_textura_gris(gray_pixels, display_buffer, total_pixeles);

                    texture.update(display_buffer);
                    ultimo_filtro = 2;
                    break;
                case sf::Keyboard::Scan::Num3:
                    // Aplicar OpenMP
                    filtro_openmp(rgba_pixels, gray_pixels, width, height);
                    preparar_textura_gris(gray_pixels, display_buffer, total_pixeles);

                    texture.update(display_buffer);
                    ultimo_filtro = 3;
                    break;
                case sf::Keyboard::Scan::S:
                    // Guardar la imagen
                    if (ultimo_filtro == 2)
                    {
                        stbi_write_png("img-gris_simd.png", width, height, STBI_grey, gray_pixels, width);
                        std::cout << "Imagen Simd Guardada \n";
                    }
                    else if (ultimo_filtro == 3)
                    {
                        stbi_write_png("img-gris_openmp.png", width, height, STBI_grey, gray_pixels, width);
                        std::cout << "Imagen OpenMP Guardada \n";
                    }
                    break;
                }
            }
        }
        // Dibujar en Pantalla
        window.clear();
        window.draw(sprite);
        window.display();
    }
    //Limpiar la memoria
    delete[] gray_pixels;
    delete[] display_buffer;
    stbi_image_free(rgba_pixels);

    return 0;
}