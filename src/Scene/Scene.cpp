#include "Scene.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ==========================================
// Shape Implementations
// ==========================================

// --- Cube ---
void Cube::draw() const {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glColor3f(color.x, color.y, color.z);
    float s = size;
    
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-s, -s,  s); glVertex3f( s, -s,  s); glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
        
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s); glVertex3f( s,  s, -s); glVertex3f( s, -s, -s);
        
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-s,  s, -s); glVertex3f(-s,  s,  s); glVertex3f( s,  s,  s); glVertex3f( s,  s, -s);
        
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s); glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
        
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f( s, -s, -s); glVertex3f( s,  s, -s); glVertex3f( s,  s,  s); glVertex3f( s, -s,  s);
        
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-s, -s, -s); glVertex3f(-s, -s,  s); glVertex3f(-s,  s,  s); glVertex3f(-s,  s, -s);
    glEnd();
    glPopMatrix();
}

void Cube::drawWireframe() const {
    float s = size * 1.02f;
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    
    glBegin(GL_LINE_STRIP);
        glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s);
        glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
        glVertex3f(-s, -s, -s);
    glEnd();
    glBegin(GL_LINE_STRIP);
        glVertex3f(-s,  s, -s); glVertex3f( s,  s, -s);
        glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
        glVertex3f(-s,  s, -s);
    glEnd();
    glBegin(GL_LINES);
        glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s);
        glVertex3f( s, -s, -s); glVertex3f( s,  s, -s);
        glVertex3f( s, -s,  s); glVertex3f( s,  s,  s);
        glVertex3f(-s, -s,  s); glVertex3f(-s,  s,  s);
    glEnd();
    glPopMatrix();
}

float Cube::intersect(const float origin[3], const float dir[3]) const {
    float t = -1.0f;
    float s = size;
    // Utilize the existing AABB test
    if (rayIntersectsAABB(origin, dir, 
                                  position.x - s, position.y - s, position.z - s,
                                  position.x + s, position.y + s, position.z + s, t)) {
        return t;
    }
    return -1.0f;
}

// --- Sphere ---
void Sphere::draw() const {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glColor3f(color.x, color.y, color.z);
    
    // Manual UV Sphere
    float radius = size;
    int latSegments = 20;
    int lonSegments = 20;
    
    for (int i = 0; i < latSegments; ++i) {
        float lat0 = M_PI * (-0.5 + (float)(i) / latSegments);
        float z0  = radius * sin(lat0);
        float zr0 = radius * cos(lat0);
        
        float lat1 = M_PI * (-0.5 + (float)(i+1) / latSegments);
        float z1 = radius * sin(lat1);
        float zr1 = radius * cos(lat1);
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= lonSegments; ++j) {
            float lng = 2 * M_PI * (float)(j - 1) / lonSegments;
            float x = cos(lng);
            float y = sin(lng);
            
            glNormal3f(x * zr0, z0, y * zr0);
            
            // Re-calc for Y-up
            float v0 = (float)i / latSegments;
            float v1 = (float)(i+1) / latSegments;
            float th0 = v0 * M_PI - M_PI/2;
            float th1 = v1 * M_PI - M_PI/2;
            
            float y0 = radius * sin(th0);
            float r0 = radius * cos(th0);
            
            float y1 = radius * sin(th1);
            float r1 = radius * cos(th1);
            
            float u = (float)j / lonSegments;
            float phi = u * 2 * M_PI;
            
            float x0 = r0 * sin(phi);
            float z0_val = r0 * cos(phi);
            
            float x1 = r1 * sin(phi);
            float z1_val = r1 * cos(phi);
            
            glNormal3f(x0/radius, y0/radius, z0_val/radius);
            glVertex3f(x0, y0, z0_val);
            
            glNormal3f(x1/radius, y1/radius, z1_val/radius);
            glVertex3f(x1, y1, z1_val);
        }
        glEnd();
    }
    glPopMatrix();
}

