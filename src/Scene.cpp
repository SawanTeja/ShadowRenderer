#include "Scene.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

Scene::Scene() : rotation(0.0f), lightActive(false) {
    // Default light color: warm white
    light.color = Vector3(1.0f, 0.9f, 0.7f);
    light.position = Vector3(0.0f, 5.0f, 0.0f);
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
    
    // Activate the default light
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

void Scene::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    MathGL::lookAt(0.0f, 5.0f, 10.0f, 
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);
              
    // Set up the single light
    if (lightActive) {
        glEnable(GL_LIGHT0);
        GLfloat light_position[] = { light.position.x, light.position.y, light.position.z, 1.0f };
        GLfloat light_diffuse[] = { light.color.x, light.color.y, light.color.z, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    }
    
    // Draw solid objects
    drawFloor();
    drawWall();
    
    for (const auto& cube : cubes) {
        drawCube(cube);
    }
    
    // Draw shadow from the single light
    if (lightActive) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
        
        glPushMatrix();
        Matrix4 shadowMat = Matrix4::shadow(light.position, 0.01f);
        glMultMatrixf(shadowMat.data());
        
        for (const auto& cube : cubes) {
             glPushMatrix();
             glTranslatef(cube.position.x, cube.position.y, cube.position.z);
             float s = cube.size;
             
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
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        
        // Draw light source indicator (small yellow cube)
        glPushMatrix();
        glTranslatef(light.position.x, light.position.y, light.position.z);
        glDisable(GL_LIGHTING);
        glColor3f(light.color.x, light.color.y, light.color.z); 
        
        float ls = 0.15f;
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
    c.position.x = (rand() % 100) / 10.0f - 5.0f;
    c.position.y = 0.5f;
    c.position.z = (rand() % 100) / 10.0f - 5.0f;
    
    c.color.x = r;
    c.color.y = g;
    c.color.z = b;
    
    c.size = 0.5f;
    
    cubes.push_back(c);
}

void Scene::addCubeAt(float x, float z, float r, float g, float b) {
    Cube c;
    c.position.x = x;
    c.position.y = 0.5f;
    c.position.z = z;
    
    c.color.x = r;
    c.color.y = g;
    c.color.z = b;
    
    c.size = 0.5f;
    
    cubes.push_back(c);
}

void Scene::setLightWorldPos(float x, float y, float z) {
    light.position.x = x;
    light.position.y = y;
    light.position.z = z;
    lightActive = true;
}

Vector3 Scene::getLightPosition() const {
    return light.position;
}

bool Scene::hasLight() const {
    return lightActive;
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
