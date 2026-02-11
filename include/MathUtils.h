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

#endif // MATHUTILS_H
