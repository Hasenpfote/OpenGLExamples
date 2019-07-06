#include <cmath>
#include "bayer.h"

std::vector<float> make_bayer_matrix(std::size_t level)
{
    auto dim = static_cast<std::size_t>(std::pow(2.0f, level));
    auto size = dim * dim;

    std::vector<float> matrix(size, 0.0f);

    const float biases[] = { 0.0f, 2.0f, 3.0f, 1.0f };

    for(decltype(level) l = 1; l <= level; l++)
    {
        auto _dim = static_cast<std::size_t>(std::pow(2.0f, l));
        auto _n = _dim / 2;
        auto _coef = _dim * _dim;

        for(decltype(size) s = 0; s < size; s++)
        {
            auto row = s / dim;
            auto col = s % dim;

            auto row2 = (row / _n) % 2;
            auto col2 = (col / _n) % 2;

            auto index = 2 * row2 + col2;

            matrix[s] = (_coef * matrix[s] + biases[index]) / _coef;
        }
    }
    return matrix;
}