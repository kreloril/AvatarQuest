#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef M_TAU
#define M_TAU 2.0f * M_PI
#endif // !M_TAU


namespace Renderer {

    void makeCircle(Vector2 center, float radius, Uint32 segments,
        Vector<Vector2>& pts, Vector<int>& idx);
    void makeArc(Vector2 center, float radius, float startDeg, float endDeg, Uint32 segments,
        Vector<Vector2>& pts, Vector<int>& idx);
    void makeCone(Vector2 center, float radius, float height, Uint32 segments,
        Vector<Vector2>& pts, Vector<int>& idx);
    void makeRomboid(Vector2 center, float radius, float height, Uint32 /*segments*/,
        Vector<Vector2>& pts, Vector<int>& idx);
    void makeEllipse(Vector2 center, float rx, float ry, Uint32 segments,
        Vector<Vector2>& pts, Vector<int>& idx);

    // Filled builders (triangle lists: i0,i1,i2, ...)
    void makeCircleFilled(Vector2 center, float radius, Uint32 segments,
        Vector<Vector2>& pts, Vector<int>& tri);
    void makeArcFilled(Vector2 center, float radius, float startDeg, float endDeg, Uint32 segments,
        Vector<Vector2>& pts, Vector<int>& tri);
    void makeConeFilled(Vector2 center, float radius, float height, Uint32 segments,
        Vector<Vector2>& pts, Vector<int>& tri);
    void makeRomboidFilled(Vector2 center, float radius, float height, Uint32 /*segments*/,
        Vector<Vector2>& pts, Vector<int>& tri);
    void makeEllipseFilled(Vector2 center, float rx, float ry, Uint32 segments,
        Vector<Vector2>& pts, Vector<int>& tri);

    // Regular polygon (N sides). Outline + filled.
    void makeRegularPolygon(Vector2 center, float radius, Uint32 sides, float rotationDeg,
        Vector<Vector2>& pts, Vector<int>& idx);
    void makeRegularPolygonFilled(Vector2 center, float radius, Uint32 sides, float rotationDeg,
        Vector<Vector2>& pts, Vector<int>& tri);

    uint32_t autoSegmentsForCircleWorld(float r, float L, uint32_t minS, uint32_t maxS);

}