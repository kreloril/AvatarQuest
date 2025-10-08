#include "Common.h"

namespace Util {

    void writeVec2(Util::BinStream& w, const Vector2& v) {
        w.writeF32(v.x); w.writeF32(v.y);
    }

    bool readVec2(Util::BinStream& r, Vector2& v) {
        return r.readF32(v.x) && r.readF32(v.y);
    }
    
}