#ifndef SHADOW_SYSTEM_H
#define SHADOW_SYSTEM_H

#include "MathUtils.h"
#include <vector>
#include <functional>

class ShadowSystem {
public:
    // Shadow caster represented as an AABB
    struct Caster {
        Vector3 center;
        Vector3 halfExtents;
    };

    ShadowSystem(int resolution = 64, float worldSize = 50.0f);
    ~ShadowSystem();

    // Compute shadow map for the entire terrain
    // lightPos: world position of the light source
    // getHeight: function to query terrain height at (x, z)
    // casters: list of shadow-casting AABBs (shapes, trees, etc.)
    void compute(const Vector3& lightPos,
                 const std::function<float(float, float)>& getHeight,
                 const std::vector<Caster>& casters);

    // Get interpolated shadow value at any world position
    // Returns 0.0 = fully lit, 1.0 = fully shadowed
    float getShadowAt(float worldX, float worldZ) const;

    int getResolution() const { return resolution; }
    float getWorldSize() const { return worldSize; }

private:
    int resolution;
    float worldSize;
    std::vector<float> shadowMap; // resolution * resolution

    // Check if terrain itself blocks the light ray (hills casting shadows on valleys)
    bool isOccludedByTerrain(float px, float py, float pz,
                              const Vector3& lightPos,
                              const std::function<float(float, float)>& getHeight) const;

    // Check if any AABB caster blocks the light ray
    bool isOccludedByCaster(float px, float py, float pz,
                             const Vector3& lightPos,
                             const std::vector<Caster>& casters) const;
};

#endif // SHADOW_SYSTEM_H
