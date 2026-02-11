#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <cmath>
#include <GL/gl.h>
#include "MathUtils.h"

// Forward declaration
class ShadowSystem;

class Terrain {
public:
    Terrain(float worldSize = 50.0f, int gridRes = 128);
    ~Terrain();

    // Validates terrain generation (flat)
    void generate(unsigned int seed = 42);

    // Query terrain height at any world (x, z) position (always returns 0)
    float getHeight(float x, float z) const;

    // Get approximate normal at world (x, z) for lighting (always up)
    Vector3 getNormal(float x, float z) const;

    // Render the terrain mesh
    void render(GLuint textureId) const;

    float getWorldSize() const { return worldSize; }

private:
    float worldSize;    // Half-extent: terrain spans [-worldSize, +worldSize]
    int gridRes;        // Number of grid cells per axis
    std::vector<float> heightmap; // (gridRes+1) * (gridRes+1)
};

#endif // TERRAIN_H
