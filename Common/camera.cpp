#include <hasenpfote/math/utility.h>
#include <hasenpfote/math/vector4.h>
#include "camera.h"

using namespace hasenpfote::math;

void Viewport::Set(int x, int y, int width, int height)
{
    SetPosition(x, y);
    SetSize(width, height);
}

void Viewport::SetPosition(int x, int y)
{
    this->x = x;
    this->y = y;
}

void Viewport::SetSize(int width, int height)
{
    this->width = width;
    this->height = height;
}

float Viewport::GetAspectRatio() const
{
    return width / static_cast<float>(height);
}

Vector3 Camera::ToScreenCoord(const Vector3& pos) const
{
#if 0
    CMatrix4 mvp = proj * view;
    const float w = mvp.m41 * pos.x + mvp.m42 * pos.y + mvp.m43 * pos.z + mvp.m44;
    Vector3 result = (mvp * pos) / w;   // normalized device coordinate.

    result.x = remap(result.x, -1.0f, 1.0f, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()));
    result.y = remap(result.y, -1.0f, 1.0f, static_cast<float>(vp.GetHeight()), static_cast<float>(vp.GetPositionY()));

    return result;
#else
    const CMatrix4 mvp = proj * view;
    const Vector4 clip = mvp * Vector4(pos, 1.0f);
    return Vector3(
        remap(clip.GetX() / clip.GetW(), -1.0f, 1.0f, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth())),
        remap(clip.GetY() / clip.GetW(), -1.0f, 1.0f, static_cast<float>(vp.GetHeight()), static_cast<float>(vp.GetPositionY())),
        clip.GetZ() / clip.GetW());
#endif
}

Vector3 Camera::ToWorldCoord(const Vector3& pos) const
{
#if 0
    const float x = remap(pos.x, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()), -1.0f, 1.0f);
    const float y = remap(pos.y, static_cast<float>(vp.GetPositionY()), static_cast<float>(vp.GetHeight()), 1.0f, -1.0f);
    const float z = pos.z;

    CMatrix4 inv = CMatrix4::Inverse(proj * view);
    const float w = inv.m41 * x + inv.m42 * y + inv.m43 * z + inv.m44;

    return (inv * Vector3(x, y, z)) / w;
#else
    const CMatrix4 inv = CMatrix4::Inverse(proj * view);
    const Vector4 h = inv * Vector4(
        remap(pos.GetX(), static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()), -1.0f, 1.0f),
        remap(pos.GetY(), static_cast<float>(vp.GetPositionY()), static_cast<float>(vp.GetHeight()), 1.0f, -1.0f),
        pos.GetZ(),
        1.0f);
    return Vector3(
        h.GetX() / h.GetW(),
        h.GetY() / h.GetW(),
        h.GetZ() / h.GetW()
    );
#endif
}

Vector3 Camera::GetRay(const Vector3& pos) const
{
#if 0
    const float x = remap(pos.x, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()), -1.0f, 1.0f);
    const float y = remap(pos.y, static_cast<float>(vp.GetPositionY()), static_cast<float>(vp.GetHeight()), 1.0f, -1.0f);

    CMatrix4 inv = CMatrix4::Inverse(proj * view);

    const float fw = inv.m41 * x + inv.m42 * y + inv.m43 + inv.m44;
    Vector3 far = (inv * Vector3(x, y, 1.0f)) / fw;

    const float nw = inv.m41 * x + inv.m42 * y - inv.m43 + inv.m44;
    Vector3 near = (inv * Vector3(x, y, -1.0f)) / nw;

    Vector3 ray = far - near;
    ray.Normalize();

    return ray;
