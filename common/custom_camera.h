#pragma once
#include <hasenpfote/math/vector3.h>
#include <hasenpfote/math/cmatrix4.h>
#include <hasenpfote/math/quaternion.h>
#include "camera.h"

#define ENABLE_MULTI_ASPECT

class CustomCamera final
{
public:
    CustomCamera();
    ~CustomCamera() = default;

// view
    void SetPosition(const hasenpfote::math::Vector3& position) { this->position = position; }
    hasenpfote::math::Vector3 GetPosition() { return position; }
    void SetTargetPosition(const hasenpfote::math::Vector3& target);
    hasenpfote::math::Vector3 GetForward() const;
    hasenpfote::math::Vector3 GetRight() const;
    hasenpfote::math::Vector3 GetUp() const;

    /*!
    * ビュー行列の取得.
    * @return CMatrix4
    */
    hasenpfote::math::CMatrix4 GetViewMatrix() const { return camera.GetViewMatrix(); }

// projection

    /*!
     * 35mm換算焦点距離の設定.
     * @param[in] focal_length 35mm focal length
     */
    void Set35mmEquivalentFocalLength(float focal_length);

    /*!
     * 35mm換算焦点距離の取得.
     * @return 35mm equivalent focal length
     */
    float Get35mmEquivalentFocalLength() const;

    /*!
     * 焦点距離の設定.
     * @return focal length
     */
    void SetFocalLength(float focal_length);

    /*!
     * 焦点距離の取得.
     * @return focal length
     */
    float GetFocalLength() const;

    enum class AngleOfView
    {
        Horizontal,
        Vertical,
        Diagonal
    };

    /*!
     * 画角の取得.
     * @param[in] aov
     * @return angle of view(rad)
     */
    float GetAngleOfView(AngleOfView aov) const;

    /*!
     * ズーム倍率の設定
     * @param[in] mag zoom magnification
     */
    void SetZoomMagnification(float zoom_magnification);

    /*!
     * ズーム倍率の取得.
     * @return zoom magnification
     */
    float GetZoomMagnification() const { return zoom_magnification; }

    /*!
     * 最大ズーム倍率の取得.
     * @return maximum zoom magnification
     */
    float GetMaxZoomMagnification() const { return max_zoom_magnification; }

    /*!
     * クリップ平面の設定.
     * @param[in] near near clipping plane
     * @param[in] far far clipping plane
     */
    void SetClippingPlane(float near, float far);

    /*!
     * 射影行列の取得.
     * @return CMatrix4
     */
    hasenpfote::math::CMatrix4 GetProjectionMatrix() const { return camera.GetProjectionMatrix(); }

// viewport

    /*!
     * ビューポートの設定.
     */
    void SetViewport(int x, int y, int width, int height);

    /*!
     * ビューポートサイズの設定.
     */
    void SetViewportSize(int width, int height);

    /*!
     * ビューポートの取得.
     * @return CMatrix4
     */
    const Viewport& GetViewport() const { return camera.GetViewport(); }

// motion

    void OnKey(int key, int scancode, int action, int mods);
    void OnMouseMove(double xpos, double ypos);
    void OnMouseButton(int button, int action, int mods);
    void OnMouseWheel(double xoffset, double yoffset);

// other

    /*!
     * 更新処理.
     */
    void Update(double dt);

private:

#ifndef ENABLE_MULTI_ASPECT
    /*!
     * 有効センサーサイズを計算する.
     * @param[out] effective_width  有効センサー幅(mm)
     * @param[out] effective_height 有効センサー高(mm)
     * @param[in] reference_width 参照センサー幅(mm)
     * @param[in] image_aspect
     * @note アスペクト比に依ってイメージサークルの直径が変化する.
     */
    static void ComputeEffectiveSensorSize(float& effective_width, float& effective_height, float reference_width, float image_aspect);
#else
    /*!
     * 有効センサーサイズをイメージサークルから計算する.
     * @param[out] effective_width 有効センサー幅(mm)
     * @param[out] effective_height 有効センサー高(mm)
     * @param[in] circle_diameter イメージサークルの直径(mm)
     * @param[in] image_aspect
     * @note アスペクト比に依らずイメージサークルの直径は一定となる(マルチアスペクト)
     */
    static void ComputeEffectiveSensorSizeFromImageCircle(float& effective_width, float& effective_height, float circle_diameter, float image_aspect);
#endif
    /*!
     * クロップ係数を計算する.
     * @param[in] sensor_diagonal センサーの対角(mm)
     * @return crop factor
     */
    static float ComputeCropFactor(float sensor_diagonal);

    /*!
     * 画角から焦点距離を計算する.
     * @param[in] size センサーサイズ(mm)
     * @param[in] aov 画角(rad)
     * @return focal length(mm)
     * @note センサーサイズと画角は水平,垂直,対角で入力.
     */
    static float ComputeFocalLength(float size, float aov);

    /*!
     * 焦点距離から画角を計算する.
     * @param[in] size センサーサイズ(mm)
     * @param[in] focal_length 焦点距離(mm)
     * @return angle of view(rad)
     * センサーサイズと焦点距離は水平,垂直,対角で入力.
     */
    static float ComputeAngleOfView(float size, float focal_length);

    /*!
     * 最大ズーム倍率を計算する.
     * @param[in] tele_end T端焦点距離(mm)
     * @param[in] wide_end W端焦点距離(mm)
     * @return maximum zoom magnification
     */
    static float ComputeMaxZoomMagnification(float tele_end, float wide_end);

    hasenpfote::math::Vector3 ToSemiSphere(float x, float y);

private:
    Camera camera;
    // view

    static const hasenpfote::math::Vector3 FORWARD;
    static const hasenpfote::math::Vector3 RIGHT;
    static const hasenpfote::math::Vector3 UP;

    hasenpfote::math::Vector3 position;
    hasenpfote::math::Quaternion rotation;
    hasenpfote::math::Vector3 velocity;

    // projection
    static const float _35mm_film_diagonal;
    static constexpr float sensor_width = 36.0f;
    static constexpr float sensor_height = 24.0f;

    float effective_sensor_width;
    float effective_sensor_height;
    float effective_sensor_diagonal;
    float crop_factor;

    float focal_length;
    float zoom_magnification;
    float max_zoom_magnification;

    float near;
    float far;

    bool is_dirty_proj;

    // motion

    enum class Motion
    {
        Stop,
        Start,
        Tracking
    } motion;
    hasenpfote::math::Vector3 prev;
    hasenpfote::math::Vector3 current;
};