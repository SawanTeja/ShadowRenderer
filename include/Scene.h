#ifndef SCENE_H
#define SCENE_H

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

    // Single light API
    void setLightWorldPos(float x, float y, float z);
    Vector3 getLightPosition() const;
    bool hasLight() const;

private:
    struct Light {
        Vector3 position;
        Vector3 color;
    };

    std::vector<Cube> cubes;
    Light light;
    bool lightActive;
    float rotation;
    
    void drawCube(const Cube& cube);
    void drawFloor();
    void drawWall();
};

#endif // SCENE_H