void Sphere::drawWireframe() const {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    
    // Draw 3 circles
    float r = size * 1.05f;
    int segs = 32;
    
    glBegin(GL_LINE_LOOP); // XY plane
    for(int i=0; i<segs; i++) {
        float a = 2*M_PI*i/segs;
        glVertex3f(cos(a)*r, sin(a)*r, 0);
    }
    glEnd();
    
    glBegin(GL_LINE_LOOP); // XZ plane
    for(int i=0; i<segs; i++) {
        float a = 2*M_PI*i/segs;
        glVertex3f(cos(a)*r, 0, sin(a)*r);
    }
    glEnd();
    
    glBegin(GL_LINE_LOOP); // YZ plane
    for(int i=0; i<segs; i++) {
        float a = 2*M_PI*i/segs;
        glVertex3f(0, cos(a)*r, sin(a)*r);
    }
    glEnd();
    
    glPopMatrix();
}

float Sphere::intersect(const float origin[3], const float dir[3]) const {
    // AABB approximation for now
    float t = -1.0f;
    if (rayIntersectsAABB(origin, dir, 
                                  position.x - size, position.y - size, position.z - size,
                                  position.x + size, position.y + size, position.z + size, t)) {
        return t;
    }
    return -1.0f;
}

// --- Cylinder ---
void Cylinder::draw() const {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glColor3f(color.x, color.y, color.z);
    
    float h = size * 2.0f; // Height
    float r = size;        // Radius
    int segs = 32;
    
    // Top Cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, h/2, 0);
    for(int i=0; i<=segs; i++) {
        float a = 2*M_PI*i/segs;
        glVertex3f(sin(a)*r, h/2, cos(a)*r);
    }
    glEnd();
    
    // Bottom Cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, -h/2, 0);
    for(int i=segs; i>=0; i--) {
        float a = 2*M_PI*i/segs;
        glVertex3f(sin(a)*r, -h/2, cos(a)*r);
    }
    glEnd();
    
    // Sides
    glBegin(GL_QUAD_STRIP);
    for(int i=0; i<=segs; i++) {
        float a = 2*M_PI*i/segs;
        float x = sin(a);
        float z = cos(a);
        
        glNormal3f(x, 0, z);
        glVertex3f(x*r, h/2, z*r);
        glVertex3f(x*r, -h/2, z*r);
    }
    glEnd();
    
    glPopMatrix();
}

void Cylinder::drawWireframe() const {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    float h = size * 2.0f * 1.02f;
    float r = size * 1.02f;
    int segs = 32;
    
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<segs; i++) {
        float a = 2*M_PI*i/segs;
        glVertex3f(sin(a)*r, h/2, cos(a)*r);
    }
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<segs; i++) {
        float a = 2*M_PI*i/segs;
        glVertex3f(sin(a)*r, -h/2, cos(a)*r);
    }
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(r, h/2, 0); glVertex3f(r, -h/2, 0);
    glVertex3f(-r, h/2, 0); glVertex3f(-r, -h/2, 0);
    glVertex3f(0, h/2, r); glVertex3f(0, -h/2, r);
    glVertex3f(0, h/2, -r); glVertex3f(0, -h/2, -r);
    glEnd();
    
    glPopMatrix();
}

float Cylinder::intersect(const float origin[3], const float dir[3]) const {
    // AABB approximation
    float h = size * 2.0f;
    float t = -1.0f;
    if (rayIntersectsAABB(origin, dir, 
                                  position.x - size, position.y - size, position.z - size,
                                  position.x + size, position.y + size, position.z + size, t)) {
        return t;
    }
    return -1.0f;
}

// --- Cone ---
void Cone::draw() const {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glColor3f(color.x, color.y, color.z);
    
    float h = size * 2.0f;
    float r = size;
    
    // Bottom Cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, -h/2, 0);
    for(int i=segments; i>=0; i--) {
        float a = 2*M_PI*i/segments;
        glVertex3f(sin(a)*r, -h/2, cos(a)*r);
    }
    glEnd();
    
    // Sides
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0); // Approx normal for tip
    glVertex3f(0, h/2, 0);
    for(int i=0; i<=segments; i++) {
        float a = 2*M_PI*i/segments;
        float x = sin(a);
        float z = cos(a);
        
        // Calculate normal for side
        // slope is h/r
        float ny = r/h; 
        float nx = x;
        float nz = z;
        // Normalize
        float len = sqrt(nx*nx + ny*ny + nz*nz);
        glNormal3f(nx/len, ny/len, nz/len);
        
        glVertex3f(x*r, -h/2, z*r);
    }
    glEnd();
    
    glPopMatrix();
}

