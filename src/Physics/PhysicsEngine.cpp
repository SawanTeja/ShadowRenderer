#include "Physics/PhysicsEngine.h"
#include <algorithm>
#include <iostream>

PhysicsEngine::PhysicsEngine() {
}

PhysicsEngine::~PhysicsEngine() {
    for (auto obj : objects) {
        delete obj;
    }
    objects.clear();
}

PhysicsObject* PhysicsEngine::addObject(const Vector3& initialPos) {
    PhysicsObject* obj = new PhysicsObject();
    obj->position = initialPos;
    objects.push_back(obj);
    return obj;
}

void PhysicsEngine::removeObject(PhysicsObject* obj) {
    auto it = std::find(objects.begin(), objects.end(), obj);
    if (it != objects.end()) {
        delete *it;
        objects.erase(it);
    }
}

void PhysicsEngine::update(float dt) {
    const float GRAVITY = -15.0f; // Gravity acceleration (units/s^2)

    for (auto obj : objects) {
        if (obj->isStatic) continue;

        // Apply gravity
        obj->velocity.y += GRAVITY * dt;

        // Apply user acceleration (WASD input)
        obj->velocity.x += obj->acceleration.x * dt;
        obj->velocity.y += obj->acceleration.y * dt;
        obj->velocity.z += obj->acceleration.z * dt;

        // Apply horizontal friction (only XZ, not Y — gravity handles vertical)
        if (obj->friction > 0.0f) {
            float damping = 1.0f - (obj->friction * dt);
            if (damping < 0.0f) damping = 0.0f;
            
            obj->velocity.x *= damping;
            obj->velocity.z *= damping;
            
            // Stop horizontal movement if very slow
            float horizSpeed = sqrt(obj->velocity.x * obj->velocity.x + obj->velocity.z * obj->velocity.z);
            float horizAccel = sqrt(obj->acceleration.x * obj->acceleration.x + obj->acceleration.z * obj->acceleration.z);
            if (horizSpeed < 0.01f && horizAccel < 0.01f) {
                 obj->velocity.x = 0.0f;
                 obj->velocity.z = 0.0f;
            }
        }

        // Position integration: x = x + v * dt
        obj->position.x += obj->velocity.x * dt;
        obj->position.y += obj->velocity.y * dt;
        obj->position.z += obj->velocity.z * dt;
        
    }
    
    checkCollisions(dt);
}

void PhysicsEngine::checkCollisions(float dt) {
    // 1. Terrain/Floor Collision
    for (auto obj : objects) {
        // Determine ground height at object's XZ position
        float groundY = 0.0f;
        if (getTerrainHeight) {
            groundY = getTerrainHeight(obj->position.x, obj->position.z);
        }

        if (obj->position.y - obj->size.y < groundY) {
            obj->position.y = groundY + obj->size.y;
            
            // Bounce with damping, but stop if velocity is tiny
            float bounceVel = -obj->velocity.y * 0.3f;
            if (std::abs(bounceVel) < 0.5f) {
                obj->velocity.y = 0.0f; // Stop bouncing
            } else {
                obj->velocity.y = bounceVel;
            }
            
            // Apply friction on ground contact
            obj->velocity.x *= 0.9f;
            obj->velocity.z *= 0.9f;
        }
    }

    // 2. Object vs Object Collision
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            PhysicsObject* a = objects[i];
            PhysicsObject* b = objects[j];
            
            if (a->isStatic && b->isStatic) continue;

            // AABB Collision Detection
            if (std::abs(a->position.x - b->position.x) < (a->size.x + b->size.x) &&
                std::abs(a->position.y - b->position.y) < (a->size.y + b->size.y) &&
                std::abs(a->position.z - b->position.z) < (a->size.z + b->size.z)) {
                
                resolveCollision(a, b);
            }
        }
    }
}

void PhysicsEngine::resolveCollision(PhysicsObject* a, PhysicsObject* b) {
    // Calculate overlap on each axis
    float overlapX = (a->size.x + b->size.x) - std::abs(a->position.x - b->position.x);
    float overlapY = (a->size.y + b->size.y) - std::abs(a->position.y - b->position.y);
    float overlapZ = (a->size.z + b->size.z) - std::abs(a->position.z - b->position.z);

    // Find the smallest overlap (shallowest penetration) to resolve
    // Find the smallest overlap (shallowest penetration) to resolve
    
    // If 'b' is static, 'a' moves full overlap
    if (b->isStatic) {
        if (overlapX < overlapY && overlapX < overlapZ) {
            a->position.x += (a->position.x < b->position.x) ? -overlapX : overlapX;
            a->velocity.x = 0.0f; // Slide/Stop, don't bounce
        } else if (overlapY < overlapX && overlapY < overlapZ) {
            a->position.y += (a->position.y < b->position.y) ? -overlapY : overlapY;
            a->velocity.y = 0.0f;
        } else {
            a->position.z += (a->position.z < b->position.z) ? -overlapZ : overlapZ;
            a->velocity.z = 0.0f;
        }
        return;
    }
    
    // If 'a' is static (shouldn't happen with current loop order if we check effectively, 
    // but useful for completeness), 'b' moves full overlap
    if (a->isStatic) {
        if (overlapX < overlapY && overlapX < overlapZ) {
            b->position.x += (b->position.x < a->position.x) ? overlapX : -overlapX; // Push b away
            b->velocity.x = 0.0f;
        } else if (overlapY < overlapX && overlapY < overlapZ) {
            b->position.y += (b->position.y < a->position.y) ? overlapY : -overlapY;
            b->velocity.y = 0.0f;
        } else {
            b->position.z += (b->position.z < a->position.z) ? overlapZ : -overlapZ;
            b->velocity.z = 0.0f;
        }
        return;
    }

    // Both dynamic
    if (overlapX < overlapY && overlapX < overlapZ) {
        // Resolve on X axis
        if (a->position.x < b->position.x) {
            a->position.x -= overlapX * 0.5f;
            b->position.x += overlapX * 0.5f;
        } else {
            a->position.x += overlapX * 0.5f;
            b->position.x -= overlapX * 0.5f;
        }
        // Swap/Reflect velocities
        float temp = a->velocity.x;
        a->velocity.x = b->velocity.x;
        b->velocity.x = temp;
    } else if (overlapY < overlapX && overlapY < overlapZ) {
        // Resolve on Y axis
        if (a->position.y < b->position.y) {
            a->position.y -= overlapY * 0.5f;
            b->position.y += overlapY * 0.5f;
        } else {
            a->position.y += overlapY * 0.5f;
            b->position.y -= overlapY * 0.5f;
        }
        // Swap/Reflect
        float temp = a->velocity.y;
        a->velocity.y = b->velocity.y;
        b->velocity.y = temp;
    } else {
        // Resolve on Z axis
        if (a->position.z < b->position.z) {
            a->position.z -= overlapZ * 0.5f;
            b->position.z += overlapZ * 0.5f;
        } else {
            a->position.z += overlapZ * 0.5f;
            b->position.z -= overlapZ * 0.5f;
        }
        // Swap/Reflect
        float temp = a->velocity.z;
        a->velocity.z = b->velocity.z;
        b->velocity.z = temp;
    }
}
