/*!
* @file quaternion.h
* @brief Quaternion class.
* @author Hasenpfote
* @date 2016/04/19
*/
#pragma once
#include <array>

namespace hasenpfote{ namespace math{

class Vector3;
class AxisAngle;
class CMatrix4;
class RMatrix4;

class Quaternion final
{
public:
    using Array = std::array<float, 4>;

public:
/* Constructor */

    Quaternion() = default;
    Quaternion(const Quaternion& q);
    Quaternion(float w, float x, float y, float z);
    Quaternion(float s, const Vector3& v);
    explicit Quaternion(const Array& q);
    explicit Quaternion(const AxisAngle& a);

/* Destructor */

    ~Quaternion() = default;

/* Setter, Getter */

    inline void SetW(float w){ this->w = w; }
    inline void SetX(float x){ this->x = x; }
    inline void SetY(float y){ this->y = y; }
    inline void SetZ(float z){ this->z = z; }

    inline float GetW() const { return w; }
    inline float GetX() const { return x; }
    inline float GetY() const { return y; }
    inline float GetZ() const { return z; }

/* Casting operator */

    inline explicit operator float* (){ return q.data(); }
    inline explicit operator const float* () const { return q.data(); }

/* Assignment operator */

    Quaternion& operator = (const Quaternion& q);
    Quaternion& operator = (const Array& q);
    Quaternion& operator += (const Quaternion& q);
    Quaternion& operator -= (const Quaternion& q);
    Quaternion& operator *= (const Quaternion& q);
    Quaternion& operator *= (float scale);
    Quaternion& operator /= (float divisor);

/* Operation */

    /*!
     * ノルムの二乗を計算する.
     * <p>\f$\|q\|^{2} = q^{*}\otimes q\f$</p>
     * @return ノルムの二乗.
     */
    float NormSquared() const;

    /*!
     * ノルムを計算する.
     * <p>\f$\|q\|\f$</p>
     * @return ノルム.
     */
    float Norm() const;

    /*!
     * ベクトル部のノルムを計算する.
     * <p>\f$\|q_{v}\|\f$</p>
     * @return ベクトル部のノルム.
     */
    float NormV() const;

    /*!
     * 正規化する.
     */
    void Normalize();

    /*!
     * 正規化したコピーを返す.
     * @return Quaternion
     */
    Quaternion Normalized() const;

    /*!
     * 回転を表す四元数から回転行列へ変換する.
     * @return CMatrix4
     */
    CMatrix4 ToRotationCMatrix() const;

    /*!
     * 回転を表す四元数から回転行列へ変換する.
     * @return RMatrix4
     */
    RMatrix4 ToRotationRMatrix() const;

    /*!
     * 回転軸と角度に変換する.
     * @return AxisAngle
     */
    AxisAngle ToAxisAngle() const;

    /*!
     * ベクトルを回転する.
     * @param[in] v
     * @return Vector3
     */
    Vector3 Rotate(const Vector3& v) const;

/* Static */

    /*!
     * 積の逆元を計算する.
     * <p>\f$  q^{-1} = \frac{q^{*}}{\|q\|^{2}}\f$</p>
     * @param[in] q
     * @return Quaternion
     */
    static Quaternion Inverse(const Quaternion& q);

    /*!
     * 共役四元数を計算する.
     * <p>\f$q^{*} = [s,-v]\f$</p>
     * @param[in] q
     * @return Quaternion
     */
    static Quaternion Conjugate(const Quaternion& q);

    /*!
     * 2 つの四元数の内積を計算する.
     * @param[in] a
     * @param[in] b
     * @return 内積.
     */
    static float DotProduct(const Quaternion& a, const Quaternion& b);

    /*!
     * 一般四元数の自然対数を計算する.
     * <p>\f$\ln{q}\f$</p>
     * @param[in] q
     * @return Quaternion
     */
    static Quaternion Ln(const Quaternion& q);

    /*!
     * 単位四元数の自然対数を計算する.
     * <p>\f$\ln{q}\f$</p>
     * @param[in] q an unit quaternion.
     * @return Quaternion
     */
    static Quaternion LnU(const Quaternion& q);

    /*!
     * 一般四元数の自然対数の底 e の累乗.
     * <p>\f$e^{q}\f$</p>
     * @param[in] q
     * @return Quaternion
     */
    static Quaternion Exp(const Quaternion& q);

    /*!
     * 純虚四元数の自然対数の底 e の累乗.
     * <p>\f$e^{q}\f$</p>
     * @param[in] q a purely imaginary quaternion.
     * @return Quaternion
     */
    static Quaternion ExpP(const Quaternion& q);

