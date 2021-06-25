#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <vector>
#include <iostream>
#include <memory>

#include "fft.h"
#include "math.h"

using namespace std;

struct Sine
{
    double frequency; // [Hz]
    double offset;     // [s]
    int16_t amplitude;

    double value_at(double x) const
    {
        return sin(frequency * PI * (x - (PI / 2) - offset)) * amplitude;
    }
};

constexpr Sine generated_sines[] = {
    {1500, 0, 500},
    {2100, 0, 1000},
    {3800, 0, 1500},
    {4600, 0, 2000},
    {5900, 0, 2500},
    {6000, 0, 3000},
    {7200, 0, 3500},
    {8200, 0, 4000},
    {9400, 0, 4500},
    {10500, 0, 5000},
    {11700, 0, 5500}
};

class FFTCalculation
{
public:
    FFTCalculation(const vector<int16_t>& input, vector<fft::DoubleComplex>& output, size_t sample_rate)
    : m_input(input), m_output(output), m_sample_rate(sample_rate) {}

    const vector<fft::DoubleComplex>& output() const { return m_output; }

    void calculate();
    size_t sample_rate() const { return m_sample_rate; }

private:
    const vector<int16_t>& m_input;
    vector<fft::DoubleComplex>& m_output;
    size_t m_sample_rate = 0;
};

void FFTCalculation::calculate()
{
    // TODO: Windowing
    m_output.resize(m_input.size());
    fft::fft(m_output, m_input, m_input.size());
    fft::synthesize(m_output);
    m_output.resize(m_output.size()/2);

    fft::derivative(m_output);
}

