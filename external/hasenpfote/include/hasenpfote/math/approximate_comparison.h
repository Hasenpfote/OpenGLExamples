/*!
 * @file approximate_comparison.h
 * @brief Floating point number comparison.
 * @author Hasenpfote
 */
#pragma once
#include <type_traits>
#include <limits>
#include <cmath>
#include <algorithm>

namespace hasenpfote{ namespace math{

/*!
 * Whether two numbers are close within the given absolute tolerance.
 * @param [in] a    the first number to compare.
 * @param [in] b    the second number to compare.
 * @param [in] tolerance    the absolute tolerance to use.
 * @return which returns true if two numbers are close within the given tolerance or false otherwise.
 */
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
bool AreCloseWithinAbsoluteTolerance(T a, T b, T tolerance)
{
    return std::abs(a - b) <= tolerance;
}

/*!
 * Whether two numbers are close within the given relative tolerance.
 * @param [in] a    the first number to compare.
 * @param [in] b    the second number to compare.
 * @param [in] tolerance    the relative tolerance to use.
 * @return which returns true if two numbers are close within the given tolerance or false otherwise.
 */
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
bool AreCloseWithinRelativeTolerance(T a, T b, T tolerance = std::numeric_limits<T>::epsilon())
{
    return std::abs(a - b) <= std::max(std::abs(a), std::abs(b)) * tolerance;
}

/*!
 * Whether the number is close to zero within the given absolute tolerance.
 * @param [in] a    the number to compare.
 * @param [in] tolerance    the absolute tolerance to use.
 * @return which returns true if the number is close to zero within the given tolerance or false otherwise.
 */
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
bool IsCloseToZero(T a, T tolerance)
{
    return std::abs(a) <= tolerance;
}

/*!
 * Whether two numbers are approximately equal.
 * @param [in] a    the first number to compare.
 * @param [in] b    the second number to compare.
 * @param [in] abs_tolerance    the absolute tolerance to use -- the value should be less than relative tolerance.
 * @param [in] rel_tolerance    the relative tolerance to use -- the value should be greater than or equal to epsilon.
 * @return which returns true if 'a' is approximately equal to 'b' or false otherwise.
 */
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
bool IsApproximatelyEqualTo(T a, T b, T abs_tolerance, T rel_tolerance = std::numeric_limits<T>::epsilon())
{
    // |a - b| <= max(abs_tolerance, max(|a|, |b|) * rel_tolerance)
    return std::abs(a - b) <= std::max(abs_tolerance, std::max(std::abs(a), std::abs(b)) * rel_tolerance);
}

/*!
 * Whether 'a' is definitely greater than 'b'.
 * @param [in] a    the first number to compare.
 * @param [in] b    the second number to compare.
 * @param [in] abs_tolerance    the absolute tolerance to use -- the value should be less than relative tolerance.
 * @param [in] rel_tolerance    the relative tolerance to use -- the value should be greater than or equal to epsilon.
 * @return Which returns true if 'a' is definitely greater than 'b' or false otherwise.
 */
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
bool IsDefinitelyGreaterThan(T a, T b, T abs_tolerance, T rel_tolerance = std::numeric_limits<T>::epsilon())
{
    const auto diff = a - b;
    // Check if the numbers are really close -- needed when comparing numbers near zero.
    if(diff <= abs_tolerance)
        return false;
    return diff > (std::max(std::abs(a), std::abs(b)) * rel_tolerance);
}

/*!
 * Whether 'a' is definitely less than 'b'.
 * @param [in] a    the first number to compare.
 * @param [in] b    the second number to compare.
 * @param [in] abs_tolerance    the absolute tolerance to use -- the value should be less than relative tolerance.
 * @param [in] rel_tolerance    the relative tolerance to use -- the value should be greater than or equal to epsilon.
 * @return Which returns true if 'a' is definitely less than 'b' or false otherwise.
 */
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
bool IsDefinitelyLessThan(T a, T b, T abs_tolerance, T rel_tolerance = std::numeric_limits<T>::epsilon())
{
    const auto diff = b - a;
    // Check if the numbers are really close -- needed when comparing numbers near zero.
    if(diff <= abs_tolerance)
        return false;
    return diff > (std::max(std::abs(a), std::abs(b)) * rel_tolerance);
}

/*!
 * Whether 'a' is greater than or approximately equal to 'b'.
 * @param [in] a    the first number to compare.
 * @param [in] b    the second number to compare.
 * @param [in] abs_tolerance    the absolute tolerance to use -- the value should be less than relative tolerance.
 * @param [in] rel_tolerance    the relative tolerance to use -- the value should be greater than or equal to epsilon.
 * @return Which returns true if 'a' is greater than or approximately equal to 'b' or false otherwise.
 */
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
bool IsGreaterThanOrApproximatelyEqualTo(T a, T b, T abs_tolerance, T rel_tolerance = std::numeric_limits<T>::epsilon())
{
    return IsDefinitelyGreaterThan(a, b, abs_tolerance, rel_tolerance) || IsApproximatelyEqualTo(a, b, abs_tolerance, rel_tolerance);
}

/*!
 * Whether 'a' is less than or approximately equal to 'b'.
 * @param [in] a    the first number to compare.
 * @param [in] b    the second number to compare.
 * @param [in] abs_tolerance    the absolute tolerance to use -- the value should be less than relative tolerance.
 * @param [in] rel_tolerance    the relative tolerance to use -- the value should be greater than or equal to epsilon.
 * @return Which returns true if 'a' is less than or approximately equal to 'b' or false otherwise.
 */
template<typename T, typename = typename std::enable_if<std::is_floating_point<T>::value>::type>
bool IsLessThanOrApproximatelyEqualTo(T a, T b, T abs_tolerance, T rel_tolerance = std::numeric_limits<T>::epsilon())
{
    return IsDefinitelyLessThan(a, b, abs_tolerance, rel_tolerance) || IsApproximatelyEqualTo(a, b, abs_tolerance, rel_tolerance);
}

}}