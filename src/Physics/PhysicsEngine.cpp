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
    for (auto obj : objects) {
        if (obj->isStatic) continue;

        // Apply friction if no acceleration (simple damping for now)
        // Or constantly apply friction against velocity
        
        // Simple Euler Integration
        // v = v + a * dt
        obj->velocity.x += obj->acceleration.x * dt;
        obj->velocity.y += obj->acceleration.y * dt;
        obj->velocity.z += obj->acceleration.z * dt;

        // Apply Friction
        // Friction force opposes velocity: F_f = -mu * v
        // decel = -mu * v
        // We can implement simple damping: v *= (1 - friction * dt)
        if (obj->friction > 0.0f) {
            float damping = 1.0f - (obj->friction * dt);
            if (damping < 0.0f) damping = 0.0f;
            
            // Apply straight damping
            obj->velocity.x *= damping;
            obj->velocity.y = obj->velocity.y * damping; 
            obj->velocity.y *= damping;
            obj->velocity.z *= damping;
            
            // Stop completely if very slow
            if (obj->velocity.length() < 0.01f && obj->acceleration.length() < 0.01f) {
                 obj->velocity = Vector3(0,0,0);
            }
        }

        // x = x + v * dt
        obj->position.x += obj->velocity.x * dt;
        obj->position.y += obj->velocity.y * dt;
        obj->position.z += obj->velocity.z * dt;
        
    }
    
    checkCollisions(dt);
}

void PhysicsEngine::checkCollisions(float dt) {
    // 1. Floor Collision
    for (auto obj : objects) {
        if (obj->position.y - obj->size.y < 0.0f) {
            obj->position.y = obj->size.y;
            obj->velocity.y = -obj->velocity.y * 0.5f; // Bounce with damping
            
            // Apply friction on floor
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
