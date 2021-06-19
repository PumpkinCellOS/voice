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

void sinthesize(vector<fft::DoubleComplex>& data, double n)
{
    double n2 = n * n;
    data[0] = fft::DoubleComplex(10.0 * log10(data[0].real() * data[0].real() / n2), data[0].imag());
    data[n / 2] = fft::DoubleComplex(10.0 * log10(data[1].real() * data[1].real() / n2), data[n/2].imag());
    for (int i = 1; i < n / 2; i++)
    {
        double val = data[i * 2].real() * data[i * 2].real() + data[i * 2 + 1].real() * data[i * 2 + 1].real();
        val /= n2;
        data[i] = fft::DoubleComplex(10.0 * log10(val), data[i].imag());
    }

    // Clamp everything to 0.
    for (auto& sample: data)
    {
        sample = fft::DoubleComplex(std::max(0.0, sample.real()), std::max(0.0, sample.imag()));
    }
}

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
    fft::fft(output.begin(), output.end(), input.begin(), input.end());
    sinthesize(output, double(output.size()));

    // Display result
    float time_offset = 0;
    int amplitude_offset = 1000;
    float zoom = 1;

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
                time_offset = std::max(std::min(time_offset, static_cast<float>(samples)), 0.f);
            }
            else if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.delta > 0 && zoom > 0.01)
                    zoom /= 2;
                else if (event.mouseWheelScroll.delta < 0)
                    zoom *= 2;
            }
            else if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        const sf::Color COLOR_GRAY(100, 100, 100);
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, amplitude_offset), COLOR_GRAY),
            sf::Vertex(sf::Vector2f(1920, amplitude_offset), COLOR_GRAY)
        };
        window.draw(line, 2, sf::Lines);

        for(unsigned int i = time_offset; i < (input.size() / 2)-1; i++)
        {
            constexpr double WAVE_SCALE = 1000;
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom, -input[i]/WAVE_SCALE/zoom+amplitude_offset), sf::Color(255, 0, 0)),
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom+1, -input[i+1]/WAVE_SCALE/zoom+amplitude_offset), sf::Color(255, 0, 0))
            };
            window.draw(line, 2, sf::Lines);
        }

        size_t displayed_samples = 1920*zoom;
        size_t step = std::max(static_cast<size_t>(1), displayed_samples / 20);
        for(int i = (int)time_offset; i < std::min(samples/2, (int)(time_offset+displayed_samples)); i++)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom, -output[i].real()/zoom+amplitude_offset)),
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom+1, -output[i+1].real()/zoom+amplitude_offset))
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
