#ifndef SHADOW_SYSTEM_H
#define SHADOW_SYSTEM_H

#include "MathUtils.h"
#include <vector>
#include <functional>
#include <GL/gl.h>

class ShadowSystem {
public:
    ShadowSystem();
    ~ShadowSystem();

    // Render shadows using the stencil buffer technique
    // lightPos: Position of the light source
    // drawOccluders: Callback function to draw all objects that cast shadows
    void renderShadows(const Vector3& lightPos, const std::function<void()>& drawOccluders);

private:
};

#endif // SHADOW_SYSTEM_H
