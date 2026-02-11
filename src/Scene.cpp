#include "Scene.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Scene::Scene() : rotation(0.0f), lightActive(false), selectedIndex(-1), floorTextureId(0), wallTextureId(0),
                 cameraYaw(0.0f), cameraPitch(0.5f), cameraDistance(10.0f) {
    // Default light color: warm white
    light.color = Vector3(1.0f, 0.9f, 0.7f);
    light.position = Vector3(0.0f, 5.0f, 0.0f);
}

Scene::~Scene() {
}

GLuint Scene::loadTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLuint textureId;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        // Set texture wrapping to GL_REPEAT (usually basic for floors/walls)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        
        stbi_image_free(data);
        std::cout << "Successfully loaded texture: " << filename << std::endl;
        return textureId;
    } else {
        std::cout << "Failed to load texture: " << filename << std::endl;
        return 0;
    }
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

    // Load textures
    floorTextureId = loadTexture("textures/floor_texture.jpg");
    wallTextureId = loadTexture("textures/wall.jpg");

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();
    
    float camX = cameraDistance * cos(cameraPitch) * sin(cameraYaw);
    float camY = cameraDistance * sin(cameraPitch);
    float camZ = cameraDistance * cos(cameraPitch) * cos(cameraYaw);
    
    MathGL::lookAt(camX, camY, camZ, 
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
    
    // ---- Step 1: Draw the floor (with texture if avail) AND mark it in stencil (stencil = 1) ----
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    drawFloor();
    glDisable(GL_STENCIL_TEST);
    
    // Draw wall (with texture if avail) and cubes (no stencil writes)
    drawWall();
    for (const auto& cube : cubes) {
        drawCube(cube);
    }
    
        // ---- Step 2: Draw shadows using two-pass stencil technique ----
        // Pass 1: Project shadow geometry into stencil only (no color/depth writes).
        // Pass 2: Draw one floor quad with shadow color where stencil was marked.
        // This gives correct shadow shapes AND zero fringes.
        if (lightActive) {
            const float wallZ = -5.0f;
            bool lightInFront = (light.position.z > wallZ);
            
            // --- Pass 1: write shadow shape into stencil ---
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_EQUAL, 1, 0xFF);        // Only on floor (stencil==1)
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);  // Mark shadow pixels as 2
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // No color output
            glDepthMask(GL_FALSE);                    // No depth writes
            
            // Use polygon offset to resolve z-fighting with the floor
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);

            glPushMatrix();
            // Project to exactly y=0.0f (no manual offset)
            Matrix4 shadowMat = Matrix4::shadow(light.position, 0.0f);
            glMultMatrixf(shadowMat.data());
            
            for (const auto& cube : cubes) {
                bool cubeInFront = (cube.position.z > wallZ);
                if (lightInFront != cubeInFront) continue;
                
                glPushMatrix();
                glTranslatef(cube.position.x, cube.position.y, cube.position.z);
                float s = cube.size;
                
                glBegin(GL_QUADS);
                    glVertex3f(-s, -s,  s); glVertex3f( s, -s,  s); glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
                    glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s); glVertex3f( s,  s, -s); glVertex3f( s, -s, -s);
                    glVertex3f(-s,  s, -s); glVertex3f(-s,  s,  s); glVertex3f( s,  s,  s); glVertex3f( s,  s, -s);
                    glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s); glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
                    glVertex3f( s, -s, -s); glVertex3f( s,  s, -s); glVertex3f( s,  s,  s); glVertex3f( s, -s,  s);
                    glVertex3f(-s, -s, -s); glVertex3f(-s, -s,  s); glVertex3f(-s,  s,  s); glVertex3f(-s,  s, -s);
                glEnd();
                glPopMatrix();
            }
            glPopMatrix();
            
            // --- Pass 2: draw ONE quad with shadow color where stencil == 2 ---
            glStencilFunc(GL_EQUAL, 2, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.0f, 0.0f, 0.0f, 0.35f);
            
            // Draw quad exactly at y=0.0f, relying on polygon offset
            glBegin(GL_QUADS);
                glVertex3f(-10.0f, 0.0f, 10.0f);
                glVertex3f( 10.0f, 0.0f, 10.0f);
                glVertex3f( 10.0f, 0.0f, -10.0f);
                glVertex3f(-10.0f, 0.0f, -10.0f);
            glEnd();
            
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
            glDisable(GL_STENCIL_TEST);
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

    // Draw selection highlight wireframe
    if (selectedIndex >= 0) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glColor3f(1.0f, 1.0f, 0.0f); // Bright yellow
        glLineWidth(2.5f);

        if (selectedIndex < (int)cubes.size()) {
            // Highlight a cube
            const Cube& c = cubes[selectedIndex];
            drawCubeWireframe(c.position, c.size);
        } else if (selectedIndex == (int)cubes.size() && lightActive) {
            // Highlight the light
            drawCubeWireframe(light.position, 0.15f);
        }

        glLineWidth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
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

