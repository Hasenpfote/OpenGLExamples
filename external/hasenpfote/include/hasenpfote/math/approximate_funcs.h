/*!
 * @file approximate_funcs.h
 * @brief Approximate functions.
 * @author Hasenpfote
 */
#pragma once

namespace hasenpfote{ namespace math{

/*!
 * sinc 関数のテイラー級数による近似.
 * <p>
 * sinc(x)<br>
 * \f$\frac{\sin{x}}{x} = 1 - \frac{1}{6}x^{2} + \frac{1}{120}x^{4} - \frac{1}{5040}x^{6}\f$
 * </p>
 * @param[in] x
 * @return
 */
float sinc(float x);

/*!
 * sinc 関数の逆数のテイラー級数による近似.
 * <p>
 * reciprocal of sinc(x)<br>
 * \f$\frac{x}{\sin{x}} = 1 + \frac{1}{6}x^{2} + \frac{7}{360}x^{4} + \frac{31}{15120}x^{6} + ･･･ \f$
 * </p>
 * @param x
 * @return
 */
float rcp_sinc(float x);

}}