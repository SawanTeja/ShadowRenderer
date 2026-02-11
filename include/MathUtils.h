#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <cmath>
#include <cstring>
#include <iostream>
#include <GL/gl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper to replace gluPerspective and gluLookAt
namespace MathGL {

    inline void perspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar) {
        GLdouble f = 1.0 / tan(fovy * M_PI / 360.0);
        GLdouble m[16];
        
        memset(m, 0, sizeof(m));
        
        m[0] = f / aspect;
        m[5] = f;
        m[10] = (zFar + zNear) / (zNear - zFar);
        m[11] = -1.0;
        m[14] = (2.0 * zFar * zNear) / (zNear - zFar);
        
        glMultMatrixd(m);
    }

    inline void lookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
                       GLdouble centerX, GLdouble centerY, GLdouble centerZ,
                       GLdouble upX, GLdouble upY, GLdouble upZ) {
        
        GLdouble f[3], s[3], u[3];
        GLdouble m[16];
        
        f[0] = centerX - eyeX;
        f[1] = centerY - eyeY;
        f[2] = centerZ - eyeZ;
        
        GLdouble len = sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
        if (len == 0) len = 1;
        f[0] /= len; f[1] /= len; f[2] /= len;
        
        // s = f x up
        s[0] = f[1]*upZ - f[2]*upY;
        s[1] = f[2]*upX - f[0]*upZ;
        s[2] = f[0]*upY - f[1]*upX;
        
        len = sqrt(s[0]*s[0] + s[1]*s[1] + s[2]*s[2]);
        if (len == 0) len = 1;
        s[0] /= len; s[1] /= len; s[2] /= len;
        
        // u = s x f
        u[0] = s[1]*f[2] - s[2]*f[1];
        u[1] = s[2]*f[0] - s[0]*f[2];
        u[2] = s[0]*f[1] - s[1]*f[0];
        
        m[0] = s[0]; m[4] = s[1]; m[8] = s[2]; m[12] = 0.0;
        m[1] = u[0]; m[5] = u[1]; m[9] = u[2]; m[13] = 0.0;
        m[2] = -f[0]; m[6] = -f[1]; m[10] = -f[2]; m[14] = 0.0;
        m[3] = 0.0; m[7] = 0.0; m[11] = 0.0; m[15] = 1.0;
        
        glMultMatrixd(m);
        glTranslated(-eyeX, -eyeY, -eyeZ);
    }
}

struct Vector3 {
    float x, y, z;
    
    Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    
    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    
    Vector3 normalize() const {
        float len = length();
        if (len > 0) return *this * (1.0f / len);
        return *this;
    }
};

struct Matrix4 {
    float m[16];

    Matrix4() {
        std::memset(m, 0, sizeof(m));
    }

    static Matrix4 identity() {
        Matrix4 res;
        res.m[0] = 1; res.m[5] = 1; res.m[10] = 1; res.m[15] = 1;
        return res;
    }
    
    // Create a shadow matrix on the plane Y=0 from a light source
    static Matrix4 shadow(Vector3 lightPos, float planeY = 0.0f) {
        Matrix4 mat;
        float ly = lightPos.y;
        float d = ly - planeY;
        
        mat.m[0] = d;   mat.m[4] = -lightPos.x; mat.m[8] = 0;      mat.m[12] = 0;
        mat.m[1] = 0;   mat.m[5] = 0;           mat.m[9] = 0;      mat.m[13] = 0;
        mat.m[2] = 0;   mat.m[6] = -lightPos.z; mat.m[10] = d;     mat.m[14] = 0;
        mat.m[3] = 0;   mat.m[7] = -1;          mat.m[11] = 0;     mat.m[15] = d; // Homogeneous coordinate
        
        return mat;
    }
    
    const float* data() const { return m; }
};