#else
    const float x = remap(pos.GetX(), static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()), -1.0f, 1.0f);
    const float y = remap(pos.GetY(), static_cast<float>(vp.GetPositionY()), static_cast<float>(vp.GetHeight()), 1.0f, -1.0f);

    const CMatrix4 inv = CMatrix4::Inverse(proj * view);

    Vector4 far = inv * Vector4(x, y, 1.0f, 1.0f);
    far /= far.GetW();
    
    Vector4 near = inv * Vector4(x, y, -1.0f, 1.0f);
    near /= near.GetW();

    Vector3 ray(
        far.GetX() - near.GetX(),
        far.GetY() - near.GetY(),
        far.GetZ() - near.GetZ());
    ray.Normalize();

    return ray;
#endif
}

#if 0
Vector3 Camera::Project(const Vector3& pos, const CMatrix4& view, const CMatrix4& proj, const Viewport& vp)
{
    const CMatrix4 mvp = proj * view;

    float x = mvp.m11 * pos.x + mvp.m12 * pos.y + mvp.m13 * pos.z + mvp.m14 * 1.0f;
    float y = mvp.m21 * pos.x + mvp.m22 * pos.y + mvp.m23 * pos.z + mvp.m24 * 1.0f;
    float z = mvp.m31 * pos.x + mvp.m32 * pos.y + mvp.m33 * pos.z + mvp.m34 * 1.0f;
    float w = mvp.m41 * pos.x + mvp.m42 * pos.y + mvp.m43 * pos.z + mvp.m44 * 1.0f;

    x /= w; // [-1, 1]
    y /= w; // [-1, 1]
    z /= w; // [-1, 1]
    w /= w; // 1

    x = remap(x, -1.0f, 1.0f, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()));
    y = remap(y, -1.0f, 1.0f, static_cast<float>(vp.GetHeight()), static_cast<float>(vp.GetPositionY()));

    return Vector3(x, y, z);
}

Vector3 Camera::UnProject(const Vector3& pos, const CMatrix4& view, const CMatrix4& proj, const Viewport& vp)
{
    float x = remap(pos.x, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()), -1.0f, 1.0f);
    float y = remap(pos.y, static_cast<float>(vp.GetPositionY()), static_cast<float>(vp.GetHeight()), 1.0f, -1.0f);
    float z = pos.z;// 1.0f; // [-1, 1] ... ê≥ãKâªç¿ïWånÇ≈ÇÃ near / far clipping plane ëäìñ
    float w = 1.0f; // å≈íË

    CMatrix4 inv = CMatrix4::Inverse(proj * view);

    float _x = inv.m11 * x + inv.m12 * y + inv.m13 * z + inv.m14 * w;
    float _y = inv.m21 * x + inv.m22 * y + inv.m23 * z + inv.m24 * w;
    float _z = inv.m31 * x + inv.m32 * y + inv.m33 * z + inv.m34 * w;
    float _w = inv.m41 * x + inv.m42 * y + inv.m43 * z + inv.m44 * w;

    _w = 1.0f / _w; // [near, far]
    _x *= _w;
    _y *= _w;
    _z *= _w;

    return Vector3(_x, _y, _z);
}

Vector3 Camera::GetRay(const Vector3& pos, const CMatrix4& view, const CMatrix4& proj, const Viewport& vp)
{
    float x = remap(pos.x, static_cast<float>(vp.GetPositionX()), static_cast<float>(vp.GetWidth()), -1.0f, 1.0f);
    float y = remap(pos.y, static_cast<float>(vp.GetPositionY()), static_cast<float>(vp.GetHeight()), 1.0f, -1.0f);

    CMatrix4 inv = CMatrix4::Inverse(proj * view);

    float rcp_w = 1.0f / (inv.m41 * x + inv.m42 * y + inv.m43 + inv.m44);
    Vector3 far = rcp_w * (inv * Vector3(x, y, 1.0f));

    rcp_w = 1.0f / (inv.m41 * x + inv.m42 * y - inv.m43 + inv.m44);
    Vector3 near = rcp_w * (inv * Vector3(x, y, -1.0f));

    Vector3 ray = far - near;
    ray.Normalize();

    return ray;
}
#endif