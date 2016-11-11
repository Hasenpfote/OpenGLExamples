/*!
* @file quantization.h
* @brief quantization helper.
* @author Hasenpfote
* @date 2016/04/23
*/
#pragma once
#include <cstdint>
#include <cmath>

namespace hasenpfote{ namespace math{
class Quaternion;
class Vector3;
}}

namespace hasenpfote{ namespace quantization{

// N-bit unsigned normalized value([0,1]) encoder.
template <unsigned N> std::uint16_t encode_unorm(float x)
{
    static_assert(((N) > 0) && ((N) <= 16), "Out of range.");
    return static_cast<std::uint16_t>(x * ((1 << (N)) - 1) + 0.5f);
}

// N-bit unsigned normalized value([0,1]) decoder.
template <unsigned N> float decode_unorm(std::uint16_t x)
{
    static_assert(((N) > 0) && ((N) <= 16), "Out of range.");
    return x / static_cast<float>((1 << (N)) - 1);
}

// N-bit signed normalized value([-1,+1]) encoder.
template <unsigned N> std::uint16_t encode_snorm(float x)
{
    static_assert(((N) > 1) && ((N) <= 16), "Out of range.");
    return static_cast<std::uint16_t>((x < 0) | (encode_unorm<(N) - 1>(std::abs(x)) << 1));
}

// N-bit signed normalized value([-1,+1]) decoder.
template <unsigned N> float decode_snorm(std::uint16_t x)
{
    static_assert(((N) > 1) && ((N) <= 16), "Out of range.");
    return decode_unorm<(N)- 1>(static_cast<std::uint16_t>(x >> 1)) * ((x & 0x1) ? -1.0f : 1.0f);
}

/*!
 * 回転を表す四元数を量子化する.
 * <pre>
 * [30-31]bits: specifying the max component among X, Y, Z, W.<br>
 * [20-29]bits: component1. \f$[-\frac{1}{\sqrt{2}}, +\frac{1}{\sqrt{2}}]\f$<br>
 * [10-19]bits: component2. \f$[-\frac{1}{\sqrt{2}}, +\frac{1}{\sqrt{2}}]\f$<br>
 * [ 0- 9]bits: component3. \f$[-\frac{1}{\sqrt{2}}, +\frac{1}{\sqrt{2}}]\f$
 * </pre>
 * @param[in] q
 * @return 量子化された四元数
 */
std::uint32_t encode32_quat(const math::Quaternion& q);

/*!
 * 回転を表す四元数を逆量子化する.
 * @param[in] q
 * @return 逆量子化された四元数
 * @see encode32_quat()
 */
math::Quaternion decode32_quat(std::uint32_t q);

/*!
 * 3 次元ベクトルを 16:16:16 に量子化する.
 * @param[in] v
 * @return 量子化された 3 次元ベクトル
 */
std::uint64_t encode161616_vec(const math::Vector3& v);

/*!
 * 3 次元ベクトルを逆量子化する.
 * @param[in] v
 * @return 逆量子化された 3 次元ベクトル
 * @see encode161616_vec()
 */
math::Vector3 decode161616_vec(std::uint64_t v);

}}