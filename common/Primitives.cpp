#include "common.h"

namespace {
    constexpr float kPI = 3.14159265358979323846f;
    inline float deg2rad(float d) { return d * (kPI / 180.f); }
    inline Uint32 clampSeg(Uint32 s, Uint32 minS = 3u) { return s < minS ? minS : s; }
    inline Uint32 clampSides(Uint32 n) { return n < 3u ? 3u : n; }

    // Build polyline indices (closed=open loop to first)
    inline void buildPolylineIdx(int n, bool closed, Vector<int>& idx) {
        idx.clear();
        if (n <= 1) return;
        if (closed) { for (int i = 0; i < n; ++i) { int j = (i + 1) % n; idx.push_back(i); idx.push_back(j); } }
        else { for (int i = 0; i < n - 1; ++i) { idx.push_back(i); idx.push_back(i + 1); } }
    }
    // Triangle fan with center index 0 and rim 1..N
    inline void buildFanIdx(int rimCount, Vector<int>& tri) {
        tri.clear();
        if (rimCount < 2) return;
        tri.reserve(rimCount * 3);
        for (int i = 1; i <= rimCount - 1; ++i) { tri.push_back(0); tri.push_back(i); tri.push_back(i + 1); }
    }
    // Arc rim points inclusive end
    inline void buildArcPoints(Vector2 c, float r, float a0Deg, float a1Deg, Uint32 segs, Vector<Vector2>& pts) {
        pts.clear();
        const float a0 = deg2rad(a0Deg), a1 = deg2rad(a1Deg);
        const float span = a1 - a0; const Uint32 S = clampSeg(segs, 2u);
        pts.reserve(S + 1);
        for (Uint32 i = 0; i <= S; ++i) {
            float t = float(i) / float(S), a = a0 + span * t;
            pts.push_back({ c.x + r * std::cos(a), c.y + r * std::sin(a) });
        }
    }
}

namespace Renderer {

    // ---------- OUTLINES ----------
    void makeCircle(Vector2 c, float radius, Uint32 segments, Vector<Vector2>& pts, Vector<int>& idx) {
        const float r = std::max(radius, 0.001f);
        const uint32_t S = std::max<uint32_t>(3, segments);

        pts.clear(); pts.reserve(S);
        idx.clear(); idx.reserve(S * 2);

        // Start at -90° so a vertex points “up”; equal angular spacing
        const float a0 = -0.5f * M_PI;
        const float step = M_TAU / (float)S;

        for (uint32_t i = 0; i < S; ++i) {
            const float a = a0 + step * (float)i; // ensure (float) cast
            const float ca = std::cos(a);
            const float sa = std::sin(a);
            pts.push_back({ c.x + r * ca, c.y + r * sa });
        }

        // Closed ring as line list (i, i+1), last wraps to 0
        for (int i = 0; i < (int)S; ++i) {
            const int j = (i + 1) % (int)S;
            idx.push_back(i); idx.push_back(j);
        }
    }

    void makeArc(Vector2 c, float r, float a0Deg, float a1Deg, Uint32 segs,
        Vector<Vector2>& pts, Vector<int>& idx) {
        buildArcPoints(c, r, a0Deg, a1Deg, segs, pts);
        buildPolylineIdx((int)pts.size(), false, idx);
    }

    void makeCone(Vector2 c, float r, float h, Uint32 segs,
        Vector<Vector2>& pts, Vector<int>& idx) {
        Vector<Vector2> rim; buildArcPoints(c, r, -135.f, -45.f, segs, rim);
        Vector2 apex{ c.x, c.y - h };
        pts.clear(); pts.reserve(rim.size() + 1);
        for (auto& p : rim) pts.push_back(p);
        pts.push_back(apex);
        int N = (int)rim.size(), apexIx = (int)pts.size() - 1;
        idx.clear(); idx.reserve((N - 1) * 2 + 4);
        for (int i = 0; i < N - 1; ++i) { idx.push_back(i); idx.push_back(i + 1); }
        idx.push_back(0); idx.push_back(apexIx);
        idx.push_back(N - 1); idx.push_back(apexIx);
    }

    void makeRomboid(Vector2 c, float r, float h, Uint32, Vector<Vector2>& pts, Vector<int>& idx) {
        pts = { {c.x - r,c.y},{c.x,c.y - h},{c.x + r,c.y},{c.x,c.y + h} };
        buildPolylineIdx(4, true, idx);
    }

    void makeEllipse(Vector2 c, float rx, float ry, Uint32 segs, Vector<Vector2>& pts, Vector<int>& idx) {
        const Uint32 S = clampSeg(segs);
        pts.clear(); pts.reserve(S);
        for (Uint32 i = 0; i < S; ++i) {
            float a = (2.f * kPI) * (float(i) / float(S));
            pts.push_back({ c.x + rx * std::cos(a), c.y + ry * std::sin(a) });
        }
        buildPolylineIdx((int)pts.size(), true, idx);
    }

