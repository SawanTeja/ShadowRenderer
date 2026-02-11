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
}
