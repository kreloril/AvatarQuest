#include "common.h"

namespace TileMap {

    enum struct MapSegmentLocation {
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

    struct SegmentSplitConfig {
        int   maxTilesPerLeaf = 2048;   // stop when ? this many tiles (estimated)
        int   maxDepth = 4;       // hard cap on recursion
        float minLeafPxW = 128.0f;  // do not split below this width
        float minLeafPxH = 128.0f;  // do not split below this height
        float maxLeafFracOfVP = 0.5f;    // leaf w/h ? fraction of viewport (0 = disable)
    };

    struct MapSegmentNode {
        SDL_FRect rect{};                   // world-space, tile-aligned
        int child[4] = { -1, -1, -1, -1 };  // TL, TR, BL, BR indices into TileMap::nodes
        int parent = -1;
        uint16_t depth = 0;
        bool isLeaf = true;

        // Render payload for leaves:
        Vector<TileMap::TileTransform> visibleTiles;
    };

    struct TileMapData {
        TileVector mapSize;   // in tiles
        TileVector tileSize;  // in pixels
        SDL_FRect viewPort; // w/h = constraint; x/y set per-frame from player/camera

        VectorRef<TileMap::Tile> tiles;
        int tileCount = 0;
        Vector<int> mapData;

        // NEW: Quadtree storage + four roots (one per quadrant)
        Vector<MapSegmentNode> nodes;               // node arena
        UMap<MapSegmentLocation, int> roots;        // root node index per quadrant
		Vector<TileMap::TileTransform> visibleTiles; // current frames visible tiles
        // NEW: split tunables
        SegmentSplitConfig splitConfig;
    };

    int g_nextMapId = 0;
    UMap<int, Ref<TileMapData>> g_tileMaps;

    // ---------- math / utility ----------
    static inline bool intersects(const SDL_FRect& a, const SDL_FRect& b) {
        return !(a.x + a.w <= b.x || b.x + b.w <= a.x ||
            a.y + a.h <= b.y || b.y + b.h <= a.y);
    }

    static inline float aligned_floor(float v, float step) {
        return (step <= 0.0f) ? v : std::floor(v / step) * step;
    }
    static inline float aligned_ceil(float v, float step) {
        return (step <= 0.0f) ? v : std::ceil(v / step) * step;
    }

    static void clamp_to_map(SDL_FRect& r, float mapWpx, float mapHpx) {
        float x2 = std::clamp(r.x + r.w, 0.0f, mapWpx);
        float y2 = std::clamp(r.y + r.h, 0.0f, mapHpx);
        r.x = std::clamp(r.x, 0.0f, mapWpx);
        r.y = std::clamp(r.y, 0.0f, mapHpx);
        r.w = std::max(0.0f, x2 - r.x);
        r.h = std::max(0.0f, y2 - r.y);
    }

    // ---------- tile range ----------
    static void rect_to_tile_range(const SDL_FRect& r, const TileVector& tileSz,
        int mapW, int mapH, int& tx0, int& ty0, int& tx1, int& ty1)
    {
        const float tw = std::max(1.0f, (float)tileSz.x), th = std::max(1.0f, (float)tileSz.y);
        tx0 = int(std::floor(r.x / tw));
        ty0 = int(std::floor(r.y / th));
        tx1 = int(std::ceil((r.x + r.w) / tw)) - 1;
        ty1 = int(std::ceil((r.y + r.h) / th)) - 1;
        tx0 = std::clamp(tx0, 0, mapW - 1);
        ty0 = std::clamp(ty0, 0, mapH - 1);
        tx1 = std::clamp(tx1, 0, mapW - 1);
        ty1 = std::clamp(ty1, 0, mapH - 1);
    }

    static int approx_tile_count(const SDL_FRect& r, const TileVector& tileSz, int mapW, int mapH) {
        int tx0, ty0, tx1, ty1;
        rect_to_tile_range(r, tileSz, mapW, mapH, tx0, ty0, tx1, ty1);
        return (tx1 < tx0 || ty1 < ty0) ? 0 : (tx1 - tx0 + 1) * (ty1 - ty0 + 1);
    }

