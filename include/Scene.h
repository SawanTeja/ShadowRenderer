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

    // Picking & selection
    // Returns -1 = nothing, 0..N-1 = cube index, N = light
    int pickObject(float screenX, float screenY, int vpW, int vpH);
    void setSelected(int index);
    int  getSelected() const;

    // Move APIs
    void moveCube(int index, float x, float z);
    int  cubeCount() const;
    Vector3 getCubePosition(int index) const;

private:
    struct Light {
        Vector3 position;
        Vector3 color;
    };

    std::vector<Cube> cubes;
    Light light;
    bool lightActive;
    float rotation;
    int selectedIndex;  // -1 = none
    
    void drawCube(const Cube& cube);
    void drawCubeWireframe(const Vector3& pos, float size);
    void drawFloor();
    void drawWall();

    GLuint floorTextureId;
    GLuint wallTextureId;
    GLuint loadTexture(const char* filename);
};

#endif // SCENE_H
