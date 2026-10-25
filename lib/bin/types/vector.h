#pragma once

#include <cmath>
#include <ostream>
namespace Gmeng {
    //
    // 2D Vector
    //
    struct Vec2 {
        float x, y;

        // Constructor
        Vec2(float x = 0, float y = 0) : x(x), y(y) {}

        // Vector addition
        Vec2 operator+(const Vec2& other) const {
            return Vec2(x + other.x, y + other.y);
        }

        // Vector subtraction
        Vec2 operator-(const Vec2& other) const {
            return Vec2(x - other.x, y - other.y);
        }

        // Scalar multiplication
        Vec2 operator*(float scalar) const {
            return Vec2(x * scalar, y * scalar);
        }

        // Scalar division
        Vec2 operator/(float scalar) const {
            return Vec2(x / scalar, y / scalar);
        }

        // Dot product
        float dot(const Vec2& other) const {
            return x * other.x + y * other.y;
        }

        // Length (magnitude)
        float length() const {
            return std::sqrt(dot(*this));
        }

        // Normalize to unit vector
        Vec2 normalized() const {
            float len = length();
            return len > 0 ? *this / len : Vec2();
        }
    };

    //
    // 3D Vector
    //
    struct Vec3 {
        float x, y, z;

        Vec3(float x = 0, float y = 0, float z = 0)
            : x(x), y(y), z(z) {}

        Vec3 operator+(const Vec3& other) const {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        Vec3 operator-(const Vec3& other) const {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        Vec3 operator*(float scalar) const {
            return Vec3(x * scalar, y * scalar, z * scalar);
        }

        Vec3 operator/(float scalar) const {
            return Vec3(x / scalar, y / scalar, z / scalar);
        }

        // Dot product: returns scalar
        float dot(const Vec3& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        // Cross product: returns vector perpendicular to both
        Vec3 cross(const Vec3& other) const {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }

        float length() const {
            return std::sqrt(dot(*this));
        }

        Vec3 normalized() const {
            float len = length();
            return len > 0 ? *this / len : Vec3();
        }
    };

    //
    // 4D Vector
    //
    struct Vec4 {
        float x, y, z, w;

        Vec4(float x = 0, float y = 0, float z = 0, float w = 0)
            : x(x), y(y), z(z), w(w) {}

        Vec4 operator+(const Vec4& other) const {
            return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
        }

        Vec4 operator-(const Vec4& other) const {
            return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
        }

        Vec4 operator*(float scalar) const {
            return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        Vec4 operator/(float scalar) const {
            return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
        }

        float dot(const Vec4& other) const {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        float length() const {
            return std::sqrt(dot(*this));
        }

        Vec4 normalized() const {
            float len = length();
            return len > 0 ? *this / len : Vec4();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Vec2& v) {
        return os << "Vec2(" << v.x << ", " << v.y << ")";
    }

    inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        return os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    }

    inline std::ostream& operator<<(std::ostream& os, const Vec4& v) {
        return os << "Vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    }

};