    // ---------- split criteria ----------
    static bool should_split(const SDL_FRect& rect, const SDL_FRect& vp, const TileVector& tileSz,
        int mapW, int mapH, const SegmentSplitConfig& cfg, int depth)
    {
        if (depth >= cfg.maxDepth) return false;
        if (rect.w <= cfg.minLeafPxW || rect.h <= cfg.minLeafPxH) return false;

        if (cfg.maxLeafFracOfVP > 0.0f) {
            if (rect.w > vp.w * cfg.maxLeafFracOfVP || rect.h > vp.h * cfg.maxLeafFracOfVP)
                return true;
        }
        return approx_tile_count(rect, tileSz, mapW, mapH) > cfg.maxTilesPerLeaf;
    }

    // ---------- rectangle split (quadtree) ----------
    static void split_rect(const SDL_FRect& r, const TileVector& tileSz, SDL_FRect out[4]) {
        const float tw = std::max(1.0f, (float)tileSz.x), th = std::max(1.0f, (float)tileSz.y);
        const float mx = aligned_floor(r.x + r.w * 0.5f, tw);
        const float my = aligned_floor(r.y + r.h * 0.5f, th);
        out[0] = { r.x, r.y,                std::max(0.0f, mx - r.x),         std::max(0.0f, my - r.y) }; // TL
        out[1] = { mx,  r.y,                std::max(0.0f, (r.x + r.w) - mx),    std::max(0.0f, my - r.y) }; // TR
        out[2] = { r.x, my,                 std::max(0.0f, mx - r.x),          std::max(0.0f, (r.y + r.h) - my) }; // BL
        out[3] = { mx,  my,                 std::max(0.0f, (r.x + r.w) - mx),    std::max(0.0f, (r.y + r.h) - my) }; // BR
    }

    // ---------- emit tiles for a leaf rect ----------
    static void emit_tiles_for_rect(const SDL_FRect& r, const TileVector& tileSz, const TileVector& mapSz,
        const VectorRef<TileMap::Tile>& tiles, const Vector<int>& mapData,
        Vector<TileMap::TileTransform>& out)
    {
        const int mapW = (int)mapSz.x, mapH = (int)mapSz.y;
        if (mapW <= 0 || mapH <= 0 || (int)mapData.size() < mapW * mapH) return;

        int tx0, ty0, tx1, ty1;
        rect_to_tile_range(r, tileSz, mapW, mapH, tx0, ty0, tx1, ty1);
        const float tw = std::max(1.0f, (float)tileSz.x), th = std::max(1.0f, (float)tileSz.y);

        for (int ty = ty0; ty <= ty1; ++ty) {
            const int base = ty * mapW;
            for (int tx = tx0; tx <= tx1; ++tx) {
                const int tIdx = mapData[base + tx];
                if (tIdx < 0 || tIdx >= (int)tiles.size()) continue;

                TileMap::TileTransform tr;
                tr.position = { tx * tw, ty * th }; // world px
                tr.scale = { 1.0f, 1.0f };
                tr.rotation = 0.0f;
                tr.tileIndex = tIdx;
                out.push_back(tr);
            }
        }
    }

    static int new_node(TileMapData& m, const SDL_FRect& r, int parent, uint16_t depth) {
        MapSegmentNode n;
        n.rect = r; n.parent = parent; n.depth = depth; n.isLeaf = true;
        m.nodes.push_back(std::move(n));
        return (int)m.nodes.size() - 1;
    }

