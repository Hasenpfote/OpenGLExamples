#include <iterator>
#include <cstdlib>
#include <cassert>
#include <array>
#include <cmath>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include "bitonic_sort.h"

namespace bitonic_sort
{

static int solve_quadratic_equation(double a, double b, double c, std::array<double, 2> & roots)
{
    double d = b * b - 4.0 * a * c;
    if(d < 0.0)
        return 0;   // equation has imaginary roots.

    double x = (std::abs(b) + std::sqrt(d)) / (2.0 * a);
    if(d > 0.0)
    {
        // equation has 2 roots.
        if(b < 0.0) // negative sign
        {
            // x1 = (- b - sqrt(D)) / 2a = (- (-|b|) - sqrt(D)) / 2a = (|b| - sqrt(D)) / 2a ... When b is larger, There is a possibility to underflow.
            // x2 = (- b + sqrt(D)) / 2a = (- (-|b|) + sqrt(D)) / 2a = (|b| + sqrt(D)) / 2a
            roots[0] = c / (a * x);	// rationalize the numerator
            roots[1] = x;
        }
        else        // positive sign
        {
            // x1 = (- b - sqrt(D)) / 2a = (- (+|b|) - sqrt(D)) / 2a = (-|b| - sqrt(D)) / 2a = -(|b| + sqrt(D)) / 2a
            // x2 = (- b + sqrt(D)) / 2a = (- (+|b|) + sqrt(D)) / 2a = (-|b| + sqrt(D)) / 2a = -(|b| - sqrt(D)) / 2a ... When b is larger, There is a possibility to underflow.
            roots[0] = -x;
            roots[1] = -(c / (a * x));	// rationalize the numerator
        }
        return 2;
    }

    // equation has 1 root
    roots[0] = roots[1] = (b < 0.0) ? x : -x;
    return 1;
}

static int pass_to_step(int pass)
{
    // ax^2 + bx + c = 0
    double a = 1.0;
    double b = 1.0;
    double c = -2.0 * static_cast<double>(pass);
    std::array<double, 2> roots;
    auto ret = solve_quadratic_equation(a, b, c, roots);
    assert(ret > 0);
    auto root = std::max(roots[0], roots[1]);

    return static_cast<int>(std::ceil(root));
}

static int pass_to_stage(int pass, int step)
{
    auto sum = step * (step + 1) / 2;
    return sum - pass + 1;
}

static int stage_to_offset(int stage)
{
    return std::pow(2.0, stage - 1);
}

double next_lower_power_of_two(double x)
{
    return std::pow(2.0, std::floor(std::log2(x)));
}

double next_higher_power_of_two(double x)
{
    return std::pow(2.0, std::ceil(std::log2(x)));
}

int get_num_passes(int N)
{
    auto n = static_cast<int>(std::log2(N));
    return n * (n + 1) / 2;
}

params get_params(int pass)
{
    auto step = pass_to_step(pass);
    auto stage = pass_to_stage(pass, step);
    auto seq_size = static_cast<int>(std::pow(2.0, step));
    auto offset = stage_to_offset(stage);
    auto range = offset * 2;

    return std::make_tuple(step, stage, seq_size, offset, range);
}

}