void Cone::drawWireframe() const {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    float h = size * 2.0f * 1.02f;
    float r = size * 1.02f;
    
    glBegin(GL_LINE_LOOP);
    for(int i=0; i<segments; i++) {
        float a = 2*M_PI*i/segments;
        glVertex3f(sin(a)*r, -h/2, cos(a)*r);
    }
    glEnd();
    
    glBegin(GL_LINES);
    for(int i=0; i<segments; i++) {
         float a = 2*M_PI*i/segments;
         glVertex3f(0, h/2, 0);
         glVertex3f(sin(a)*r, -h/2, cos(a)*r);
    }
    glEnd();
    
    glPopMatrix();
}

float Cone::intersect(const float origin[3], const float dir[3]) const {
     // AABB approximation
    float t = -1.0f;
    if (rayIntersectsAABB(origin, dir, 
                                  position.x - size, position.y - size, position.z - size,
                                  position.x + size, position.y + size, position.z + size, t)) {
        return t;
    }
    return -1.0f;
}

// ==========================================
// Scene Implementation
// ==========================================

Scene::Scene() : lightActive(false), selectedIndex(-1), floorTextureId(0), wallTextureId(0),
                 camera(new Camera()), terrain(nullptr) {
    // Default light
    light.color = Vector3(1.0f, 0.9f, 0.7f);
    light.position = Vector3(0.0f, 5.0f, 0.0f);
    
    physicsEngine = new PhysicsEngine();
    terrain = new Terrain(50.0f, 128);
}

Scene::~Scene() {
    delete camera;
    delete physicsEngine;
    delete terrain;
    for(auto s : shapes) delete s;
    shapes.clear();
    physicsMap.clear();
}

GLuint Scene::loadTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLuint textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        int format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        return textureId;
    }
    return 0;
}

void Scene::init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);

    floorTextureId = loadTexture("textures/floor_texture.jpg");
    wallTextureId = loadTexture("textures/wall.jpg");

    // Generate terrain
    terrain->generate(42);

    // Give the physics engine access to terrain height
    physicsEngine->getTerrainHeight = [this](float x, float z) -> float {
        return terrain->getHeight(x, z);
    };

    // Add some default shapes
    addShape(SHAPE_CUBE, 0.0f, 0.0f, 1.0f);
    generateTrees(50);
    lightActive = true;
}

void Scene::resize(int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    MathGL::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void Scene::update(float dt) {
    physicsEngine->update(dt);
    
    // Sync graphical shapes with physics objects
    for (auto shape : shapes) {
        if (physicsMap.find(shape) != physicsMap.end()) {
            PhysicsObject* physObj = physicsMap[shape];
            shape->position = physObj->position;
        }
    }
}

void Scene::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();
    
    // Camera Follow Logic
    if (selectedIndex >= 0 && selectedIndex < (int)shapes.size()) {
        camera->setTarget(shapes[selectedIndex]->position);
    }
    
    camera->applyLookAt();
              
    if (lightActive) {
        glEnable(GL_LIGHT0);
        GLfloat light_position[] = { light.position.x, light.position.y, light.position.z, 1.0f };
        GLfloat light_diffuse[] = { light.color.x, light.color.y, light.color.z, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    }
    
    // Draw Floor with Stencil
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    drawFloor();
    glDisable(GL_STENCIL_TEST);
    
    drawWall();
    
    // Draw all shapes
    for (auto shape : shapes) {
        shape->draw();
    }
    
    // Draw trees
    for (const auto& t : trees) {
        drawTree(t);
    }
    
    // Shadows
    if (lightActive) {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 1, 0xFF); // Only on floor
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f);

        glPushMatrix();
        Matrix4 shadowMat = Matrix4::shadow(light.position, 0.0f);
        glMultMatrixf(shadowMat.data());
        
        for (auto shape : shapes) {
             // Simple check if light is in front of object relative to wall "could" be done, but
             // generic shadow volume is robust. Here we are using projection.
             // We just project everything.
             shape->draw();
        }
        
        // Tree shadows
        for (const auto& t : trees) {
            drawTree(t);
        }
        glPopMatrix();
        
        // Draw Shadow Color
        glStencilFunc(GL_EQUAL, 2, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.35f);
        
        glBegin(GL_QUADS);
            glVertex3f(-100.0f, 0.0f, 100.0f);
            glVertex3f( 100.0f, 0.0f, 100.0f);
            glVertex3f( 100.0f, 0.0f, -100.0f);
            glVertex3f(-100.0f, 0.0f, -100.0f);
        glEnd();
        
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_LIGHTING);
        
        // Draw Light
        glPushMatrix();
        glTranslatef(light.position.x, light.position.y, light.position.z);
        glDisable(GL_LIGHTING);
        glColor3f(light.color.x, light.color.y, light.color.z); 
        drawLightWireframe(Vector3(0,0,0), 0.15f); // Local coords
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    // Selection Highlight
    if (selectedIndex >= 0) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(2.5f);

        if (selectedIndex < (int)shapes.size()) {
            shapes[selectedIndex]->drawWireframe();
        } else if (selectedIndex == (int)shapes.size() && lightActive) {
            drawLightWireframe(light.position, 0.15f);
        }

        glLineWidth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }
}

