#include "Terrain.h"
#include "ShadowSystem.h"
#include <cstdlib>
#include <iostream>

// ================================================================
// Pseudo-random hash: produces a float in [-1, 1] from integer coords
// ================================================================
float Terrain::hash(int x, int z, int seed) {
    int n = x * 73856093 ^ z * 19349663 ^ seed * 83492791;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    return 1.0f - (float)(n & 0x7fffffff) / 1073741824.0f; // [-1, 1]
}

// ================================================================
// Smooth noise with bilinear interpolation of hash values
// ================================================================
float Terrain::smoothNoise(float x, float z, int seed) {
    int ix = (int)floor(x);
    int iz = (int)floor(z);
    float fx = x - ix;
    float fz = z - iz;

    // Smoothstep for smoother interpolation
    fx = fx * fx * (3.0f - 2.0f * fx);
    fz = fz * fz * (3.0f - 2.0f * fz);

    float v00 = hash(ix, iz, seed);
    float v10 = hash(ix + 1, iz, seed);
    float v01 = hash(ix, iz + 1, seed);
    float v11 = hash(ix + 1, iz + 1, seed);

    float i1 = v00 + (v10 - v00) * fx;
    float i2 = v01 + (v11 - v01) * fx;

    return i1 + (i2 - i1) * fz;
}

// ================================================================
// Fractal Brownian Motion: multiple octaves of value noise
// ================================================================
float Terrain::valueFBM(float x, float z, int octaves, float persistence, float lacunarity, int seed) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxVal = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += smoothNoise(x * frequency, z * frequency, seed + i * 31) * amplitude;
        maxVal += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxVal; // Normalize to [-1, 1]
}

// ================================================================
// Constructor / Destructor
// ================================================================
Terrain::Terrain(float worldSize, int gridRes)
    : worldSize(worldSize), gridRes(gridRes) {
    heightmap.resize((gridRes + 1) * (gridRes + 1), 0.0f);
}

Terrain::~Terrain() {
}

// ================================================================
// Generate the heightmap
// ================================================================
void Terrain::generate(unsigned int seed) {
    float cellSize = (2.0f * worldSize) / gridRes;

    for (int iz = 0; iz <= gridRes; iz++) {
        for (int ix = 0; ix <= gridRes; ix++) {
            float wx = -worldSize + ix * cellSize;
            float wz = -worldSize + iz * cellSize;

            // Scale for terrain features:
            // Large hills:  low frequency, high amplitude
            // Small bumps:  high frequency, low amplitude
            float noiseScale = 0.04f; // Controls feature size
            float height = valueFBM(wx * noiseScale, wz * noiseScale, 5, 0.5f, 2.0f, (int)seed);

            // Map [-1,1] to terrain height range
            // Base amplitude: 4 units means hills go from -4 to +4
            float amplitude = 4.0f;
            height *= amplitude;

            // Flatten the center area so the spawn point is clear
            float distFromCenter = sqrt(wx * wx + wz * wz);
            float flattenRadius = 8.0f;
            float transitionWidth = 6.0f;
            if (distFromCenter < flattenRadius) {
                height = 0.0f;
            } else if (distFromCenter < flattenRadius + transitionWidth) {
                float t = (distFromCenter - flattenRadius) / transitionWidth;
                // Smooth blend
                t = t * t * (3.0f - 2.0f * t);
                height *= t;
            }

            heightmap[iz * (gridRes + 1) + ix] = height;
        }
    }
}

// ================================================================
// World-to-grid coordinate conversion
// ================================================================
void Terrain::worldToGrid(float wx, float wz, float& gx, float& gz) const {
    float cellSize = (2.0f * worldSize) / gridRes;
    gx = (wx + worldSize) / cellSize;
    gz = (wz + worldSize) / cellSize;
}

// ================================================================
// Get height at integer grid position (clamped)
// ================================================================
float Terrain::heightAt(int ix, int iz) const {
    if (ix < 0) ix = 0;
    if (ix > gridRes) ix = gridRes;
    if (iz < 0) iz = 0;
    if (iz > gridRes) iz = gridRes;
    return heightmap[iz * (gridRes + 1) + ix];
}

// ================================================================
// Get interpolated height at world position
// ================================================================
float Terrain::getHeight(float x, float z) const {
    float gx, gz;
    worldToGrid(x, z, gx, gz);

    // Clamp to grid bounds
    if (gx < 0) gx = 0;
    if (gx > gridRes) gx = (float)gridRes;
    if (gz < 0) gz = 0;
    if (gz > gridRes) gz = (float)gridRes;

    int ix = (int)floor(gx);
    int iz = (int)floor(gz);
    float fx = gx - ix;
    float fz = gz - iz;

    // Bilinear interpolation
    float h00 = heightAt(ix, iz);
    float h10 = heightAt(ix + 1, iz);
    float h01 = heightAt(ix, iz + 1);
    float h11 = heightAt(ix + 1, iz + 1);

    float h0 = h00 + (h10 - h00) * fx;
    float h1 = h01 + (h11 - h01) * fx;

    return h0 + (h1 - h0) * fz;
}

// ================================================================
// Get terrain normal at world position (finite differences)
// ================================================================
Vector3 Terrain::getNormal(float x, float z) const {
    float eps = 0.5f;
    float hL = getHeight(x - eps, z);
    float hR = getHeight(x + eps, z);
    float hD = getHeight(x, z - eps);
    float hU = getHeight(x, z + eps);

    // Cross product of tangent vectors gives normal
    Vector3 n(hL - hR, 2.0f * eps, hD - hU);
    return n.normalize();
}

// ================================================================
// Render the terrain mesh with optional shadow data
// ================================================================
void Terrain::render(GLuint textureId, const ShadowSystem* shadows) const {
    float cellSize = (2.0f * worldSize) / gridRes;

    bool hasTexture = (textureId != 0);
    if (hasTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }

    // Render as triangle strips, row by row
    float texScale = 0.1f; // Texture repeat every 10 world units

    for (int iz = 0; iz < gridRes; iz++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int ix = 0; ix <= gridRes; ix++) {
            // Two vertices per column: current row and next row
            for (int row = 1; row >= 0; row--) {
                int z_idx = iz + row;
                float wx = -worldSize + ix * cellSize;
                float wz = -worldSize + z_idx * cellSize;
                float h = heightmap[z_idx * (gridRes + 1) + ix];

                // Compute normal
                Vector3 n = getNormal(wx, wz);
                glNormal3f(n.x, n.y, n.z);

                // Per-vertex shadow darkening
                float shadowVal = 0.0f;
                if (shadows) {
                    shadowVal = shadows->getShadowAt(wx, wz);
                }
                float brightness = 1.0f - shadowVal * 0.5f; // Darken by up to 50%

                if (hasTexture) {
                    glColor3f(0.5f * brightness, 1.0f * brightness, 0.5f * brightness);
                } else {
                    glColor3f(0.1f * brightness, 0.6f * brightness, 0.1f * brightness);
                }

                // Texture coordinates
                glTexCoord2f(wx * texScale, wz * texScale);

                glVertex3f(wx, h, wz);
            }
        }
        glEnd();
    }

    if (hasTexture) {
        glDisable(GL_TEXTURE_2D);
    }
}
