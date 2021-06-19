#pragma once

#include <complex>
#include <vector>

namespace fft
{

using std::complex;
using std::vector;

using DoubleComplex = complex<double>;

using Iterator = vector<complex<double>>::iterator;
using ConstIterator = vector<int16_t>::const_iterator;

void fft(Iterator Xbegin, Iterator Xend, ConstIterator xbegin, ConstIterator xend, int s = 1);

}
