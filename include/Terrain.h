#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <cmath>
#include <GL/gl.h>
#include "MathUtils.h"

class Terrain {
public:
    Terrain(float worldSize = 50.0f, int gridRes = 128);
    ~Terrain();

    // Generate the heightmap with procedural noise
    void generate(unsigned int seed = 42);

    // Query terrain height at any world (x, z) position via bilinear interpolation
    float getHeight(float x, float z) const;

    // Get approximate normal at world (x, z) for lighting
    Vector3 getNormal(float x, float z) const;

    // Render the terrain mesh
    void render(GLuint textureId) const;

    float getWorldSize() const { return worldSize; }

private:
    float worldSize;    // Half-extent: terrain spans [-worldSize, +worldSize]
    int gridRes;        // Number of grid cells per axis
    std::vector<float> heightmap; // (gridRes+1) * (gridRes+1)

    // Convert world coords to grid indices
    void worldToGrid(float wx, float wz, float& gx, float& gz) const;

    // Get height at integer grid coordinates (clamped)
    float heightAt(int ix, int iz) const;

    // Pseudo-random hash for noise generation
    static float hash(int x, int z, int seed);

    // Smooth noise interpolation
    static float smoothNoise(float x, float z, int seed);

    // Multi-octave value noise
    static float valueFBM(float x, float z, int octaves, float persistence, float lacunarity, int seed);
};

#endif // TERRAIN_H
