#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include <vector>
#include <map>
#include "MathUtils.h"

struct PhysicsObject {
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    float mass;
    float friction; // 0.0 to 1.0, where 1.0 is high friction
    bool isStatic;
    Vector3 size; // Half-extents for AABB (x, y, z)

    PhysicsObject() 
        : position(0,0,0), velocity(0,0,0), acceleration(0,0,0), 
          mass(1.0f), friction(5.0f), isStatic(false), size(0.5f, 0.5f, 0.5f) {}
};

class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();

    void update(float dt);
    
    // Register an object to be simulated
    // Returns a pointer to the internal physics object so the Scene can sync with it
    PhysicsObject* addObject(const Vector3& initialPos);
    
    // Remove object
    void removeObject(PhysicsObject* obj);

private:
    std::vector<PhysicsObject*> objects;
    
    void checkCollisions(float dt);
    void resolveCollision(PhysicsObject* a, PhysicsObject* b);
};

#endif // PHYSICS_ENGINE_H
