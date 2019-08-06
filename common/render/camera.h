#pragma once
#include <hasenpfote/math/vector3.h>
#include <hasenpfote/math/quaternion.h>
#include <hasenpfote/math/cmatrix4.h>

namespace common::render
{

class Viewport final
{
public:
    Viewport() = default;
    ~Viewport() = default;

    void Set(int x, int y, int width, int height);
    void SetPosition(int x, int y);
    void SetSize(int width, int height);
    int GetPositionX() const { return x; }
    int GetPositionY() const { return y; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    float GetAspectRatio() const;

private:
    int x;
    int y;
    int width;
    int height;
};

class Camera
{
public:
    Camera() = default;
    virtual ~Camera() = default;

    void SetViewMatrix(const hasenpfote::math::CMatrix4 view) { this->view = view; }
    hasenpfote::math::CMatrix4 GetViewMatrix() const { return view; }

    void SetProjectionMatrix(const hasenpfote::math::CMatrix4 proj) { this->proj = proj; }
    hasenpfote::math::CMatrix4 GetProjectionMatrix() const { return proj; }

    Viewport& GetViewport() { return vp; }
    const Viewport& GetViewport() const { return vp; }

    /*!
     * ワールド座標からスクリーン座標へ変換.
     * @param[in] world coord.
     * @return screen coord.
     */
    hasenpfote::math::Vector3 ToScreenCoord(const hasenpfote::math::Vector3& pos) const;

    /*!
     * スクリーン座標からワールド座標へ変換.
     * @param[in] screen coord.
     * @return world coord.
     */
    hasenpfote::math::Vector3 ToWorldCoord(const hasenpfote::math::Vector3& pos) const;

    /*!
     * スクリーン座標からワールド座標上の光線を得る.
     * @param[int] screen coord.
     * @return world coord.
     */
    hasenpfote::math::Vector3 GetRay(const hasenpfote::math::Vector3& pos) const;

private:
    hasenpfote::math::CMatrix4 view;
    hasenpfote::math::CMatrix4 proj;
    Viewport vp;
};

}   // namespace common::render