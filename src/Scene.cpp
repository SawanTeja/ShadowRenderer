#include "Scene.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

Scene::Scene() : rotation(0.0f) {
}

Scene::~Scene() {
}

void Scene::init() {
    std::cout << "Scene::init() called" << std::endl;
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << (renderer ? (const char*)renderer : "Unknown") << std::endl;
    std::cout << "OpenGL Version: " << (version ? (const char*)version : "Unknown") << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Add a default cube - Blue
    addCube(0.0f, 0.0f, 1.0f);
    
    // Add default light
    addLight();
    if (!lights.empty()) {
        lights[0].position = Vector3(0.0f, 5.0f, 0.0f);
    }
}

void Scene::resize(int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    MathGL::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
}

void Scene::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    MathGL::lookAt(0.0f, 5.0f, 10.0f, 
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);
              
    // Enabled lights
    // OpenGL fixed pipeline supports GL_LIGHT0 to GL_LIGHT7
    for (size_t i = 0; i < lights.size() && i < 8; ++i) {
        glEnable(GL_LIGHT0 + i);
        GLfloat light_position[] = { lights[i].position.x, lights[i].position.y, lights[i].position.z, 1.0f };
        GLfloat light_diffuse[] = { lights[i].color.x, lights[i].color.y, lights[i].color.z, 1.0f };
        glLightfv(GL_LIGHT0 + i, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, light_diffuse);
    }
    
    // Draw solid objects
    drawFloor();
    drawWall();
    
    for (const auto& cube : cubes) {
        drawCube(cube);
    }
    
    // Draw shadows for EACH light
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (const auto& light : lights) {
        // Shadow color based on light intensity/count? just constant for now
        glColor4f(0.0f, 0.0f, 0.0f, 0.3f); // Semi-transparent black, lighter for multi-shadows
        
        glPushMatrix();
        Matrix4 shadowMat = Matrix4::shadow(light.position, 0.01f);
        glMultMatrixf(shadowMat.data());
        
        for (const auto& cube : cubes) {
             glPushMatrix();
             glTranslatef(cube.position.x, cube.position.y, cube.position.z);
             float s = cube.size;
             
             // Draw Cube Geometry (simplified)
             glBegin(GL_QUADS);
                // Front
                glVertex3f(-s, -s,  s); glVertex3f( s, -s,  s); glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
                // Back
                glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s); glVertex3f( s,  s, -s); glVertex3f( s, -s, -s);
                // Top
                glVertex3f(-s,  s, -s); glVertex3f(-s,  s,  s); glVertex3f( s,  s,  s); glVertex3f( s,  s, -s);
                // Bottom
                glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s); glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
                // Right
                glVertex3f( s, -s, -s); glVertex3f( s,  s, -s); glVertex3f( s,  s,  s); glVertex3f( s, -s,  s);
                // Left
                glVertex3f(-s, -s, -s); glVertex3f(-s, -s,  s); glVertex3f(-s,  s,  s); glVertex3f(-s,  s, -s);
             glEnd();
             glPopMatrix();
        }
        glPopMatrix();
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    
    // Draw Light Sources
    for (const auto& light : lights) {
        glPushMatrix();
        glTranslatef(light.position.x, light.position.y, light.position.z);
        glDisable(GL_LIGHTING);
        glColor3f(light.color.x, light.color.y, light.color.z); 
        
        // Draw small cube as light
        float ls = 0.1f;
        glBegin(GL_QUADS);
            // Front
            glVertex3f(-ls, -ls, ls); glVertex3f(ls, -ls, ls); glVertex3f(ls, ls, ls); glVertex3f(-ls, ls, ls);
            // Back
            glVertex3f(-ls, -ls, -ls); glVertex3f(-ls, ls, -ls); glVertex3f(ls, ls, -ls); glVertex3f(ls, -ls, -ls);
            // Top
            glVertex3f(-ls, ls, -ls); glVertex3f(-ls, ls, ls); glVertex3f(ls, ls, ls); glVertex3f(ls, ls, -ls);
             // Bottom
            glVertex3f(-ls, -ls, -ls); glVertex3f(ls, -ls, -ls); glVertex3f(ls, -ls, ls); glVertex3f(-ls, -ls, ls);
             // Right
            glVertex3f(ls, -ls, -ls); glVertex3f(ls, ls, -ls); glVertex3f(ls, ls, ls); glVertex3f(ls, -ls, ls);
             // Left
            glVertex3f(-ls, -ls, -ls); glVertex3f(-ls, -ls, ls); glVertex3f(-ls, ls, ls); glVertex3f(-ls, ls, -ls);
        glEnd();
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
}

