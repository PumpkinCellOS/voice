#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <vector>
#include <cmath>
#include <memory>
#include <iostream>
#include <complex>

using namespace std;

float sines[3][6] = {
                   {900, 1200, 2500, 5000, 7200, 9100},
                   {0.2, 0.5, 0, 0.1, 1, 0.2},
                   {2000, 3000, 1500, 2500, 3500, 1000},
                    };

vector<int16_t> vec;
vector<complex<double>> vec2;

using cd = complex<double>;
const double PI = acos(-1);

using Iterator = vector<complex<double>>::iterator;
using ConstIterator = vector<int16_t>::const_iterator;

void fft(Iterator Xbegin, Iterator Xend, ConstIterator xbegin, ConstIterator xend, int s = 1)
{
    double N = xend - xbegin;
    if(N == 1)
        *Xbegin = complex<double>(*xbegin);
    else
    {
        fft(Xbegin, Xbegin + N/2.0, xbegin, xbegin + N/2.0, 2*s);
        fft(Xbegin + N/2.0, Xend, xbegin + s, xbegin + s + N/2.0, 2*s);
        for(double k = 0; k < N/2.0; k++)
        {
            auto& begin_k = *(Xbegin + k);
            auto& begin_k_n_2 = *(Xbegin + k + N/2.0);
            auto p = begin_k;
            auto q = exp(complex<double>(-2*M_PI*complex<double>(0, 1)*k/N)) * begin_k_n_2;
            begin_k = p + q;
            begin_k_n_2 = p - q;
        }
    }
}

void sinthesize(double n)
{
    double n2 = n * n;
    vec2[0] = cd(10.0 * log10(vec2[0].real() * vec2[0].real() / n2), vec2[0].imag());
    vec2[n / 2] = cd(10.0 * log10(vec2[1].real() * vec2[1].real() / n2), vec2[n/2].imag());
    for (int i = 1; i < n / 2; i++) {
        double val = vec2[i * 2].real() * vec2[i * 2].real() + vec2[i * 2 + 1].real() * vec2[i * 2 + 1].real();
        val /= n2;
        vec2[i] = cd(10.0 * log10(val), vec2[i].imag());
    }
}

float calSine(int a, float x)
{
    return (sin(sines[0][a]*PI*((x/44100)-(PI/2)-sines[1][a]))+1)*sines[2][a];
}

int main()
{
    sf::Font font;
    font.loadFromFile("arial.ttf");

    sf::SoundBuffer buffer;

    int samples = 131072;

    for(int i = 0; i < samples; i++){
        float t = 0;
        for(int j = 0; j < 6; j++){
            t+=calSine(j, i);
        }
        vec.push_back(int16_t(t));
        vec2.push_back(cd(t));
    }

    buffer.loadFromSamples(vec.data(), vec.size(), 1, 44100);

    sf::Sound sound1;
    sound1.setBuffer(buffer);

    //sound1.play();

    float t = 0;
    int a = 1000;
    float zoom = 1;

    vec2.resize(samples);

    fft(vec2.begin(), vec2.end(), vec.begin(), vec.end());

    sinthesize(double(vec2.size()));

    cout << endl;

    //int samplerate = buffer.getSampleRate();

    sf::RenderWindow window(sf::VideoMode(1920, 1000), "3D");

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::D)
                    t+=30*zoom;
                else if (event.key.code == sf::Keyboard::A)
                    t-=30*zoom;
                else if (event.key.code == sf::Keyboard::W)
                    a+=30;
                else if (event.key.code == sf::Keyboard::S)
                    a-=30;
                t = std::max(std::min(t, static_cast<float>(samples)), 0.f);
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

        for(unsigned int i = t; i < (vec.size() / 2)-1; i++)
        {
            constexpr double WAVE_SCALE = 1000;
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-t)/zoom, -vec[i]/WAVE_SCALE/zoom+a), sf::Color(255, 0, 0)),
                sf::Vertex(sf::Vector2f((i-t)/zoom+1, -vec[i+1]/WAVE_SCALE/zoom+a), sf::Color(255, 0, 0))
            };
            window.draw(line, 2, sf::Lines);
        }

        size_t displayed_samples = 1920*zoom;
        size_t step = std::max(static_cast<size_t>(1), displayed_samples / 20);
        for(int i = (int)t; i < std::min(samples/2, (int)(t+displayed_samples)); i++)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((i-t)/zoom, -vec2[i].real()/zoom+a)),
                sf::Vertex(sf::Vector2f((i-t)/zoom+1, -vec2[i+1].real()/zoom+a))
            };
            window.draw(line, 2, sf::Lines);

            if(i % step == 0)
            {
                sf::Text text(to_string(i), font);
                text.setCharacterSize(14);
                text.setStyle(sf::Text::Bold);
                text.setFillColor(sf::Color::White);
                text.setPosition((i-t)/zoom, 10);
                window.draw(text);
            }
        }
        window.display();
    }
    return 0;
}
