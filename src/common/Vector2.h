#pragma once
struct Vector2 {
    float x, y;
    Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    // -------- existing helpers (kept) --------
    int toIntX() const { return static_cast<int>(std::round(x)); }
    int toIntY() const { return static_cast<int>(std::round(y)); }

    Vector2 operator+(const Vector2& o) const { return { x + o.x, y + o.y }; }
    Vector2 operator-(const Vector2& o) const { return { x - o.x, y - o.y }; }
    Vector2 operator*(float s) const { return { x * s, y * s }; }
    Vector2 operator/(float s) const { return (s != 0.0f) ? Vector2(x / s, y / s) : Vector2(0, 0); }

    bool operator==(const Vector2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vector2& o) const { return !(*this == o); }

    Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
    Vector2& operator-=(const Vector2& o) { x -= o.x; y -= o.y; return *this; }
    Vector2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vector2& operator/=(float s) { if (s != 0.0f) { x /= s; y /= s; } return *this; }

    float length() const { return std::sqrt(x * x + y * y); }
    float dot(const Vector2& o) const { return x * o.x + y * o.y; }
    float cross(const Vector2& o) const { return x * o.y - y * o.x; }
    Vector2 perpendicular() const { return { -y, x }; }

    Vector2 normalized() const {
        float L = length();
        return (L > 0.0f) ? Vector2(x / L, y / L) : Vector2(0, 0);
    }

    Vector2 floor() const { return { std::floor(x), std::floor(y) }; }
    Vector2 ceil()  const { return { std::ceil(x),  std::ceil(y) }; }
    Vector2 round() const { return { std::round(x), std::round(y) }; }
    Vector2 abs()   const { return { std::fabs(x),  std::fabs(y) }; }

    Vector2 operator-() const { return { -x, -y }; }
    bool isZero() const { return x == 0.0f && y == 0.0f; }

    bool isEqual(const Vector2& o, float eps = 1e-6f) const {
        return std::fabs(x - o.x) < eps && std::fabs(y - o.y) < eps;
    }
    bool isNotEqual(const Vector2& o, float eps = 1e-6f) const { return !isEqual(o, eps); }

    // -------- NEW/IMPROVED: core vector utilities --------

    // Squared length (avoids sqrt when you just compare distances)
    float lengthSq() const { return x * x + y * y; }

    // Distance helpers
    static float distance(const Vector2& a, const Vector2& b) { return (a - b).length(); }
    static float distanceSq(const Vector2& a, const Vector2& b) { return (a - b).lengthSq(); }

    // Angle to another point (degrees)
    float angleToDeg(const Vector2& other) const {
        Vector2 d = other - *this;
        return std::atan2(d.y, d.x) * (180.0f / 3.14159265358979323846f);
    }
    // Angle to another point (radians)
    float angleToRad(const Vector2& other) const {
        Vector2 d = other - *this;
        return std::atan2(d.y, d.x);
    }

    // Rotate THIS point around pivot by degrees/radians (returns a new vector)
    Vector2 rotatedDeg(float deg, const Vector2& pivot = Vector2()) const {
        float rad = deg * (3.14159265358979323846f / 180.0f);
        return rotatedRad(rad, pivot);
    }
    Vector2 rotatedRad(float rad, const Vector2& pivot = Vector2()) const {
        float c = std::cos(rad), s = std::sin(rad);
        float tx = x - pivot.x, ty = y - pivot.y;
        return { pivot.x + tx * c - ty * s, pivot.y + tx * s + ty * c };
    }

    // Snap this vector to a grid (uniform or anisotropic)
    Vector2 snapped(float spacing) const {
        if (std::fabs(spacing) < 1e-6f) spacing = 1.0f;
        return { std::round(x / spacing) * spacing, std::round(y / spacing) * spacing };
    }
    Vector2 snapped(const Vector2& spacing) const {
        float sx = (std::fabs(spacing.x) < 1e-6f) ? 1.0f : spacing.x;
        float sy = (std::fabs(spacing.y) < 1e-6f) ? 1.0f : spacing.y;
        return { std::round(x / sx) * sx, std::round(y / sy) * sy };
    }

    // Within radius (bool)
    bool withinRadius(const Vector2& p, float radius) const {
        return Vector2::distanceSq(*this, p) <= radius * radius;
    }

    // -------- Rect / geometry helpers (unchanged API, cleaned) --------

    static Vector2 centerOfRect(const Vector2 rectPos, const Vector2 rectSize) {
        return { rectPos.x + rectSize.x * 0.5f, rectPos.y + rectSize.y * 0.5f };
    }

    // NOTE: depends on farmMin/farmMax in your codebase.
    static Vector2 clamp(Vector2 vecA, Vector2 vecB) {
        return { farmMin(vecA.x, vecB.x), farmMin(vecA.y, vecB.y) };
    }

    bool intersectsWithRect(const Vector2 rectPosA, const Vector2 rectSizeA, float& distance) const {
        float leftA = rectPosA.x, rightA = rectPosA.x + rectSizeA.x;
        float topA = rectPosA.y, bottomA = rectPosA.y + rectSizeA.y;
        if (x >= leftA && x <= rightA && y >= topA && y <= bottomA) {
            distance = farmMin(rightA, x) - farmMax(leftA, x);
            return true;
        }
        return false;
    }

