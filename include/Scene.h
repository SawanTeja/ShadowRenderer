#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <vector>
#include <GL/gl.h>
#include "MathUtils.h"

struct Cube {
    Vector3 position;
    Vector3 color;
    float size;
};

class Scene {
public:
    Scene();
    ~Scene();

    void init();
    void render();
    void resize(int width, int height);

    void addCube(float r, float g, float b);
    void addCubeAt(float x, float z, float r, float g, float b);
    void addLight();
    void addLightAt(float x, float z, float r, float g, float b);
    void setLightPosition(float x, float y);

private:
    struct Light {
        Vector3 position;
        Vector3 color;
    };

    std::vector<Cube> cubes;
    std::vector<Light> lights;
    // Vector3 lightPos; // Replaced by lights vector
    float rotation;
    
    void drawCube(const Cube& cube);
    void drawFloor();
    void drawWall();
    void drawShadows();
    
    // Display list IDs if needed, or just immediate mode for simplicity
};

#endif // SCENE_H
