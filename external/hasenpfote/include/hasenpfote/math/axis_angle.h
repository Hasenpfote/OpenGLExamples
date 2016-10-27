/*!
* @file axis_angle.h
* @brief Axis-angle class.
* @author Hasenpfote
* @date 2016/04/20
*/
#pragma once
#include "vector3.h"

namespace hasenpfote{ namespace math{

class AxisAngle final
{
public:
/* Constructor */

    AxisAngle() = default;
    AxisAngle(const AxisAngle& a);
    AxisAngle(const Vector3& axis, float angle);

/* Destructor */

    ~AxisAngle() = default;

/* Setter, Getter */

    inline void SetAxis(const Vector3& axis){ this->axis = axis; }
    inline void SetAngle(float angle){ this->angle = angle; }

    inline Vector3& GetAxis(){ return axis; }
    inline const Vector3& GetAxis() const { return axis; }
    inline float GetAngle() const { return angle; }

/* Assignment operator */

    AxisAngle& operator = (const AxisAngle& a);

private:
    Vector3 axis;   //!< an unit vector.
    float angle;    //!< an angle in radians.
};

/* Stream out */
std::ostream& operator << (std::ostream& os, const AxisAngle& a);

/* Inline */

}}