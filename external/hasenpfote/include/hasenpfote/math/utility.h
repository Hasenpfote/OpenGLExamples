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

#if 0
/*!
 * degrees to radians.
 * @param[in] angle an angle in degrees.
 * @return an angle in radians.
 */
constexpr float to_radians(float angle)
{
    return angle * constants::pi<float>() / 180.0f;
}

/*!
 * radians to degrees.
 * @param[in] angle an angle in radians.
 * @return an angle in degrees.
 */
constexpr float to_degrees(float angle)
{
    return angle * 180.0f / constants::pi<float>();
}

/*!
 * 値 x を [min, max] の範囲内に制限する.
 * @param[in] x
 * @param[in] min
 * @param[in] max
 * @return 制限された値 x
 */
template <typename T>
T clamp(T x, T min, T max)
{
    if(x < min)
        return min;
    if(x > max)
        return max;
    return x;
}

/*!
 * [min1, max1] の範囲内にある x を [min2, max2] に写像する.
 * @param[in] x
 * @param[in] min1
 * @param[in] max1
 * @param[in] min2
 * @param[in] max2
 * @return 写像された値 x
 */
template<typename T>
T remap(T x, T min1, T max1, T min2, T max2)
{
    return (((x - min1) / (max1 - min1)) * (max2 - min2)) + min2;
}

/*!
 * 値 x が 閉区間 [lower, upper] に含まれるか.
 * @param[in] x
 * @param[in] lower
 * @param[in] upper
 * @retval true 含む
 * @retval false 含まない
 */
template <typename T>
bool contains_closed(T x, T lower, T upper)
{
    if(x < lower || x > upper)
        return false;
    return true;
}

/*!
* 値 x が 開区間 (lower, upper) に含まれるか.
* @param[in] x
* @param[in] lower
* @param[in] upper
* @retval true 含む
* @retval false 含まない
*/
template <typename T>
bool contains_open(T x, T lower, T upper)
{
    if(x <= lower || x >= upper)
        return false;
    return true;
}
#endif

}}