    // Recursive build within current viewport
    static void build_quadtree_recursive(TileMapData& m, int nodeIdx)
    {
        MapSegmentNode& node = m.nodes[nodeIdx];
        const int mapW = (int)m.mapSize.x, mapH = (int)m.mapSize.y;

        if (!intersects(node.rect, m.viewPort)) {
            node.visibleTiles.clear(); // outside view; keep leaf but empty
            return;
        }

        if (!should_split(node.rect, m.viewPort, m.tileSize, mapW, mapH, m.splitConfig, node.depth)) {
            node.isLeaf = true;
            node.visibleTiles.clear();
            emit_tiles_for_rect(node.rect, m.tileSize, m.mapSize, m.tiles, m.mapData, node.visibleTiles);
            return;
        }

        // Split into children
        SDL_FRect childR[4]; split_rect(node.rect, m.tileSize, childR);
        const float tw = std::max(1.0f, (float)m.tileSize.x), th = std::max(1.0f, (float)m.tileSize.y);
        const float mapWpx = m.mapSize.x * tw, mapHpx = m.mapSize.y * th;

        for (int i = 0; i < 4; ++i) {
            clamp_to_map(childR[i], mapWpx, mapHpx);
            if (childR[i].w <= 0 || childR[i].h <= 0) { node.child[i] = -1; continue; }
            node.child[i] = new_node(m, childR[i], nodeIdx, node.depth + 1);
            build_quadtree_recursive(m, node.child[i]);
        }

        node.isLeaf = false;
        node.visibleTiles.clear(); // payload lives in leaves
    }

    static inline void updateViewportFromPlayer(TileMapData& m, const Vector2& playerPos)
    {
        const float halfW = m.viewPort.w * 0.5f;
        const float halfH = m.viewPort.h * 0.5f;

        const float mapWpx = m.mapSize.x * std::max(1.0f, (float)m.tileSize.x);
        const float mapHpx = m.mapSize.y * std::max(1.0f, (float)m.tileSize.y);

        m.viewPort.x = playerPos.x - halfW;
        m.viewPort.y = playerPos.y - halfH;

        // optional clamp to keep camera inside the world
        m.viewPort.x = std::clamp(m.viewPort.x, 0.0f, std::max(0.0f, mapWpx - m.viewPort.w));
        m.viewPort.y = std::clamp(m.viewPort.y, 0.0f, std::max(0.0f, mapHpx - m.viewPort.h));
    }

    void createMapSegments(Ref<TileMapData>& mref) {

		// needs to take in the account of the viewport size and position, and tile size
        if (!mref) return;
        TileMapData& m = *mref;

        m.nodes.clear();
        m.roots.clear();

        const SDL_FRect& vp = m.viewPort;
        if (vp.w <= 0 || vp.h <= 0) return;

        // 4 quadrants (TL, TR, BL, BR) from viewport
        SDL_FRect q[4] = {
            { vp.x,             vp.y,             vp.w * 0.5f, vp.h * 0.5f }, // TL
            { vp.x + vp.w * 0.5f, vp.y,             vp.w * 0.5f, vp.h * 0.5f }, // TR
            { vp.x,             vp.y + vp.h * 0.5f, vp.w * 0.5f, vp.h * 0.5f }, // BL
            { vp.x + vp.w * 0.5f, vp.y + vp.h * 0.5f, vp.w * 0.5f, vp.h * 0.5f }  // BR
        };

        const float tw = std::max(1.0f, (float)m.tileSize.x), th = std::max(1.0f,(float)m.tileSize.y);
        const float mapWpx = m.mapSize.x * tw, mapHpx = m.mapSize.y * th;

        auto align_clamp = [&](SDL_FRect& r) {
            float x0 = aligned_floor(r.x, tw), y0 = aligned_floor(r.y, th);
            float x1 = aligned_ceil(r.x + r.w, tw), y1 = aligned_ceil(r.y + r.h, th);
            r = { x0, y0, std::max(0.0f, x1 - x0), std::max(0.0f, y1 - y0) };
            clamp_to_map(r, mapWpx, mapHpx);
            };
        for (int i = 0; i < 4; ++i) align_clamp(q[i]);

        // Create root nodes & build recursively
        int rTL = new_node(m, q[0], -1, 0); build_quadtree_recursive(m, rTL);
        int rTR = new_node(m, q[1], -1, 0); build_quadtree_recursive(m, rTR);
        int rBL = new_node(m, q[2], -1, 0); build_quadtree_recursive(m, rBL);
        int rBR = new_node(m, q[3], -1, 0); build_quadtree_recursive(m, rBR);

        m.roots[MapSegmentLocation::TopLeft] = rTL;
        m.roots[MapSegmentLocation::TopRight] = rTR;
        m.roots[MapSegmentLocation::BottomLeft] = rBL;
        m.roots[MapSegmentLocation::BottomRight] = rBR;

    }