void Scene::addShape(ShapeType type, float r, float g, float b) {
    float x = (rand() % 100) / 10.0f - 5.0f;
    float z = (rand() % 100) / 10.0f - 5.0f;
    addShapeAt(type, x, z, r, g, b);
}

void Scene::addShapeAt(ShapeType type, float x, float z, float r, float g, float b) {
    float groundY = terrain ? terrain->getHeight(x, z) : 0.0f;
    float size = 0.5f;
    Vector3 pos(x, groundY + size, z);
    Vector3 col(r, g, b);

    Shape* newShape = nullptr;
    switch(type) {
        case SHAPE_CUBE: newShape = new Cube(pos, col, size); break;
        case SHAPE_SPHERE: newShape = new Sphere(pos, col, size); break;
        case SHAPE_CYLINDER: newShape = new Cylinder(pos, col, size); break;
        case SHAPE_CONE: newShape = new Cone(pos, col, size, 32); break;
        case SHAPE_TRICONE: newShape = new Cone(pos, col, size, 3); break;
    }
    
    if (newShape) {
        shapes.push_back(newShape);
        
        // Add to physics engine
        PhysicsObject* physObj = physicsEngine->addObject(pos);
        // Default properties for now
        physObj->mass = 1.0f;
        physObj->mass = 1.0f;
        physObj->friction = 2.0f; // Reasonable friction
        physObj->size = Vector3(size, size, size); // Set size
        
        if (type == SHAPE_CUBE) {
             // Maybe give cube different properties?
        }
        
        physicsMap[newShape] = physObj;
    }
}

void Scene::rotateCamera(float dx, float dy) {
    camera->rotate(dx, dy);
}

void Scene::zoomCamera(float delta) {
    camera->zoom(delta);
}

void Scene::getCameraPosition(float& x, float& y, float& z) const {
    camera->getPosition(x, y, z);
}

Camera* Scene::getCamera() const {
    return camera;
}

float Scene::getTerrainHeight(float x, float z) const {
    if (terrain) return terrain->getHeight(x, z);
    return 0.0f;
}

void Scene::setLightWorldPos(float x, float y, float z) {
    light.position = Vector3(x, y, z);
    lightActive = true;
}

Vector3 Scene::getLightPosition() const { return light.position; }
bool Scene::hasLight() const { return lightActive; }