int main(int argc, char* argv[])
{
    // Load or generate input
    vector<int16_t> input;
    vector<fft::DoubleComplex> output;

    std::unique_ptr<FFTCalculation> fft_calculation;

    sf::SoundBuffer buffer;

    if (argc == 1)
    {
        constexpr size_t generated_sample_rate = 44100;
        constexpr size_t generated_sample_count = 1 << 17;
        for (size_t i = 0; i < generated_sample_count; i++)
        {
            float t = 0;
            for (const Sine& sine: generated_sines)
            {
                t += sine.value_at(static_cast<double>(i) / generated_sample_rate);
            }
            input.push_back(int16_t(t));
            output.push_back(fft::DoubleComplex(t));
        }
        fft_calculation = std::make_unique<FFTCalculation>(input, output, generated_sample_rate);
        buffer.loadFromSamples(input.data(), input.size(), 1, 44100);
    }
    else if (argc == 2)
    {
        if (!buffer.loadFromFile(argv[1]))
        {
            std::cout << "Error: Could not load file " << argv[1] << std::endl;
            return 1;
        }
        input.resize(buffer.getSampleCount());
        for (size_t i = 0; i < buffer.getSampleCount(); i++)
        {
            input[i] = buffer.getSamples()[i];
        }
        fft_calculation = std::make_unique<FFTCalculation>(input, output, buffer.getSampleRate());
    }
    else
    {
        std::cout << "Usage: Voice [sound file]" << std::endl;
        return 1;
    }

    // Play the sound
    // TODO: this should be configurable
    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.play();

    // Calculate
    fft_calculation->calculate();
    size_t sample_count = fft_calculation->output().size();
    size_t sample_rate = fft_calculation->sample_rate();

    // Display
    float time_offset = 0;
    float amplitude_offset = 0;
    float zoom = 1;

    bool dragging = false;
    sf::Vector2i lastMousePos;

    sf::Vector2i window_size { 1920, 1000 };
    sf::RenderWindow window(sf::VideoMode(window_size.x, window_size.y), "Voice");

    // Load font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        std::cout << "Error: Failed to load font file" << std::endl;
        return 1;
    }

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
                else if (event.mouseWheelScroll.delta < 0 && zoom < (static_cast<double>(sample_count) / window_size.x / 2))
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
                if (dragging)
                {
                    time_offset += (lastMousePos.x - event.mouseMove.x) * zoom;
                    amplitude_offset += (lastMousePos.y - event.mouseMove.y) * zoom;
                    lastMousePos = { event.mouseMove.x, event.mouseMove.y };
                }
            }
            else if (event.type == sf::Event::Resized)
            {
                window_size.x = event.size.width;
                window_size.y = event.size.height;
                window.setView(sf::View(sf::FloatRect(0, 0, window_size.x, window_size.y)));
            }
        }
        time_offset = std::max(std::min(time_offset, static_cast<float>(sample_count)), 0.f);

        window.clear();

        const sf::Color COLOR_GRAY(100, 100, 100);
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, -amplitude_offset/zoom + window_size.y / 2.0), COLOR_GRAY),
            sf::Vertex(sf::Vector2f(1920, -amplitude_offset/zoom + window_size.y / 2.0), COLOR_GRAY)
        };
        window.draw(line, 2, sf::Lines);

        int sine_offset = 0;

        for (unsigned int i = time_offset; i < (input.size() / 4)-1; i++)
        {
            constexpr double WAVE_SCALE = 500;
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom, (-input[i]/WAVE_SCALE-amplitude_offset)/zoom + window_size.y / 2.0 + sine_offset), sf::Color(255, 0, 0)),
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom+1, (-input[i+1]/WAVE_SCALE-amplitude_offset)/zoom + window_size.y / 2.0 + sine_offset), sf::Color(255, 0, 0))
            };
            window.draw(line, 2, sf::Lines);
        }

        size_t displayed_samples = window_size.x * zoom;
        constexpr size_t label_spacing = 50;
        size_t step = std::max(static_cast<size_t>(1), displayed_samples / (window_size.x / label_spacing));
        for (size_t i = time_offset; i < std::min(sample_count/2, static_cast<size_t>(time_offset + displayed_samples)); i++)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom, (-output[i].real()-amplitude_offset)/zoom + window_size.y / 2.0)),
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom+1, (-output[i+1].real()-amplitude_offset)/zoom + window_size.y / 2.0))
            };
            window.draw(line, 2, sf::Lines);

            if (i % step == 0)
            {
                sf::Text text(to_string(static_cast<unsigned>(i / (static_cast<double>(sample_rate) / (sample_count / sqrt(2))))), font, 12);
                text.setStyle(sf::Text::Bold);
                text.setFillColor(sf::Color::White);
                text.setPosition((i-time_offset)/zoom, 10);
                window.draw(text);
            }
        }

        for (size_t i = time_offset; i < std::min(sample_count/2, static_cast<size_t>(time_offset + displayed_samples)); i++)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom, (output[i].imag()-amplitude_offset)/zoom + window_size.y / 2.0), sf::Color(0, 0, 255)),
                sf::Vertex(sf::Vector2f((i-time_offset)/zoom+1, (output[i+1].imag()-amplitude_offset)/zoom + window_size.y / 2.0), sf::Color(0, 0, 255))
            };
            window.draw(line, 2, sf::Lines);
        }

        for(unsigned int i = 0; i < fft::peaks.size(); i++){
            if(i % 3 == 1){
                sf::Text text(to_string(int(fft::peaks[i]*Constant)), font, 12);
                text.setStyle(sf::Text::Bold);
                text.setFillColor(sf::Color::Green);
                text.setPosition((fft::peaks[i] - time_offset - to_string(fft::peaks[i]).size()*4) / zoom + (1 / zoom) * 12, (-output[fft::peaks[i]].real() - amplitude_offset) / zoom + window_size.y / 2.0 - 20);
                window.draw(text);
            }else if(i % 3 == 2)
            {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f((fft::peaks[i] - time_offset - to_string(fft::peaks[i]).size()*4) / zoom, 0), sf::Color(192, 192, 192)),
                    sf::Vertex(sf::Vector2f((fft::peaks[i] - time_offset - to_string(fft::peaks[i]).size()*4) / zoom, window_size.y), sf::Color(192, 192, 192))
                };
                window.draw(line, 2, sf::Lines);
            }else if(i % 3 == 0)
            {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f((fft::peaks[i] - time_offset - to_string(fft::peaks[i]).size()*4) / zoom, 0), sf::Color::Blue),
                    sf::Vertex(sf::Vector2f((fft::peaks[i] - time_offset - to_string(fft::peaks[i]).size()*4) / zoom, window_size.y), sf::Color::Blue)
                };
                window.draw(line, 2, sf::Lines);
            }
        }

        window.display();
    }
    return 0;
}
