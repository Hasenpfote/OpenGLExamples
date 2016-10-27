/*!
* @file complex_number.h
* @brief Complex number class.
* @author Hasenpfote
* @date 2016/04/19
*/
#pragma once
#include <ostream>

namespace hasenpfote{ namespace math{

class ComplexNumber final
{
public:
/* Constructor */

    ComplexNumber() = default;
    ComplexNumber(const ComplexNumber& c);
    ComplexNumber(float re, float im);

/* Destructor */

    ~ComplexNumber() = default;

/* Setter, Getter */

    inline void SetRealPart(float re){ this->re = re; }
    inline void SetImaginaryPart(float im){ this->im = im; }

    inline float GetRealPart() const { return re; }
    inline float GetImaginaryPart() const { return im; }

/* Assignment operator */

    ComplexNumber& operator = (const ComplexNumber& c);
    ComplexNumber& operator += (const ComplexNumber& c);
    ComplexNumber& operator -= (const ComplexNumber& c);
    ComplexNumber& operator *= (const ComplexNumber& c);
    ComplexNumber& operator *= (float scale);
    ComplexNumber& operator /= (float divisor);

/* Operation */

    /*!
     * ノルムの二乗を計算する.
     * @return ノルムの二乗
     */
    float NormSquared() const;

    /*!
     * ノルムを計算する.
     * @return ノルム
     */
    float Norm() const;

    /*!
     * 偏角を計算する.
     * @return 偏角
     */
    float Argument() const;

    /*!
     * 正規化する.
     */
    void Normalize();

    /*!
     * 正規化したコピーを返す.
     * @return ComplexNumber
     */
    ComplexNumber Normalized() const;

/* Static */

    /*!
     * 極座標形式で生成.
     * @prama[in] rho
     * @prama[in] theta
     * @return ComplexNumber
     */
    static ComplexNumber Polar(float rho, float theta);

    /*!
     * 積の逆元を計算する.
     * @prama[in] c
     * @return ComplexNumber
     */
    static ComplexNumber Inverse(const ComplexNumber& c);

    /*!
     * 共役複素数を計算する.
     * @prama[in] c
     * @return ComplexNumber
     */
    static ComplexNumber Conjugate(const ComplexNumber& c);

    /*!
     * 累乗した複素数を計算する.
     * @prama[in] c
     * @prama[in] exponent
     * @return ComplexNumber
     */
    static ComplexNumber Pow(const ComplexNumber& c, float exponent);

    /*!
     * 自然対数を計算する.
     * @prama[in] c
     * @return ComplexNumber
     */
    static ComplexNumber Ln(const ComplexNumber& c);

    /*!
     * 自然対数の底 e の累乗を計算する.
     * @prama[in] c
     * @return ComplexNumber
     */
    static ComplexNumber Exp(const ComplexNumber& c);

    /*!
     * 回転を表す複素数を生成.
     * @prama[in] angle
     * @return ComplexNumber
     */
    static ComplexNumber Rotation(float angle);

public:
    static const ComplexNumber IDENTITY;

private:
    float re;
    float im;
};

/* Unary operator */
ComplexNumber operator + (const ComplexNumber& c);
ComplexNumber operator - (const ComplexNumber& c);

/* Binary operator */
ComplexNumber operator + (const ComplexNumber& lhs, const ComplexNumber& rhs);
ComplexNumber operator - (const ComplexNumber& lhs, const ComplexNumber& rhs);
ComplexNumber operator * (const ComplexNumber& lhs, const ComplexNumber& rhs);
ComplexNumber operator * (const ComplexNumber& c, float scale);
ComplexNumber operator * (float scale, const ComplexNumber& c);
ComplexNumber operator / (const ComplexNumber& c, float divisor);

/* Stream out */
std::ostream& operator << (std::ostream& os, const ComplexNumber& c);

/* Inline */

}}