int Scene::pickObject(float screenX, float screenY, int vpW, int vpH) {
    float origin[3], dir[3];
    float camX, camY, camZ;
    camera->getPosition(camX, camY, camZ);
    Vector3 target = camera->getTarget();
    
    buildScreenRay(screenX, screenY, vpW, vpH, camX, camY, camZ, target.x, target.y, target.z, origin, dir);

    int bestIdx = -1;
    float bestT = 1e30f;

    for (int i = 0; i < (int)shapes.size(); i++) {
        float t = shapes[i]->intersect(origin, dir);
        if (t > 0 && t < bestT) {
            bestT = t;
            bestIdx = i;
        }
    }

    // Light picking
    if (lightActive) {
        float ls = 0.15f;
        float t;
        if (rayIntersectsAABB(origin, dir,
                                      light.position.x - ls, light.position.y - ls, light.position.z - ls,
                                      light.position.x + ls, light.position.y + ls, light.position.z + ls, t)) {
            if (t < bestT) {
                bestIdx = (int)shapes.size();
            }
        }
    }

    return bestIdx;
}

void Scene::setSelected(int index) { selectedIndex = index; }
int Scene::getSelected() const { return selectedIndex; }

void Scene::moveShape(int index, float x, float z) {
    if (index >= 0 && index < (int)shapes.size()) {
        shapes[index]->position.x = x;
        shapes[index]->position.z = z;
    }
}

int Scene::shapeCount() const { return (int)shapes.size(); }

Vector3 Scene::getShapePosition(int index) const {
    if (index >= 0 && index < (int)shapes.size()) {
        return shapes[index]->position;
    }
    return Vector3();
}

ShapeType Scene::getShapeType(int index) const {
    if (index >= 0 && index < (int)shapes.size()) {
        return shapes[index]->getType();
    }
    return SHAPE_CUBE;
}

void Scene::changeShapeType(int index, ShapeType newType) {
    if (index < 0 || index >= (int)shapes.size()) return;

    Shape* old = shapes[index];
    Vector3 pos = old->position;
    Vector3 col = old->color;
    float sz = old->size;

    // Create new shape of the target type
    Shape* newShape = nullptr;
    switch(newType) {
        case SHAPE_CUBE:     newShape = new Cube(pos, col, sz); break;
        case SHAPE_SPHERE:   newShape = new Sphere(pos, col, sz); break;
        case SHAPE_CYLINDER: newShape = new Cylinder(pos, col, sz); break;
        case SHAPE_CONE:     newShape = new Cone(pos, col, sz, 32); break;
        case SHAPE_TRICONE:  newShape = new Cone(pos, col, sz, 3); break;
    }
    if (!newShape) return;

    // Transfer physics mapping
    auto it = physicsMap.find(old);
    if (it != physicsMap.end()) {
        PhysicsObject* physObj = it->second;
        physicsMap.erase(it);
        physicsMap[newShape] = physObj;
    }

    shapes[index] = newShape;
    delete old;
}

void Scene::moveSelectedShape(float dx, float dz) {
    
    if (selectedIndex >= 0 && selectedIndex < (int)shapes.size()) {
        PhysicsObject* physObj = getPhysicsObject(selectedIndex);
        if (physObj) {
            physObj->position.x += dx;
            physObj->position.z += dz;
            // Reset velocity/accel if manually moved?
            physObj->velocity = Vector3(0,0,0);
            physObj->acceleration = Vector3(0,0,0);
        } else {
            shapes[selectedIndex]->position.x += dx;
            shapes[selectedIndex]->position.z += dz;
        }
    }
}

PhysicsObject* Scene::getPhysicsObject(int index) {
    if (index >= 0 && index < (int)shapes.size()) {
        Shape* s = shapes[index];
        if (physicsMap.find(s) != physicsMap.end()) {
            return physicsMap[s];
        }
    }
    return nullptr;
}

void Scene::drawFloor() {
    if (terrain) {
        terrain->render(floorTextureId);
    } else {
        // Fallback flat floor
        if (floorTextureId != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, floorTextureId);
            glColor3f(0.5f, 1.0f, 0.5f);
        } else {
            glColor3f(0.1f, 0.6f, 0.1f);
        }
        glBegin(GL_QUADS);
            glNormal3f(0.0f, 1.0f, 0.0f);
            float sz = 100.0f;
            float rep = 10.0f;
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-sz, 0.0f, sz);
            glTexCoord2f(rep, 0.0f); glVertex3f( sz, 0.0f, sz);
            glTexCoord2f(rep, rep); glVertex3f( sz, 0.0f, -sz);
            glTexCoord2f(0.0f, rep); glVertex3f(-sz, 0.0f, -sz);
        glEnd();
        if (floorTextureId != 0) glDisable(GL_TEXTURE_2D);
    }
}

