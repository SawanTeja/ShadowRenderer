#include "ShadowSystem.h"
#include <cmath>
#include <algorithm>

ShadowSystem::ShadowSystem(int resolution, float worldSize)
    : resolution(resolution), worldSize(worldSize) {
    shadowMap.resize(resolution * resolution, 0.0f);
}

ShadowSystem::~ShadowSystem() {
}

// ================================================================
// Main shadow computation
// ================================================================
void ShadowSystem::compute(const Vector3& lightPos,
                            const std::function<float(float, float)>& getHeight,
                            const std::vector<Caster>& casters) {
    float cellSize = (2.0f * worldSize) / (resolution - 1);

    for (int iz = 0; iz < resolution; iz++) {
        for (int ix = 0; ix < resolution; ix++) {
            float wx = -worldSize + ix * cellSize;
            float wz = -worldSize + iz * cellSize;
            float wy = getHeight(wx, wz);

            float shadow = 0.0f;

            // Check terrain self-occlusion (hills blocking sunlight)
            if (isOccludedByTerrain(wx, wy, wz, lightPos, getHeight)) {
                shadow = 1.0f;
            }
            // Check object/tree occlusion
            else if (isOccludedByCaster(wx, wy, wz, lightPos, casters)) {
                shadow = 1.0f;
            }

            shadowMap[iz * resolution + ix] = shadow;
        }
    }
}

// ================================================================
// Terrain self-shadowing: march from vertex toward light,
// check if terrain height exceeds ray height at any point
// ================================================================
bool ShadowSystem::isOccludedByTerrain(float px, float py, float pz,
                                        const Vector3& lightPos,
                                        const std::function<float(float, float)>& getHeight) const {
    float dx = lightPos.x - px;
    float dy = lightPos.y - py;
    float dz = lightPos.z - pz;
    float dist = sqrt(dx * dx + dy * dy + dz * dz);
    if (dist < 0.1f) return false;

    // Normalize direction
    dx /= dist; dy /= dist; dz /= dist;

    // March in fixed steps toward the light
    int numSteps = 25;
    float maxDist = std::min(dist, worldSize * 0.8f);
    float stepSize = maxDist / numSteps;

    for (int i = 1; i < numSteps; i++) {
        float t = i * stepSize;
        float sx = px + dx * t;
        float sy = py + dy * t;
        float sz = pz + dz * t;

        // Stay within terrain bounds
        if (sx < -worldSize || sx > worldSize || sz < -worldSize || sz > worldSize)
            break;

        float terrainH = getHeight(sx, sz);
        if (terrainH > sy + 0.15f) {
            return true; // Terrain blocks the light
        }
    }

    return false;
}

// ================================================================
// Object occlusion: cast ray from terrain point to light,
// check if any AABB shadow caster blocks the ray
// ================================================================
bool ShadowSystem::isOccludedByCaster(float px, float py, float pz,
                                       const Vector3& lightPos,
                                       const std::vector<Caster>& casters) const {
    // Ray from terrain surface toward light
    float origin[3] = { px, py + 0.05f, pz }; // Tiny offset above surface
    float dir[3] = {
        lightPos.x - px,
        lightPos.y - py,
        lightPos.z - pz
    };
    float dist = sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
    if (dist < 0.01f) return false;

    dir[0] /= dist;
    dir[1] /= dist;
    dir[2] /= dist;

    for (const auto& c : casters) {
        float t;
        if (rayIntersectsAABB(origin, dir,
                               c.center.x - c.halfExtents.x,
                               c.center.y - c.halfExtents.y,
                               c.center.z - c.halfExtents.z,
                               c.center.x + c.halfExtents.x,
                               c.center.y + c.halfExtents.y,
                               c.center.z + c.halfExtents.z,
                               t)) {
            if (t > 0.0f && t < dist) {
                return true; // Object blocks the light
            }
        }
    }

    return false;
}

// ================================================================
// Bilinear interpolation of shadow value at any world position
// ================================================================
float ShadowSystem::getShadowAt(float worldX, float worldZ) const {
    float cellSize = (2.0f * worldSize) / (resolution - 1);
    float gx = (worldX + worldSize) / cellSize;
    float gz = (worldZ + worldSize) / cellSize;

    // Clamp
    if (gx < 0.0f) gx = 0.0f;
    if (gx >= resolution - 1) gx = (float)(resolution - 2);
    if (gz < 0.0f) gz = 0.0f;
    if (gz >= resolution - 1) gz = (float)(resolution - 2);

    int ix = (int)floor(gx);
    int iz = (int)floor(gz);
    float fx = gx - ix;
    float fz = gz - iz;

    // Bilinear interpolation of shadow values
    float s00 = shadowMap[iz * resolution + ix];
    float s10 = shadowMap[iz * resolution + ix + 1];
    float s01 = shadowMap[(iz + 1) * resolution + ix];
    float s11 = shadowMap[(iz + 1) * resolution + ix + 1];

    float s0 = s00 + (s10 - s00) * fx;
    float s1 = s01 + (s11 - s01) * fx;

    return s0 + (s1 - s0) * fz;
}
