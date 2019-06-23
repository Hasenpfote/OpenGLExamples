/**
* @file utility.h
* @brief math utility.
* @author Hasenpfote
* @date 2016/04/16
*/
#pragma once
#include <cstdint>
#include "constants/constants.h"

namespace hasenpfote{ namespace math{

/*!
 * a と b がほぼ等しいか
 * @param a
 * @param b
 * @param max_ulps unit in the last place.
 * @return a ≈ b
 */
bool almost_equals(float a, float b, std::uint32_t max_ulps);
bool almost_equals(double a, double b, std::uint64_t max_ulps);

}}