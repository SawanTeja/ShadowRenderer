#include "ShadowSystem.h"
#include <iostream>

ShadowSystem::ShadowSystem() {}

ShadowSystem::~ShadowSystem() {}

// ================================================================
// Render shadows using stencil buffer planar projection
// ================================================================
void ShadowSystem::renderShadows(const Vector3& lightPos, const std::function<void()>& drawOccluders) {
    // 1. Disable writing to color/depth buffers, enable stencil writing
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass if stencil value is 1 (floor)
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); // Increment stencil value to 2 where shadow is
    
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    
    // Prevent z-fighting with the floor
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    // 2. Render shadow volumes (projected geometry)
    glPushMatrix();
    Matrix4 shadowMat = Matrix4::shadow(lightPos, 0.0f); // Project onto Y=0
    glMultMatrixf(shadowMat.data());

    // Clean up state for drawing shapes as flat silhouettes if needed, 
    // but usually lighting is disabled in scene render before calling this if strictly drawing shadow volume.
    // However, here we just project the geometry. Use black color just in case (though ColorMask is false).
    if (drawOccluders) {
        drawOccluders();
    }

    glPopMatrix();
    
    // 3. Render the shadow overlay
    // Enable color writing again
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    
    // Draw a semi-transparent quad where stencil value is 2 (Floor + Shadow)
    glStencilFunc(GL_EQUAL, 2, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING); // Shadows are just dark overlays
    
    glColor4f(0.0f, 0.0f, 0.0f, 0.35f); // Shadow color/intensity

    // Draw full screen quad (conceptually covering the world)
    // Since we are in 3D space, we draw a large quad over the floor.
    // The stencil test ensures it only draws on the shadowed floor areas.
    float sz = 200.0f; // Sufficiently large
    glBegin(GL_QUADS);
        glVertex3f(-sz, 0.0f, sz);
        glVertex3f( sz, 0.0f, sz);
        glVertex3f( sz, 0.0f, -sz);
        glVertex3f(-sz, 0.0f, -sz);
    glEnd();

    // Restore state
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
}