    // ---------- FILLED ----------
    void makeCircleFilled(Vector2 c, float r, Uint32 segs, Vector<Vector2>& pts, Vector<int>& tri) {
        const Uint32 S = clampSeg(segs);
        pts.clear(); pts.reserve(S + 1);
        pts.push_back(c);
        for (Uint32 i = 0; i < S; ++i) {
            float a = (2.f * kPI) * (float(i) / float(S));
            pts.push_back({ c.x + r * std::cos(a), c.y + r * std::sin(a) });
        }
        buildFanIdx((int)S, tri);
    }

    void makeArcFilled(Vector2 c, float r, float a0Deg, float a1Deg, Uint32 segs,
        Vector<Vector2>& pts, Vector<int>& tri) {
        Vector<Vector2> rim; buildArcPoints(c, r, a0Deg, a1Deg, segs, rim);
        pts.clear(); pts.reserve(rim.size() + 1);
        pts.push_back(c);
        for (auto& p : rim) pts.push_back(p);
        buildFanIdx((int)rim.size(), tri);
    }

    void makeConeFilled(Vector2 c, float r, float h, Uint32 segs,
        Vector<Vector2>& pts, Vector<int>& tri) {
        Vector<Vector2> rim; buildArcPoints(c, r, -135.f, -45.f, segs, rim);
        Vector2 apex{ c.x, c.y - h };
        pts.clear(); pts.reserve(rim.size() + 1);
        pts.push_back(apex);
        for (auto& p : rim) pts.push_back(p);
        buildFanIdx((int)rim.size(), tri);
    }

    void makeRomboidFilled(Vector2 c, float r, float h, Uint32, Vector<Vector2>& pts, Vector<int>& tri) {
        pts = { {c.x,c.y}, {c.x - r,c.y},{c.x,c.y - h},{c.x + r,c.y},{c.x,c.y + h} };
        tri = { 0,1,2, 0,2,3, 0,3,4, 0,4,1 };
    }

    void makeEllipseFilled(Vector2 c, float rx, float ry, Uint32 segs,
        Vector<Vector2>& pts, Vector<int>& tri) {
        const Uint32 S = clampSeg(segs);
        pts.clear(); pts.reserve(S + 1);
        pts.push_back(c);
        for (Uint32 i = 0; i < S; ++i) {
            float a = (2.f * kPI) * (float(i) / float(S));
            pts.push_back({ c.x + rx * std::cos(a), c.y + ry * std::sin(a) });
        }
        buildFanIdx((int)S, tri);
    }

    // ---------- REGULAR POLYGON ----------
    void makeRegularPolygon(Vector2 c, float r, Uint32 sides, float rotDeg,
        Vector<Vector2>& pts, Vector<int>& idx) {
        const Uint32 N = clampSides(sides);
        pts.clear(); pts.reserve(N);
        idx.clear(); idx.reserve(N * 2);
        const float start = deg2rad(rotDeg - 90.f);
        const float step = 2.f * kPI / float(N);
        for (Uint32 i = 0; i < N; ++i) {
            float a = start + step * float(i);
            pts.push_back({ c.x + r * std::cos(a), c.y + r * std::sin(a) });
        }
        buildPolylineIdx((int)N, true, idx);
    }

    void makeRegularPolygonFilled(Vector2 c, float r, Uint32 sides, float rotDeg,
        Vector<Vector2>& pts, Vector<int>& tri) {
        const Uint32 N = clampSides(sides);
        pts.clear(); pts.reserve(N + 1);
        tri.clear(); tri.reserve(N * 3);
        pts.push_back(c);
        const float start = deg2rad(rotDeg - 90.f);
        const float step = 2.f * kPI / float(N);
        for (Uint32 i = 0; i < N; ++i) {
            float a = start + step * float(i);
            pts.push_back({ c.x + r * std::cos(a), c.y + r * std::sin(a) });
        }
        buildFanIdx((int)N, tri);
        // close last: (0,N,1)
        tri.push_back(0); tri.push_back((int)N); tri.push_back(1);
    }
    uint32_t autoSegmentsForCircleWorld(float r, float L, uint32_t minS, uint32_t maxS)
    {
        r = std::max(r, 1.0f);
        L = std::max(L, 1.0f);
        const float circ = M_TAU * r;
        uint32_t s = (uint32_t)std::ceil(circ / L); // ← linear in radius
        if (s < minS) s = minS;
        if (s > maxS) s = maxS;
        return s;
    }
}