void Scene::rotateCamera(float dx, float dy) {
    cameraYaw += dx;
    cameraPitch += dy;
    
    // Clamp pitch to avoid flipping
    if (cameraPitch < 0.1f) cameraPitch = 0.1f;
    if (cameraPitch > 1.5f) cameraPitch = 1.5f; // Slightly less than PI/2
}

void Scene::zoomCamera(float delta) {
    cameraDistance -= delta;
    if (cameraDistance < 2.0f) cameraDistance = 2.0f;
    if (cameraDistance > 50.0f) cameraDistance = 50.0f;
}

void Scene::getCameraPosition(float& x, float& y, float& z) const {
    x = cameraDistance * cos(cameraPitch) * sin(cameraYaw);
    y = cameraDistance * sin(cameraPitch);
    z = cameraDistance * cos(cameraPitch) * cos(cameraYaw);
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

// ---------- Picking & selection ----------

int Scene::pickObject(float screenX, float screenY, int vpW, int vpH) {
    float origin[3], dir[3];
    
    float camX = cameraDistance * cos(cameraPitch) * sin(cameraYaw);
    float camY = cameraDistance * sin(cameraPitch);
    float camZ = cameraDistance * cos(cameraPitch) * cos(cameraYaw);

    buildScreenRay(screenX, screenY, vpW, vpH, camX, camY, camZ, origin, dir);

    int bestIdx = -1;
    float bestT = 1e30f;

    // Test each cube
    for (int i = 0; i < (int)cubes.size(); i++) {
        const Cube& c = cubes[i];
        float s = c.size;
        float t;
        if (rayIntersectsAABB(origin, dir,
                              c.position.x - s, c.position.y - s, c.position.z - s,
                              c.position.x + s, c.position.y + s, c.position.z + s, t)) {
            if (t < bestT) { bestT = t; bestIdx = i; }
        }
    }

    // Test the light indicator (small cube around light position)
    if (lightActive) {
        float ls = 0.15f;
        float t;
        if (rayIntersectsAABB(origin, dir,
                              light.position.x - ls, light.position.y - ls, light.position.z - ls,
                              light.position.x + ls, light.position.y + ls, light.position.z + ls, t)) {
            if (t < bestT) { bestT = t; bestIdx = (int)cubes.size(); }
        }
    }

    return bestIdx;
}

void Scene::setSelected(int index) {
    selectedIndex = index;
}

int Scene::getSelected() const {
    return selectedIndex;
}

void Scene::moveCube(int index, float x, float z) {
    if (index >= 0 && index < (int)cubes.size()) {
        cubes[index].position.x = x;
        cubes[index].position.z = z;
    }
}

int Scene::cubeCount() const {
    return (int)cubes.size();
}

Vector3 Scene::getCubePosition(int index) const {
    if (index >= 0 && index < (int)cubes.size()) {
        return cubes[index].position;
    }
    return Vector3();
}

// ---------- Drawing helpers ----------

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

void Scene::drawCubeWireframe(const Vector3& pos, float size) {
    float s = size * 1.02f; // Slightly larger to avoid z-fighting
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);

    glBegin(GL_LINE_STRIP);
        // Bottom face
        glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s);
        glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
        glVertex3f(-s, -s, -s);
    glEnd();
    glBegin(GL_LINE_STRIP);
        // Top face
        glVertex3f(-s,  s, -s); glVertex3f( s,  s, -s);
        glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
        glVertex3f(-s,  s, -s);
    glEnd();
    glBegin(GL_LINES);
        // Vertical edges
        glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s);
        glVertex3f( s, -s, -s); glVertex3f( s,  s, -s);
        glVertex3f( s, -s,  s); glVertex3f( s,  s,  s);
        glVertex3f(-s, -s,  s); glVertex3f(-s,  s,  s);
    glEnd();

    glPopMatrix();
}

void Scene::drawFloor() {
    if (floorTextureId != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, floorTextureId);
        glColor3f(1.0f, 1.0f, 1.0f); // White so texture shows correctly
    } else {
        glColor3f(0.5f, 0.5f, 0.5f);
    }

    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0.0f, 10.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 10.0f, 0.0f, 10.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 10.0f, 0.0f, -10.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, 0.0f, -10.0f);
    glEnd();

    if (floorTextureId != 0) {
        glDisable(GL_TEXTURE_2D);
    }
}

void Scene::drawWall() {
    if (wallTextureId != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, wallTextureId);
        glColor3f(1.0f, 1.0f, 1.0f); // White so texture shows correctly
    } else {
        glColor3f(0.7f, 0.7f, 0.7f);
    }

    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0.0f, -5.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 10.0f, 0.0f, -5.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 10.0f, 10.0f, -5.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, 10.0f, -5.0f);
    glEnd();

    if (wallTextureId != 0) {
        glDisable(GL_TEXTURE_2D);
    }
}
