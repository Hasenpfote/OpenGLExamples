#include <hasenpfote/math/utils.h>
#include "arcball.h"

using namespace hasenpfote::math;

ArcBall::ArcBall()
{
    event = Event::Stop;
    rotation = Quaternion::IDENTITY;
}

void ArcBall::SetWindowSize(int width, int height)
{
    this->width = width;
    this->height = height;
}

void ArcBall::Start()
{
    rotation = Quaternion::IDENTITY;
    event = Event::Start;
}

void ArcBall::Stop()
{
    rotation = Quaternion::IDENTITY;
    event = Event::Stop;
}

void ArcBall::Update(float x, float y)
{
    if(event == Event::Start){
        prev = ToScreenCoord(x, y);
        event = Event::Tracking;
    }
    else
    if(event == Event::Tracking){
        current = ToScreenCoord(x, y);
        if(Vector3::DotProduct(prev, current) < 1.0f){
            rotation = Quaternion::RotationShortestArc(prev, current);
        }
        else{
            rotation = Quaternion::IDENTITY;
        }
        prev = current;
    }
}

Vector3 ArcBall::ToScreenCoord(float x, float y)
{
    Vector3 result;
    result.SetX(Remap(x, 0.0f, static_cast<float>(width), -1.0f, 1.0f));
    result.SetY(Remap(y, 0.0f, static_cast<float>(height), 1.0f, -1.0f));

    // needs clip

    const float d = result.GetX() * result.GetX() + result.GetY() * result.GetY();
    if(d <= 1.0f){
        result.SetZ(std::sqrtf(1.0f - d));
    }
    else{
        result.SetZ(0.0f);
        result.Normalize();
    }
    return result;
}