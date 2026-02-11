#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <GL/gl.h>
#include "MathUtils.h"
#include "Camera.h"
#include "Physics/PhysicsEngine.h"
#include <map>

// Enum for Shape Type
enum ShapeType {
    SHAPE_CUBE,
    SHAPE_SPHERE,
    SHAPE_CYLINDER,
    SHAPE_CONE,
    SHAPE_TRICONE
};

// Abstract Base Class
class Shape {
public:
    Vector3 position;
    Vector3 color;
    float size;

    Shape(const Vector3& pos, const Vector3& col, float s) 
        : position(pos), color(col), size(s) {}
    virtual ~Shape() {}

    virtual void draw() const = 0;
    virtual void drawWireframe() const = 0;
    
    // Returns t if hit, or -1 if no hit. origin and dir are in world space.
    virtual float intersect(const float origin[3], const float dir[3]) const = 0;
};

class Cube : public Shape {
public:
    Cube(const Vector3& pos, const Vector3& col, float s) : Shape(pos, col, s) {}
    void draw() const override;
    void drawWireframe() const override;
    float intersect(const float origin[3], const float dir[3]) const override;
};

class Sphere : public Shape {
public:
    Sphere(const Vector3& pos, const Vector3& col, float s) : Shape(pos, col, s) {}
    void draw() const override;
    void drawWireframe() const override;
    float intersect(const float origin[3], const float dir[3]) const override;
};

class Cylinder : public Shape {
public:
    Cylinder(const Vector3& pos, const Vector3& col, float s) : Shape(pos, col, s) {}
    void draw() const override;
    void drawWireframe() const override;
    float intersect(const float origin[3], const float dir[3]) const override;
};

// Cone / Tricone (depending on segments)
class Cone : public Shape {
public:
    int segments; // 3 for Tricone, 20+ for Cone
    Cone(const Vector3& pos, const Vector3& col, float s, int segs = 32) 
        : Shape(pos, col, s), segments(segs) {}
    void draw() const override;
    void drawWireframe() const override;
    float intersect(const float origin[3], const float dir[3]) const override;
};

class Scene {
public:
    Scene();
    ~Scene();

    void init();
    void update(float dt);
    void render();
    void resize(int width, int height);

    void addShape(ShapeType type, float r, float g, float b);
    void addShapeAt(ShapeType type, float x, float z, float r, float g, float b);

    // Camera Control
    void rotateCamera(float dx, float dy);
    void zoomCamera(float delta);
    void getCameraPosition(float& x, float& y, float& z) const;

    // Single light API
    void setLightWorldPos(float x, float y, float z);
    Vector3 getLightPosition() const;
    bool hasLight() const;

    // Picking & selection
    // Returns -1 = nothing, 0..N-1 = shape index, N = light
    int pickObject(float screenX, float screenY, int vpW, int vpH);
    void setSelected(int index);
    int  getSelected() const;

    // Move APIs
    void moveShape(int index, float x, float z);
    void moveSelectedShape(float dx, float dz); // Relative move
    int  shapeCount() const;
    Vector3 getShapePosition(int index) const;

    // Physics Access
    PhysicsEngine* getPhysicsEngine() const { return physicsEngine; }
    PhysicsObject* getPhysicsObject(int index);
    
    // Camera Access
    Camera* getCamera() const;

private:
    struct Light {
        Vector3 position;
        Vector3 color;
    };

    std::vector<Shape*> shapes;
    Light light;
    bool lightActive;
    
    // Camera State
    Camera* camera;
    
    int selectedIndex;  // -1 = none
    
    PhysicsEngine* physicsEngine;
    std::map<Shape*, PhysicsObject*> physicsMap;

    void drawFloor();
    void drawWall();
    void drawLightWireframe(const Vector3& pos, float size);

    GLuint floorTextureId;
    GLuint wallTextureId;
    GLuint loadTexture(const char* filename);
};

#endif // SCENE_H
