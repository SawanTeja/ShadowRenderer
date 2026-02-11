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

    void addCube();
    void setLightPosition(float x, float y);

private:
    std::vector<Cube> cubes;
    Vector3 lightPos;
    float rotation;
    
    void drawCube(const Cube& cube);
    void drawFloor();
    void drawWall();
    void drawShadows();
    
    // Display list IDs if needed, or just immediate mode for simplicity
};

#endif // SCENE_H
