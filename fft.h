#pragma once

#include <complex>
#include <vector>

namespace fft
{

using std::complex;
using std::vector;

using DoubleComplex = complex<double>;

void fft(vector<DoubleComplex>& output, const vector<int16_t>& input, size_t window_size, size_t offset = 0);
void synthesize(vector<fft::DoubleComplex>& data);
void deriative(vector<DoubleComplex>& x);
}
