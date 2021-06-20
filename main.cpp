#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <vector>
#include <iostream>

#include "fft.h"
#include "math.h"

using namespace std;

constexpr float sines[3][6] = {
    {900, 1200, 2500, 5000, 7200, 9100},
    {0.2, 0.5, 0, 0.1, 1, 0.2},
    {2000, 3000, 1500, 2500, 3500, 1000},
};

float calSine(int a, float x)
{
    return (sin(sines[0][a]*PI*((x/44100)-(PI/2)-sines[1][a]))+1)*sines[2][a];
}

int main()
{
    // Load font
    sf::Font font;
    font.loadFromFile("arial.ttf");

    constexpr int samples = 131072;

    // Generate input
    vector<int16_t> input;
    vector<complex<double>> output;
    for(int i = 0; i < samples; i++)
    {
        float t = 0;
        for(int j = 0; j < 6; j++)
        {
            t += calSine(j, i);
        }
        input.push_back(int16_t(t));
        output.push_back(fft::DoubleComplex(t));
    }

    // Setup SFML buffer
    sf::SoundBuffer buffer;
    buffer.loadFromSamples(input.data(), input.size(), 1, 44100);
    sf::Sound sound1;
    sound1.setBuffer(buffer);

    // Calculate FFT of input
    output.resize(samples);
    fft::fft(output, input, samples);
    fft::synthesize(output);

    // Display
    float time_offset = 0;
    float amplitude_offset = 0;
    float zoom = 1;

    bool dragging = false;
    sf::Vector2i lastMousePos;

    // TODO: Do not hardcode window size!
    sf::RenderWindow window(sf::VideoMode(1920, 1000), "3D");

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::D)
                    time_offset+=30*zoom;
                else if (event.key.code == sf::Keyboard::A)
                    time_offset-=30*zoom;
                else if (event.key.code == sf::Keyboard::W)
                    amplitude_offset+=30;
                else if (event.key.code == sf::Keyboard::S)
                    amplitude_offset-=30;
            }
            else if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.delta > 0 && zoom > 0.125)
                    zoom /= 2;
                else if (event.mouseWheelScroll.delta < 0 && zoom < (samples / 1920 / 2))
                    zoom *= 2;
            }
            else if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                dragging = true;
                lastMousePos = {event.mouseButton.x, event.mouseButton.y};
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                dragging = false;
            }
            else if (event.type == sf::Event::MouseMoved)
            {
                if(dragging)
                {
                    // TODO: Amplitude should be affected by zoom
                    time_offset += (lastMousePos.x - event.mouseMove.x) * zoom;
                    amplitude_offset += (lastMousePos.y - event.mouseMove.y) * zoom;
                    lastMousePos = { event.mouseMove.x, event.mouseMove.y };
                }
            }
        }
        time_offset = std::max(std::min(time_offset, static_cast<float>(samples)), 0.f);

        window.clear();

        const sf::Color COLOR_GRAY(100, 100, 100);
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, -amplitude_offset/zoom+500), COLOR_GRAY),
            sf::Vertex(sf::Vector2f(1920, -amplitude_offset/zoom+500), COLOR_GRAY)
        };
        window.draw(line, 2, sf::Lines);

        for (unsigned int i = time_offset; i < (input.size() / 2)-1; i++)
        {
            constexpr double WAVE_SCALE = 1000;
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom, (-input[i]/WAVE_SCALE-amplitude_offset)/zoom+500), sf::Color(255, 0, 0)),
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom+1, (-input[i+1]/WAVE_SCALE-amplitude_offset)/zoom+500), sf::Color(255, 0, 0))
            };
            window.draw(line, 2, sf::Lines);
        }

        size_t displayed_samples = 1920*zoom;
        size_t step = std::max(static_cast<size_t>(1), displayed_samples / 20);
        for (int i = (int)time_offset; i < std::min(samples/2, (int)(time_offset+displayed_samples)); i++)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom, (-output[i].real()-amplitude_offset)/zoom+500)),
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom+1, (-output[i+1].real()-amplitude_offset)/zoom+500))
            };
            window.draw(line, 2, sf::Lines);

            if(i % step == 0)
            {
                sf::Text text(to_string(i), font);
                text.setCharacterSize(14);
                text.setStyle(sf::Text::Bold);
                text.setFillColor(sf::Color::White);
                text.setPosition((i-time_offset)/zoom, 10);
                window.draw(text);
            }
        }

        window.display();
    }
    return 0;
}