    static bool intrsectsWithRect(const Vector2 posA, const Vector2 sizeA,
        const Vector2 posB, const Vector2 sizeB, float& distance)
    {
        float leftA = posA.x, rightA = posA.x + sizeA.x, topA = posA.y, bottomA = posA.y + sizeA.y;
        float leftB = posB.x, rightB = posB.x + sizeB.x, topB = posB.y, bottomB = posB.y + sizeB.y;
        if (leftA < rightB && rightA > leftB && topA < bottomB && bottomA > topB) {
            distance = farmMin(rightA, rightB) - farmMax(leftA, leftB);
            return true;
        }
        return false;
    }

    static bool rayIntersectsWithRect(const Vector2& rayStart, const Vector2& rayEnd,
        const Vector2& rectPos, const Vector2& rectSize,
        float& intersectionDistance)
    {
        Vector2 dir = rayEnd - rayStart;
        const float EPS = 1e-6f;
        float tMin = 0.0f, tMax = 1.0f;

        float rectMinX = rectPos.x, rectMaxX = rectPos.x + rectSize.x;
        float rectMinY = rectPos.y, rectMaxY = rectPos.y + rectSize.y;

        if (std::fabs(dir.x) < EPS && std::fabs(dir.y) < EPS) return false;

        if (std::fabs(dir.x) >= EPS) {
            float t1 = (rectMinX - rayStart.x) / dir.x;
            float t2 = (rectMaxX - rayStart.x) / dir.x;
            if (t1 > t2) std::swap(t1, t2);
            tMin = farmMax(tMin, t1);
            tMax = farmMin(tMax, t2);
            if (tMin > tMax || tMax < 0.0f) return false;
        }
        if (std::fabs(dir.y) >= EPS) {
            float t1 = (rectMinY - rayStart.y) / dir.y;
            float t2 = (rectMaxY - rayStart.y) / dir.y;
            if (t1 > t2) std::swap(t1, t2);
            tMin = farmMax(tMin, t1);
            tMax = farmMin(tMax, t2);
            if (tMin > tMax || tMax < 0.0f) return false;
        }
        intersectionDistance = tMin; // earliest intersection on [0,1]
        return true;
    }

    static bool rayIntersectWithLine(const Vector2& rayStart, const Vector2& rayEnd,
        const Vector2& lineStart, const Vector2& lineEnd,
        float& intersectionDistance)
    {
        Vector2 rayDir = rayEnd - rayStart;
        Vector2 lineDir = lineEnd - lineStart;
        float det = rayDir.x * lineDir.y - rayDir.y * lineDir.x;
        if (std::fabs(det) < 1e-6f) return false; // parallel or coincident

        float t = ((lineStart.x - rayStart.x) * lineDir.y - (lineStart.y - rayStart.y) * lineDir.x) / det;
        float u = ((lineStart.x - rayStart.x) * rayDir.y - (lineStart.y - rayStart.y) * rayDir.x) / det;

        if (t >= 0.0f && u >= 0.0f && u <= 1.0f) { intersectionDistance = t; return true; }
        return false;
    }

    // Create a line of points from start→end with offset and "scale" spacing (x used as step)
    static Vector<Vector2> createLine(const Vector2& start, const Vector2& end,
        const Vector2& offset, const Vector2& scale)
    {
        Vector<Vector2> pts;
        Vector2 dir = end - start;
        float L = dir.length();
        if (L <= 0.0f) return pts;
        dir = dir * (1.0f / L);

        int num = std::max(1, static_cast<int>(L / std::max(1e-6f, scale.x)));
        for (int i = 0; i <= num; ++i) {
            float t = (float)i / (float)num;
            Vector2 p = start + dir * (L * t);
            pts.push_back({ p.x + offset.x, p.y + offset.y });
        }
        return pts;
    }

    // ------- Backward-compat shims (optional) -------
    // Old name had typos/return type; keep shims if existing code calls them
    float withinRadias(const Vector2& position, float radius) const { return withinRadius(position, radius); }
    Vector2 rotate(float angleDeg, const Vector2 pivot) const { return rotatedDeg(angleDeg, pivot); }

    inline Vector2 rotatedAround(const Vector2& pivot, float deg) const {
        const float r = deg * 0.01745329251994329577f; // pi/180
        const float s = std::sin(r), c = std::cos(r);
        const float dx = x - pivot.x, dy = y - pivot.y;
        return { pivot.x + dx * c - dy * s, pivot.y + dx * s + dy * c };
    }

    // Angle (deg) between two vectors a and b, using atan2; signed CCW
    static inline float angleDegBetween(const Vector2& a, const Vector2& b) {
        float ang = std::atan2(b.y, b.x) - std::atan2(a.y, a.x);
        return ang * 57.29577951308232f; // 180/pi
    }

    // If you don’t already have dot/len2, keep these tiny helpers here too:
    static inline float dot(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }
    inline float len2() const { return x * x + y * y; }


};