#pragma once
#include <iostream>
#include <hasenpfote/math/vector3.h>
#include <hasenpfote/math/quaternion.h>

namespace common::render
{

class ArcBall final
{
public:
    ArcBall();
    ~ArcBall() = default;

    ArcBall(const ArcBall&) = delete;
    ArcBall& operator = (const ArcBall&) = delete;
    ArcBall(ArcBall&&) = delete;
    ArcBall& operator = (ArcBall&&) = delete;

    void SetWindowSize(int width, int height);
    void Start();
    void Stop();
    void Update(float x, float y);
    hasenpfote::math::Quaternion GetRotationQuaternion() { return rotation; }

private:
    hasenpfote::math::Vector3 ToScreenCoord(float x, float y);

private:
    enum class Event
    {
        Stop,
        Start,
        Tracking
    };

    int width;
    int height;
    Event event;

    hasenpfote::math::Vector3 prev;
    hasenpfote::math::Vector3 current;
    hasenpfote::math::Quaternion rotation;
};

}   // namespace common::render