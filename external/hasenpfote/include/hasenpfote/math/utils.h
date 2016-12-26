/*!
 * @file utils.h
 * @brief Math utility functions.
 * @author Hasenpfote
 */
#pragma once

#include "constants/constants.h"

namespace hasenpfote{ namespace math{

/*!
 * Convert from degrees to radians.
 * @param [in] angle     an angle in degrees.
 * @return an angle in radians.
 */
template<typename T>
constexpr T ConvertDegreesToRadians(T angle)
{
    return angle * constants::pi<T>() / static_cast<T>(180);
}

/*!
 * Convert from radians to degrees.
 * @param [in] angle     an angle in radians.
 * @return an angle in degrees.
 */
template<typename T>
constexpr T ConvertRadiansToDegrees(T angle)
{
    return angle * static_cast<T>(180) / constants::pi<T>();
}

/*!
 * 値 x を [min, max] の範囲内に制限する.
 * @param[in] x
 * @param[in] min
 * @param[in] max
 * @return 制限された値 x
 */
template <typename T>
T Clamp(T x, T min, T max)
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
T Remap(T x, T min1, T max1, T min2, T max2)
{
    return (((x - min1) / (max1 - min1)) * (max2 - min2)) + min2;
}

/*!
 * Whether the number includes on the open interval.
 * @param[in] x
 * @param[in] lower
 * @param[in] upper
 * @return which returns true if the number includes on the open interval or false otherwise.
 */
template <typename T>
bool IncludesOnOpenInterval(T x, T lower, T upper)
{
    if(x <= lower || x >= upper)
        return false;
    return true;
}

/*!
 * Whether the number includes on the closed interval.
 * @param[in] x
 * @param[in] lower
 * @param[in] upper
 * @return which returns true if the number includes on the closed interval or false otherwise.
 */
template <typename T>
bool IncludesOnClosedInterval(T x, T lower, T upper)
{
    if(x < lower || x > upper)
        return false;
    return true;
}

}}