// Unproject screen coordinates to world coordinates on a horizontal plane (Y=planeY).
// screenX, screenY are normalized [0,1] (top-left origin).
// Returns the world XZ position where the ray hits the plane.
inline bool unprojectScreenToFloor(float screenX, float screenY,
                                    float planeY, int viewportW, int viewportH,
                                    float eyeX, float eyeY, float eyeZ,
                                    float targetX, float targetY, float targetZ,
                                    float& outX, float& outZ) {
    // Camera parameters
    float fovY = 45.0f;
    float aspect = (float)viewportW / (float)viewportH;

    // Convert screen coords (0..1, top-left origin) to NDC (-1..1, bottom-left origin)
    float ndcX = screenX * 2.0f - 1.0f;   // -1 (left) to +1 (right)
    float ndcY = 1.0f - screenY * 2.0f;    // -1 (bottom) to +1 (top), flip Y

    // Compute camera basis vectors (same as lookAt with target, up (0,1,0))
    // Forward = normalize(target - eye)
    float fx = targetX - eyeX, fy = targetY - eyeY, fz = targetZ - eyeZ;
    float fLen = std::sqrt(fx*fx + fy*fy + fz*fz);
    fx /= fLen; fy /= fLen; fz /= fLen;

    // Right = normalize(forward x up)
    float rx = fy * 0.0f - fz * 1.0f;
    float ry = fz * 0.0f - fx * 0.0f;
    float rz = fx * 1.0f - fy * 0.0f;
    float rLen = std::sqrt(rx*rx + ry*ry + rz*rz);
    rx /= rLen; ry /= rLen; rz /= rLen;

    // Up = right x forward
    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;

    // Half-heights of the near plane (in world units at distance 1)
    float halfH = std::tan(fovY * (float)M_PI / 360.0f);
    float halfW = halfH * aspect;

    // Ray direction in world space
    float dirX = fx + ndcX * halfW * rx + ndcY * halfH * ux;
    float dirY = fy + ndcX * halfW * ry + ndcY * halfH * uy;
    float dirZ = fz + ndcX * halfW * rz + ndcY * halfH * uz;

    // Normalize
    float dLen = std::sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ);
    dirX /= dLen; dirY /= dLen; dirZ /= dLen;

    // Ray-plane intersection: eye + t * dir, plane Y = planeY
    // eyeY + t * dirY = planeY  =>  t = (planeY - eyeY) / dirY
    if (std::fabs(dirY) < 1e-6f) return false; // Ray parallel to plane
    float t = (planeY - eyeY) / dirY;
    if (t < 0) return false; // Intersection behind camera

    outX = eyeX + t * dirX;
    outZ = eyeZ + t * dirZ;
    return true;
}

// Build a ray (origin + direction) from normalized screen coords.
inline void buildScreenRay(float screenX, float screenY,
                           int viewportW, int viewportH,
                           float eyeX, float eyeY, float eyeZ,
                           float targetX, float targetY, float targetZ,
                           float outOrigin[3], float outDir[3]) {
    float fovY = 45.0f;
    float aspect = (float)viewportW / (float)viewportH;

    float ndcX = screenX * 2.0f - 1.0f;
    float ndcY = 1.0f - screenY * 2.0f;

    // Camera basis vectors (target = origin, up = Y)
    float fx = targetX - eyeX, fy = targetY - eyeY, fz = targetZ - eyeZ;
    float fLen = std::sqrt(fx*fx + fy*fy + fz*fz);
    fx /= fLen; fy /= fLen; fz /= fLen;

    float rx = fy * 0.0f - fz * 1.0f;
    float ry = fz * 0.0f - fx * 0.0f;
    float rz = fx * 1.0f - fy * 0.0f;
    float rLen = std::sqrt(rx*rx + ry*ry + rz*rz);
    rx /= rLen; ry /= rLen; rz /= rLen;

    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;

    float halfH = std::tan(fovY * (float)M_PI / 360.0f);
    float halfW = halfH * aspect;

    outOrigin[0] = eyeX; outOrigin[1] = eyeY; outOrigin[2] = eyeZ;

    float dx = fx + ndcX * halfW * rx + ndcY * halfH * ux;
    float dy = fy + ndcX * halfW * ry + ndcY * halfH * uy;
    float dz = fz + ndcX * halfW * rz + ndcY * halfH * uz;
    float dLen = std::sqrt(dx*dx + dy*dy + dz*dz);
    outDir[0] = dx / dLen; outDir[1] = dy / dLen; outDir[2] = dz / dLen;
}

// Slab-based ray-AABB intersection test.
// Returns true if the ray hits the box, and writes the parametric distance to outT.
inline bool rayIntersectsAABB(const float origin[3], const float dir[3],
                              float minX, float minY, float minZ,
                              float maxX, float maxY, float maxZ,
                              float& outT) {
    float tmin = -1e30f, tmax = 1e30f;
    float bounds[2][3] = {{minX, minY, minZ}, {maxX, maxY, maxZ}};

    for (int i = 0; i < 3; i++) {
        if (std::fabs(dir[i]) < 1e-8f) {
            // Ray parallel to slab
            if (origin[i] < bounds[0][i] || origin[i] > bounds[1][i])
                return false;
        } else {
            float invD = 1.0f / dir[i];
            float t0 = (bounds[0][i] - origin[i]) * invD;
            float t1 = (bounds[1][i] - origin[i]) * invD;
            if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; }
            if (t0 > tmin) tmin = t0;
            if (t1 < tmax) tmax = t1;
            if (tmin > tmax) return false;
        }
    }
    if (tmax < 0) return false;
    outT = (tmin >= 0) ? tmin : tmax;
    return true;
}

#endif // MATHUTILS_H
