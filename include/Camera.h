#ifndef CAMERA_H
#define CAMERA_H

#include "MathUtils.h"

class Camera {
public:
    Camera();
    ~Camera();

    // Orbital controls
    void rotate(float dYaw, float dPitch);
    void zoom(float delta);
    
    // Target control
    void setTarget(const Vector3& targetPos);
    Vector3 getTarget() const;

    // View implementation
    void applyLookAt(); // Calls glMultMatrix/lookAt
    
    // Getters for absolute calculations
    void getPosition(float& x, float& y, float& z) const;
    
    // Params
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getDistance() const { return distance; }

private:
    Vector3 target;
    float yaw;
    float pitch;
    float distance;
};

#endif // CAMERA_H