    /*! 
     * 一般四元数の累乗を計算する.
     * <p>\f$base^{exponent}\f$</p>
     * @param[in] q
     * @param[in] exponent
     * @return Quaternion
     */
    static Quaternion Pow(const Quaternion& q, float exponent);

    /*!
     * 単位四元数の累乗を計算する.
     * <p>\f$base^{exponent}\f$</p>
     * @param[in] q an unit quaternion.
     * @param[in] exponent
     * @return Quaternion
     */
    static Quaternion PowU(const Quaternion& q, float exponent);

    /*! 
     * 任意軸周りの回転を表す四元数を生成.
     * @param axis an unit vector.
     * @param angle an angle in radians.
     * @return Quaternion
     */
    static Quaternion RotationAxis(const Vector3& axis, float angle);

    /*!
     * 任意軸周りの回転を表す四元数を生成.
     * @param a axis angle.
     * @return Quaternion
     */
    static Quaternion RotationAxis(const AxisAngle& a);

    /*!
     * 2 つのベクトル間の最小弧回転を表す四元数を生成.
     * @param a an unit vector.
     * @param b an unit vector.
     * @return Quaternion
     */
    static Quaternion RotationShortestArc(const Vector3& a, const Vector3& b);

    /*!
     * a と b の間の差分を計算する.
     * <p>\f$a\otimes diff = b\f$</p>
     * @param a an unit quaternion.
     * @param b an unit quaternion.
     * @return Quaternion
     */
    static Quaternion RotationalDifference(const Quaternion& a, const Quaternion& b);

    /*!
     * 2 つの四元数間を線形補間する.
     * @param[in] a
     * @param[in] b
     * @param[in] t [0,1] の補間パラメータ.
     * @return Quaternion
     */
    static Quaternion Lerp(const Quaternion& a, const Quaternion& b, float t);

    /*!
     * 2 つの四元数間を球面線形補間する.
     * @param[in] a an unit quaternion.
     * @param[in] b an unit quaternion.
     * @param[in] t [0,1]
     * @param[in] allowFlip 最小弧の補間を行い不要なスピンを低減する
     *            true:  360度周期で補間を行う(最大回転変化量は 180度)
     *            false: 720度周期で補間を行う(最大回転変化量は 360度)
     * @return Quaternion
     */
    static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t, bool allowFlip = true);

    /*!
     * p と q 間を球面三次補間する.
     * <p>\f$S_{n}(t) = squad(t; q_{n}, q_{n+1}, a_{n}, b_{n+1})\f$</p>
     * @param p \f$q_{n}\f$
     * @param q \f$q_{n+1}\f$
     * @param a \f$a_{n}\f$
     * @param b \f$b_{n+1}\f$
     * @param t \f$[0,1]\f$
     * @return Quaternion
     */
    static Quaternion Squad(const Quaternion& p, const Quaternion& q, const Quaternion& a, const Quaternion& b, float t);

    /*!
     * スプライン補間で利用する制御点を計算する.
     * <p>\f$a_{n} = b_{n} = q_{n}\otimes e^{-\frac{ln{(q_{n}^{-1}\otimes q_{n-1})} + ln{(q_{n}^{-1}\otimes q_{n+1})}}{4}}\f$</p>
     * <pre>{@code
     *      a = spline(q0, q1, q2);
     *      b = spline(q1, q2, q3);
     *      q = squad(q1, q2, a, b, t);
     * }</pre>
     * @param prev      \f$q_{n-1}\f$
     * @param current   \f$q_{n}\f$
     * @param next      \f$q_{n+1}\f$
     * @return Quaternion
     */
    static Quaternion Spline(const Quaternion& prev, const Quaternion& current, const Quaternion& next);

public:
    enum class Component : std::uint32_t
    {
        W = 0,
        X = 1,
        Y = 2,
        Z = 3
    };
    static const Quaternion IDENTITY;

private:
    union
    {
        struct
        {
            float w;
            float x;
            float y;
            float z;
        };
        Array q;
    };
};

/* Unary operator */
Quaternion operator + (const Quaternion& q);
Quaternion operator - (const Quaternion& q);

/* Binary operator */
Quaternion operator + (const Quaternion& lhs, const Quaternion& rhs);
Quaternion operator - (const Quaternion& lhs, const Quaternion& rhs);
Quaternion operator * (const Quaternion& lhs, const Quaternion& rhs);
Quaternion operator * (const Quaternion& q, float scale);
Quaternion operator * (float scale, const Quaternion& q);
Quaternion operator / (const Quaternion& q, float divisor);

/* Stream out */
std::ostream& operator << (std::ostream& os, const Quaternion& q);

/* Inline */

}}