#pragma once

struct TileVector {
    int x = 0;
    int y = 0;

    // ---- Constructors ----
    constexpr TileVector() noexcept = default;
    constexpr TileVector(int xx, int yy) noexcept : x(xx), y(yy) {}

    // ---- Truthiness ----
    constexpr explicit operator bool() const noexcept { return x != 0 || y != 0; }

    // ---- Comparisons ----
    friend constexpr bool operator==(const TileVector& a, const TileVector& b) noexcept {
        return a.x == b.x && a.y == b.y;
    }
    friend constexpr bool operator!=(const TileVector& a, const TileVector& b) noexcept {
        return !(a == b);
    }
    // Lexicographic comparison (useful for std::map)
    friend constexpr bool operator<(const TileVector& a, const TileVector& b) noexcept {
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
    }

    // ---- Unary ----
    friend constexpr TileVector operator+(const TileVector& v) noexcept { return v; }
    friend constexpr TileVector operator-(const TileVector& v) noexcept { return { -v.x, -v.y }; }

    // ---- Basic arithmetic ----
    friend constexpr TileVector operator+(const TileVector& a, const TileVector& b) noexcept {
        return { a.x + b.x, a.y + b.y };
    }
    friend constexpr TileVector operator-(const TileVector& a, const TileVector& b) noexcept {
        return { a.x - b.x, a.y - b.y };
    }
    TileVector& operator+=(const TileVector& r) noexcept { x += r.x; y += r.y; return *this; }
    TileVector& operator-=(const TileVector& r) noexcept { x -= r.x; y -= r.y; return *this; }

    // ---- Scalar multiply/divide ----
    friend constexpr TileVector operator*(const TileVector& v, int s) noexcept {
        return { v.x * s, v.y * s };
    }
    friend constexpr TileVector operator*(int s, const TileVector& v) noexcept {
        return v * s;
    }
    friend constexpr TileVector operator/(const TileVector& v, int s) noexcept {
        return s != 0 ? TileVector{ v.x / s, v.y / s } : v;
    }
    TileVector& operator*=(int s) noexcept { x *= s; y *= s; return *this; }
    TileVector& operator/=(int s) noexcept { if (s) { x /= s; y /= s; } return *this; }

    // ---- Component-wise min/max/clamp ----
    static constexpr TileVector min(const TileVector& a, const TileVector& b) noexcept {
        return { std::min(a.x, b.x), std::min(a.y, b.y) };
    }
    static constexpr TileVector max(const TileVector& a, const TileVector& b) noexcept {
        return { std::max(a.x, b.x), std::max(a.y, b.y) };
    }
    static constexpr TileVector clamp(const TileVector& v,
        const TileVector& lo,
        const TileVector& hi) noexcept {
        return {
            std::clamp(v.x, lo.x, hi.x),
            std::clamp(v.y, lo.y, hi.y)
        };
    }
    // clamp to a single scalar range (0..maxVal)
    static constexpr TileVector clamp(const TileVector& v, int lo, int hi) noexcept {
        return {
            std::clamp(v.x, lo, hi),
            std::clamp(v.y, lo, hi)
        };
    }
    // saturate (clamp to [0,1])
    static constexpr TileVector saturate(const TileVector& v) noexcept {
        return clamp(v, 0, 1);
    }

    // ---- Length & distances ----
    constexpr std::int64_t lengthSq() const noexcept {
        return std::int64_t(x) * x + std::int64_t(y) * y;
    }
    double length() const noexcept {
        return std::sqrt(static_cast<double>(lengthSq()));
    }
    constexpr int manhattan() const noexcept {
        return (x < 0 ? -x : x) + (y < 0 ? -y : y);
    }

    // ---- Dot / Cross / Angle ----
    friend constexpr std::int64_t dot(const TileVector& a, const TileVector& b) noexcept {
        return std::int64_t(a.x) * b.x + std::int64_t(a.y) * b.y;
    }
    friend constexpr std::int64_t crossZ(const TileVector& a, const TileVector& b) noexcept {
        return std::int64_t(a.x) * b.y - std::int64_t(a.y) * b.x;
    }
    friend double angleRad(const TileVector& a, const TileVector& b) noexcept {
        return std::atan2(static_cast<double>(crossZ(a, b)),
            static_cast<double>(dot(a, b)));
    }

    // ---- Grid rotations ----
    constexpr TileVector rot90CW()  const noexcept { return { y, -x }; }
    constexpr TileVector rot90CCW() const noexcept { return { -y, x }; }
    constexpr TileVector rot180()   const noexcept { return { -x, -y }; }

    // ---- Normalization (float) ----
    struct Float2 { float x, y; };
    Float2 normalized() const noexcept {
        double len = length();
        if (len == 0.0) return { 0.f, 0.f };
        return { static_cast<float>(x / len), static_cast<float>(y / len) };
    }

    // ---- Directional constants ----
    static constexpr TileVector Zero()  noexcept { return { 0, 0 }; }
    static constexpr TileVector One()   noexcept { return { 1, 1 }; }
    static constexpr TileVector Up()    noexcept { return { 0, -1 }; }
    static constexpr TileVector Down()  noexcept { return { 0, 1 }; }
    static constexpr TileVector Left()  noexcept { return { -1, 0 }; }
    static constexpr TileVector Right() noexcept { return { 1, 0 }; }
};

// ---- Output stream (for debugging/logging) ----
inline std::ostream& operator<<(std::ostream& os, const TileVector& v) {
    os << '(' << v.x << ',' << v.y << ')';
    return os;
}


namespace std {
	template<>
	struct hash<TileVector> {
		size_t operator()(const TileVector& loc) const noexcept {
			size_t h1 = std::hash<int>{}(loc.x);
			size_t h2 = std::hash<int>{}(loc.y);
			return h1 ^ (h2 << 1); // Combine hashes
		}
	};
}