void Scene::drawWall() {
    // No walls
}

void Scene::drawLightWireframe(const Vector3& pos, float size) {
    // Reuse Cube::drawWireframe logic or custom small cube
    float ls = size;
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    
    glBegin(GL_LINE_STRIP);
        glVertex3f(-ls, -ls, ls); glVertex3f(ls, -ls, ls); glVertex3f(ls, ls, ls); glVertex3f(-ls, ls, ls); glVertex3f(-ls, -ls, ls);
    glEnd();
    glBegin(GL_LINE_STRIP);
        glVertex3f(-ls, -ls, -ls); glVertex3f(-ls, ls, -ls); glVertex3f(ls, ls, -ls); glVertex3f(ls, -ls, -ls); glVertex3f(-ls, -ls, -ls);
    glEnd();
    glBegin(GL_LINES);
        glVertex3f(-ls, ls, -ls); glVertex3f(-ls, ls, ls);
        glVertex3f(ls, ls, ls); glVertex3f(ls, ls, -ls);
        glVertex3f(ls, -ls, -ls); glVertex3f(ls, -ls, ls);
        glVertex3f(-ls, -ls, -ls); glVertex3f(-ls, -ls, ls);
    glEnd();
    glEnd();
    glPopMatrix();
}

void Scene::generateTrees(int count) {
    trees.clear();
    for (int i = 0; i < count; i++) {
        Tree t;
        // Random position between -40 and 40
        float x = (rand() % 800) / 10.0f - 40.0f;
        float z = (rand() % 800) / 10.0f - 40.0f;
        
        // Avoid center area (keep spawn clear)
        if (x > -5 && x < 5 && z > -5 && z < 5) continue;
        
        // Place tree on terrain surface
        float groundY = terrain ? terrain->getHeight(x, z) : 0.0f;
        t.position = Vector3(x, groundY, z);
        // Bigger trees: 1.5 to 3.0
        t.size = (rand() % 150) / 100.0f + 1.5f; 
        
        // Physics
        PhysicsObject* p = physicsEngine->addObject(t.position);
        p->isStatic = true;
        float trunkRadius = t.size * 0.3f;
        float totalHeight = t.size * 5.0f;
        p->size = Vector3(trunkRadius, totalHeight, trunkRadius);
        
        t.physObj = p;
        trees.push_back(t);
    }
}

void Scene::drawTree(const Tree& tree) {
    glPushMatrix();
    glTranslatef(tree.position.x, tree.position.y, tree.position.z);
    
    // Trunk
    glColor3f(0.55f, 0.27f, 0.07f);
    float r = tree.size * 0.2f;
    float h = tree.size * 1.5f;
    
    glBegin(GL_QUAD_STRIP);
    for(int i=0; i<=8; i++) {
        float angle = 2.0f * M_PI * i / 8;
        float x = cos(angle) * r;
        float z = sin(angle) * r;
        glNormal3f(cos(angle), 0.0f, sin(angle));
        glVertex3f(x, h, z);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    // Leaves (Cone)
    glColor3f(0.0f, 0.8f, 0.0f); // Brighter green for leaves
    float r_cone = tree.size * 0.8f;
    float h_cone = tree.size * 2.5f;
    float y_base = h * 0.8f; // Overlap slightly
    
    // Cone Sides
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, y_base + h_cone, 0.0f); // Top
    for(int i=0; i<=8; i++) {
        float angle = 2.0f * M_PI * i / 8;
        float x = cos(angle) * r_cone;
        float z = sin(angle) * r_cone;
        // Normal could be improved
        glNormal3f(x, 0.5f, z); 
        glVertex3f(x, y_base, z);
    }
    glEnd();
    
    // Cone Base
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0.0f, y_base, 0.0f);
    for(int i=8; i>=0; i--) {
        float angle = 2.0f * M_PI * i / 8;
        float x = cos(angle) * r_cone;
        float z = sin(angle) * r_cone;
        glVertex3f(x, y_base, z);
    }
    glEnd();
    
    glPopMatrix();
}