    static void gatherFromNodeRecursive(const TileMapData& m,
        int nodeIdx,
        Vector<TileTransform>& out)
    {
        if (nodeIdx < 0 || nodeIdx >= (int)m.nodes.size()) return;
        const MapSegmentNode& node = m.nodes[nodeIdx];

        // 1) Prune early if node doesn't intersect the current viewport
        if (!intersects(node.rect, m.viewPort)) return;

        // 2) Leaf: append payload
        if (node.isLeaf) {
            if (!node.visibleTiles.empty()) {
                out.insert(out.end(), node.visibleTiles.begin(), node.visibleTiles.end());
            }
            return;
        }

        // 3) Internal: recurse into children (and validate child rects vs parent in debug)
        for (int i = 0; i < 4; ++i) {
            const int ci = node.child[i];
            if (ci == -1) continue;
            gatherFromNodeRecursive(m, ci, out);
        }
    }

    // ---------- helpers ---------------------------------------------------------

// Collect all segment tiles (world-space) into a flat vector.
    static inline void gatherVisibleTilesWorld(const Ref<TileMapData>& mref,
        Vector<TileMap::TileTransform>& out)
    {
        out.clear();
        if (!mref) return;
        const TileMapData& m = *mref;

        out.reserve(4096);

        auto it = m.roots.find(MapSegmentLocation::TopLeft);
        if (it != m.roots.end()) gatherFromNodeRecursive(m, it->second, out);
        it = m.roots.find(MapSegmentLocation::TopRight);
        if (it != m.roots.end()) gatherFromNodeRecursive(m, it->second, out);
        it = m.roots.find(MapSegmentLocation::BottomLeft);
        if (it != m.roots.end()) gatherFromNodeRecursive(m, it->second, out);
        it = m.roots.find(MapSegmentLocation::BottomRight);
        if (it != m.roots.end()) gatherFromNodeRecursive(m, it->second, out);
    }

    // Convert world-space positions to screen-space (relative to current viewport)
    static inline void toScreenSpace(const Ref<TileMapData>& mref,
        Vector<TileMap::TileTransform>& list)
    {
        if (!mref) return;
        const SDL_FRect& vp = mref->viewPort;
        for (auto& t : list) {
            t.position.x -= vp.x;
            t.position.y -= vp.y;
        }
    }

