#include "Camera.h"
#include <cmath>
#include <iostream>

Camera::Camera() : target(0, 0, 0), yaw(0.0f), pitch(0.5f), distance(10.0f) {
}

Camera::~Camera() {
}

void Camera::rotate(float dYaw, float dPitch) {
    yaw += dYaw;
    pitch += dPitch;
    
    // Clamp pitch to avoid gimbal lock/flipping
    if (pitch < 0.1f) pitch = 0.1f;
    if (pitch > 1.5f) pitch = 1.5f;
}

void Camera::zoom(float delta) {
    distance -= delta;
    if (distance < 2.0f) distance = 2.0f;
    if (distance > 50.0f) distance = 50.0f;
}

void Camera::setTarget(const Vector3& targetPos) {
    target = targetPos;
}

Vector3 Camera::getTarget() const {
    return target;
}

void Camera::getPosition(float& x, float& y, float& z) const {
    x = target.x + distance * cos(pitch) * sin(yaw);
    y = target.y + distance * sin(pitch);
    z = target.z + distance * cos(pitch) * cos(yaw);
}

void Camera::applyLookAt() {
    float x, y, z;
    getPosition(x, y, z);
    
    // Up vector is always Y because of orbital constraints
    MathGL::lookAt(x, y, z, 
                   target.x, target.y, target.z,
                   0.0f, 1.0f, 0.0f);
}