void Scene::addCube(float r, float g, float b) {
    Cube c;
    c.position.x = (rand() % 100) / 10.0f - 5.0f; // -5 to 5
    // c.position.y = (rand() % 100) / 10.0f - 2.0f; 
    c.position.y = 0.5f; // Size is 0.5, so radius is 0.5. To sit on floor(0), y needs to be 0.5
    c.position.z = (rand() % 100) / 10.0f - 5.0f; // -5 to 5
    
    c.color.x = r;
    c.color.y = g;
    c.color.z = b;
    
    c.size = 0.5f;
    
    cubes.push_back(c);
}

void Scene::addCubeAt(float x, float z, float r, float g, float b) {
    Cube c;
    c.position.x = x;
    c.position.y = 0.5f; // Sit on floor
    c.position.z = z;
    
    c.color.x = r;
    c.color.y = g;
    c.color.z = b;
    
    c.size = 0.5f;
    
    cubes.push_back(c);
}

void Scene::addLight() {
    if (lights.size() >= 8) return; // OpenGL limit
    
    Light l;
    l.position.x = (rand() % 100) / 10.0f - 5.0f;
    l.position.y = (rand() % 50) / 10.0f + 2.0f; // 2.0 to 7.0 height
    l.position.z = (rand() % 100) / 10.0f - 5.0f;
    
    // Random color or just warm white
    l.color.x = 1.0f;
    l.color.y = (rand() % 50 + 50) / 100.0f; // 0.5 - 1.0 (Orange-ish to White)
    l.color.z = 0.0f;
    
    lights.push_back(l);
}

void Scene::addLightAt(float x, float z, float r, float g, float b) {
    if (lights.size() >= 8) return; 
    
    Light l;
    l.position.x = x;
    l.position.y = 3.0f; // Height matched to ray-cast plane so light appears at click position
    l.position.z = z;
    
    l.color.x = r;
    l.color.y = g;
    l.color.z = b;
    
    lights.push_back(l);
}

void Scene::setLightPosition(float x, float y) {
    if (lights.empty()) return;
    
    // Modify the LAST light added
    Light& l = lights.back();
    
    l.position.x = x * 10.0f - 5.0f;
    l.position.y = (1.0f - y) * 10.0f;
    if (l.position.y < 1.0f) l.position.y = 1.0f;
}

void Scene::drawCube(const Cube& cube) {
    glPushMatrix();
    glTranslatef(cube.position.x, cube.position.y, cube.position.z);
    
    glColor3f(cube.color.x, cube.color.y, cube.color.z);
    float s = cube.size;
    
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

void Scene::drawFloor() {
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-10.0f, 0.0f, 10.0f);
        glVertex3f( 10.0f, 0.0f, 10.0f);
        glVertex3f( 10.0f, 0.0f, -10.0f);
        glVertex3f(-10.0f, 0.0f, -10.0f);
    glEnd();
}

void Scene::drawWall() {
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-10.0f, 0.0f, -5.0f);
        glVertex3f( 10.0f, 0.0f, -5.0f);
        glVertex3f( 10.0f, 10.0f, -5.0f);
        glVertex3f(-10.0f, 10.0f, -5.0f);
    glEnd();
}
