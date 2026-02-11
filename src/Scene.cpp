#include "Scene.h"
#include <cstdlib>
#include <cmath>

Scene::Scene() : lightPos(0.0f, 5.0f, 0.0f), rotation(0.0f) {
}

Scene::~Scene() {
}

void Scene::init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
              
    // Light
    GLfloat light_position[] = { lightPos.x, lightPos.y, lightPos.z, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    // Draw solid objects
    drawFloor();
    drawWall();
    
    for (const auto& cube : cubes) {
        drawCube(cube);
    }
    
    // Draw shadows
    // Disable lighting and depth write for shadows to simply blend them on top (or use stencil for better results)
    // But for simple planar shadows, we can just project.
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST); // Disable depth test to draw on floor/wall potentially
    // Actually, proper way is to use polygon offset or just draw slightly above
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f); // Semi-transparent black
    
    glPushMatrix();
    // Shadow matrix
    Matrix4 shadowMat = Matrix4::shadow(lightPos, 0.01f); // Slightly above 0
    glMultMatrixf(shadowMat.data()); // Use our manual matrix
    
    // Draw cubes as shadows
    for (const auto& cube : cubes) {
         // Draw cube geometry again
         glPushMatrix();
         glTranslatef(cube.position.x, cube.position.y, cube.position.z);
         float s = cube.size;
         
         // Simple immediate mode cube
         glBegin(GL_QUADS);
            // Just need enough vertices to cast shadow
            // Front
            glVertex3f(-s, -s,  s);
            glVertex3f( s, -s,  s);
            glVertex3f( s,  s,  s);
            glVertex3f(-s,  s,  s);
            // Back
            glVertex3f(-s, -s, -s);
            glVertex3f(-s,  s, -s);
            glVertex3f( s,  s, -s);
            glVertex3f( s, -s, -s);
            // Top
            glVertex3f(-s,  s, -s);
            glVertex3f(-s,  s,  s);
            glVertex3f( s,  s,  s);
            glVertex3f( s,  s, -s);
            // Bottom
            glVertex3f(-s, -s, -s);
            glVertex3f( s, -s, -s);
            glVertex3f( s, -s,  s);
            glVertex3f(-s, -s,  s);
            // Right
            glVertex3f( s, -s, -s);
            glVertex3f( s,  s, -s);
            glVertex3f( s,  s,  s);
            glVertex3f( s, -s,  s);
            // Left
            glVertex3f(-s, -s, -s);
            glVertex3f(-s, -s,  s);
            glVertex3f(-s,  s,  s);
            glVertex3f(-s,  s, -s);
         glEnd();
         glPopMatrix();
    }
    
    glPopMatrix();
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    // Draw Light Source (small marker)
    glPushMatrix();
    glTranslatef(lightPos.x, lightPos.y, lightPos.z);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.0f);
    // Draw small cube as light
    float ls = 0.1f;
    glBegin(GL_QUADS);
        // Front
        glVertex3f(-ls, -ls, ls); glVertex3f(ls, -ls, ls); glVertex3f(ls, ls, ls); glVertex3f(-ls, ls, ls);
        // ... (simplified for marker)
        // Back
        glVertex3f(-ls, -ls, -ls); glVertex3f(-ls, ls, -ls); glVertex3f(ls, ls, -ls); glVertex3f(ls, -ls, -ls);
    glEnd();
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void Scene::addCube() {
    Cube c;
    c.position.x = (rand() % 100) / 10.0f - 5.0f; // -5 to 5
    c.position.y = (rand() % 100) / 10.0f - 2.0f; // -2 to 8? Let's keep it near floor. 
    c.position.y = 1.0f; // On floor-ish
    c.position.z = (rand() % 100) / 10.0f - 5.0f; // -5 to 5
    
    c.color.x = (rand() % 100) / 100.0f;
    c.color.y = (rand() % 100) / 100.0f;
    c.color.z = (rand() % 100) / 100.0f;
    
    c.size = 0.5f;
    
    cubes.push_back(c);
}

void Scene::setLightPosition(float x, float y) {
    // x, y are mouse coordinates, need to map to world space roughly
    // Or just simple mapping for now
    // Assume screen width/height mapping to -5 to 5
    lightPos.x = x * 10.0f - 5.0f;
    lightPos.y = y * 10.0f; 
    // Actually user wants to place light source anywhere in screen.
    // Let's map X to X, Y to Height? Or Y to Z? 
    // "Place light source anywhere in the screen". 
    // Let's Map Mouse X to World X, Mouse Y to World Y.
    lightPos.x = x * 10.0f - 5.0f;
    lightPos.y = (1.0f - y) * 10.0f; // Invert Y
    if (lightPos.y < 1.0f) lightPos.y = 1.0f; // Keep it above floor
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
