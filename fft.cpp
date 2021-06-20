#include "fft.h"

#include "math.h"

#include <cmath>

namespace fft
{

using OutputIterator = vector<DoubleComplex>::iterator;
using InputIterator = vector<int16_t>::const_iterator;

void fft_impl(OutputIterator output_begin, OutputIterator output_end, InputIterator input_begin, InputIterator input_end, int s)
{
    double N = input_end - input_begin;
    if (N == 1)
        *output_begin = complex<double>(*input_begin);
    else
    {
        fft_impl(output_begin, output_begin + N/2.0, input_begin, input_begin + N/2.0, 2*s);
        fft_impl(output_begin + N/2.0, output_end, input_begin + s, input_begin + s + N/2.0, 2*s);
        for (double k = 0; k < N/2.0; k++)
        {
            auto& begin_k = *(output_begin + k);
            auto& begin_k_n_2 = *(output_begin + k + N/2.0);
            auto p = begin_k;
            auto q = exp(complex<double>(-2*PI*complex<double>(0, 1)*k/N)) * begin_k_n_2;
            begin_k = p + q;
            begin_k_n_2 = p - q;
        }
    }
}

void fft(vector<DoubleComplex>& output, vector<int16_t>& input, size_t window_size, size_t offset)
{
    fft_impl(output.begin() + offset, output.begin() + offset + window_size, input.begin() + offset, input.begin() + offset + window_size, 1);
}

void synthesize(vector<fft::DoubleComplex>& data)
{
    double n = data.size();
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

}