    // Centers viewport on player; viewport w/h are the constraint; clamps to map.
    static inline void centerViewportOnPlayer(Ref<TileMapData>& mref, const Vector2& playerPos)
    {
        TileMapData& m = *mref;
        const float halfW = m.viewPort.w * 0.5f;
        const float halfH = m.viewPort.h * 0.5f;

        const float tw = std::max(1.0f, (float)m.tileSize.x);
        const float th = std::max(1.0f, (float)m.tileSize.y);
        const float mapWpx = m.mapSize.x * tw;
        const float mapHpx = m.mapSize.y * th;

        m.viewPort.x = playerPos.x - halfW;
        m.viewPort.y = playerPos.y - halfH;

        // Clamp to keep camera inside map bounds (optional but sane)
        m.viewPort.x = std::clamp(m.viewPort.x, 0.0f, std::max(0.0f, mapWpx - m.viewPort.w));
        m.viewPort.y = std::clamp(m.viewPort.y, 0.0f, std::max(0.0f, mapHpx - m.viewPort.h));
    }

}

bool TileMap::initMap(TileVector mapSize, TileVector tileSize, TileMap::TileModel* tiles, int tileCount, int& mapId)
{
	auto tileMap = CreateRef<TileMap::TileMapData>();
	tileMap->mapSize = mapSize;
	tileMap->tileSize = tileSize;
    
    if (!loadFromModelArray(tiles, tileCount, tileMap->tiles)) {
        return false;
    }

	tileMap->tileCount = tileCount;
	tileMap->mapData.resize(static_cast<size_t>(mapSize.x * mapSize.y), 0); // Initialize with -1 (no tile)
	g_tileMaps[++g_nextMapId] = tileMap;
	mapId = g_nextMapId;
    return true;
}

bool TileMap::setMapData(int mapId, SDL_FRect& viewPort, const Vector<int>& mapData)
{
	// Set the map data for the specified mapId
	auto ir = g_tileMaps.find(mapId);
    if (ir == g_tileMaps.end()) {
        return false; // Map not found
	}
	auto& tileMap = ir->second;
	if (!tileMap) return false;
    tileMap->mapData = mapData;
	tileMap->viewPort = viewPort;

	createMapSegments(tileMap);

    return true;
}

void TileMap::setMapIndex(int mapId, TileVector& tv, int tileIndex)
{
    auto ir = g_tileMaps.find(mapId);
    if (ir == g_tileMaps.end()) {
        return; // Map not found
    }
    auto& tileMap = ir->second;
    if (!tileMap) return;
    int mapW = tileMap->mapSize.x;
    int mapH = tileMap->mapSize.y;
    if (tv.x < 0 || tv.x >= mapW || tv.y < 0 || tv.y >= mapH) {
        
        return; // Out of bounds
    }
    int index = tv.y * mapW + tv.x;
	tileMap->mapData[index] = tileIndex;
}

void TileMap::getMapIndex(int mapId, TileVector& tv, int& tileIndex)
{
    auto ir = g_tileMaps.find(mapId);
    if (ir == g_tileMaps.end()) {
        return; // Map not found
    }
    auto& tileMap = ir->second;
    if (!tileMap) return;
    int mapW = tileMap->mapSize.x;
    int mapH = tileMap->mapSize.y;
    if (tv.x < 0 || tv.x >= mapW || tv.y < 0 || tv.y >= mapH) {
		tileIndex = -1;
        return; // Out of bounds
    }
    int index = tv.y * mapW + tv.x;
	tileIndex = tileMap->mapData[index];
}

void TileMap::getMapTiles(int mapId, Vector<Ref<TileMap::Tile>>& outTiles)
{
    auto ir = g_tileMaps.find(mapId);
    if (ir == g_tileMaps.end()) {
        return; // Map not found
    }
    auto& tileMap = ir->second;
    if (!tileMap) return;
	outTiles = tileMap->tiles;
}

bool TileMap::loadMap(TileVector mapSize, TileVector tileSize, TileMap::TileModel* tiles, int tileCount, const char* mapDataFile, int& mapId)
{
    return true;
}

bool TileMap::saveMap(const char* mapDataFile, int mapId)
{
    return true;
}

bool TileMap::updateMap(float deltaTime, Vector2& viewPosition, int& mapId)
{
    auto it = g_tileMaps.find(mapId);
    if (it == g_tileMaps.end()) return false;

    Ref<TileMapData> tileMap = it->second;
    if (!tileMap) return false;

	tileMap->visibleTiles.clear();

	centerViewportOnPlayer(tileMap, viewPosition);
	createMapSegments(tileMap);

    gatherVisibleTilesWorld(tileMap, tileMap->visibleTiles);
	updateTileSets(deltaTime, tileMap->visibleTiles, tileMap->tiles);

    return true;
}

void TileMap::renderMap(int mapId)
{
    auto it = g_tileMaps.find(mapId);
    if (it == g_tileMaps.end()) return;

    Ref<TileMapData> tileMap = it->second;
    if (!tileMap) return;

    toScreenSpace(tileMap, tileMap->visibleTiles);

    // 3) Draw in one batch using your renderer
    renderTiles(tileMap->visibleTiles, tileMap->tiles);

}



void TileMap::shutDownMap(int& mapId)
{
    auto it = g_tileMaps.find(mapId);
    if (it != g_tileMaps.end()) {
        g_tileMaps.erase(it);
    }
	mapId = -1;
}
