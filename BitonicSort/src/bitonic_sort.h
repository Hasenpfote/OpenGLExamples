#pragma once

namespace bitonic_sort
{

using params = std::tuple<int, int, int, int, int>;

double next_lower_power_of_two(double x);
double next_higher_power_of_two(double x);
int get_num_passes(int N);
params get_params(int pass);

}