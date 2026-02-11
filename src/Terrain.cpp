#include "Terrain.h"
#include <cstdlib>
#include <iostream>

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
// Generate the heightmap (Flat)
// ================================================================
void Terrain::generate(unsigned int seed) {
    // Flat terrain, so just fill with 0
    std::fill(heightmap.begin(), heightmap.end(), 0.0f);
}

// ================================================================
// Get height at world position (Flat)
// ================================================================
float Terrain::getHeight(float x, float z) const {
    return 0.0f;
}

// ================================================================
// Get terrain normal at world position (Flat)
// ================================================================
Vector3 Terrain::getNormal(float x, float z) const {
    return Vector3(0.0f, 1.0f, 0.0f);
}

// ================================================================
// Render the terrain mesh
// ================================================================
void Terrain::render(GLuint textureId) const {
    float cellSize = (2.0f * worldSize) / gridRes;

    bool hasTexture = (textureId != 0);
    if (hasTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glColor3f(1.0f, 1.0f, 1.0f); 
    } else {
        glColor3f(0.1f, 0.6f, 0.1f);
    }

    // Render as triangle strips, row by row
    float texScale = 0.1f; // Texture repeat every 10 world units
    
    // Normal is always up
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int iz = 0; iz < gridRes; iz++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int ix = 0; ix <= gridRes; ix++) {
            // Two vertices per column: current row and next row
            for (int row = 1; row >= 0; row--) {
                int z_idx = iz + row;
                float wx = -worldSize + ix * cellSize;
                float wz = -worldSize + z_idx * cellSize;
                
                // Texture coordinates
                glTexCoord2f(wx * texScale, wz * texScale);

                glVertex3f(wx, 0.0f, wz);
            }
        }
        glEnd();
    }

    if (hasTexture) {
        glDisable(GL_TEXTURE_2D);
    }
}
