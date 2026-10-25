// imgui_impl_kitty_render.h
// Fast multithreaded software rasterizer for Dear ImGui draw data.
#pragma once

#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
  #define KR_INLINE   inline __attribute__((always_inline))
  #define KR_RESTRICT __restrict__
  typedef uint32_t __attribute__((may_alias, aligned(4))) kr_px32;
#else
  #define KR_INLINE   __forceinline
  #define KR_RESTRICT __restrict
  typedef uint32_t kr_px32;
#endif

namespace kr {

// ------------------------------------------------------------------------------------------
// Small helpers
// ------------------------------------------------------------------------------------------
static KR_INLINE int ceil_i(float v) {
    v = v < -1e6f ? -1e6f : (v > 1e6f ? 1e6f : v);
    return (int)std::ceil(v);
}
static KR_INLINE int f2fp(float x) {            // float -> 16.16, saturating
    x *= 65536.0f;
    x = x < -2.0e9f ? -2.0e9f : (x > 2.0e9f ? 2.0e9f : x);
    return (int)x;
}
static KR_INLINE uint32_t mul255(uint32_t a, uint32_t b) {   // exact round(a*b/255)
    uint32_t t = a * b + 128u;
    return (t + (t >> 8)) >> 8;
}
static KR_INLINE int clamp255(int v) { return v < 0 ? 0 : (v > 255 ? 255 : v); }

// out = (src*a + dst*(255-a)) / 255, exact, 2 channels per 32-bit multiply. Alpha forced to 255.
static KR_INLINE uint32_t blend_px(uint32_t dst, uint32_t src, uint32_t a) {
    const uint32_t M = 0x00FF00FFu, R = 0x00800080u, inv = 255u - a;
    uint32_t rb = (src & M) * a + (dst & M) * inv + R;
    uint32_t ag = ((src >> 8) & M) * a + ((dst >> 8) & M) * inv + R;
    rb = ((rb + ((rb >> 8) & M)) >> 8) & M;
    ag = (ag + ((ag >> 8) & M)) & 0xFF00FF00u;
    return rb | ag | 0xFF000000u;
}

// Constant-color span (opaque fill or constant-alpha blend). Auto-vectorizes.
static KR_INLINE void span_solid(kr_px32* KR_RESTRICT d, int n, uint32_t src, uint32_t a) {
    if (a >= 255u) {
        const uint32_t v = src | 0xFF000000u;
        #pragma omp simd
        for (int i = 0; i < n; i++) d[i] = v;
        return;
    }
    if (a == 0u) return;
    const uint32_t M = 0x00FF00FFu, R = 0x00800080u, inv = 255u - a;
    const uint32_t srb = (src & M) * a + R;
    const uint32_t sag = ((src >> 8) & M) * a + R;
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        const uint32_t dst = d[i];
        uint32_t rb = srb + (dst & M) * inv;
        uint32_t ag = sag + ((dst >> 8) & M) * inv;
        rb = ((rb + ((rb >> 8) & M)) >> 8) & M;
        ag = (ag + ((ag >> 8) & M)) & 0xFF00FF00u;
        d[i] = rb | ag | 0xFF000000u;
    }
}

// ------------------------------------------------------------------------------------------
// Data structures
// ------------------------------------------------------------------------------------------
enum : int { K_RECT_SOLID = 0, K_RECT_TEX, K_TRI_FLAT, K_TRI_GOURAUD, K_TRI_TEX };

struct Prim {
    int kind;
    int x0, x1, y0, y1;           // clipped bounds (rect: exact; tri: clip x-range + row range)
    uint32_t src, a;              // solid: rgb + alpha.   rect tex: tint rgb + alpha
    const kr_px32* tex; int tw, th;
    uint32_t u_fp; int du_fp;     // rect tex: u (texels) at first pixel + step, 16.16
    float v0, dvdy;               // rect tex: v (texels) at center of row y0 + per row step
    int tri;                      // index into tris for triangle kinds
};

struct TriData {
    float tx, ty, mx, my, bx, by;       // vertices sorted by y (top, mid, bottom)
    float s_tb, s_tm, s_mb;             // dx/dy of edges
    float ox, oy;                       // attribute plane origin
    float base[6], ddx[6], ddy[6];      // planes: U, V (texels), R, G, B, A (0..255)
};

struct Chunk {
    const ImDrawList* list; const ImDrawCmd* cmd;
    unsigned first, count;
    int cx0, cy0, cx1, cy1;
    const kr_px32* tex; int tw, th;
    bool callback;
};

struct Ctx {
    std::vector<Chunk> chunks;
    std::vector<std::vector<Prim>> cprims;
    std::vector<std::vector<TriData>> ctris;
    std::vector<size_t> poff, toff;
    std::vector<Prim> prims;
    std::vector<TriData> tris;
    std::vector<uint32_t> bstart, bcur, bitems;
};

static Ctx& ctx() { static thread_local Ctx c; return c; }

// ------------------------------------------------------------------------------------------
// Emit stage
// ------------------------------------------------------------------------------------------
static KR_INLINE uint32_t sample_nearest(const Chunk& ch, float u, float v) {
    int tx = (int)std::floor(u * (float)ch.tw);
    int ty = (int)std::floor(v * (float)ch.th);
    tx = tx < 0 ? 0 : (tx >= ch.tw ? ch.tw - 1 : tx);
    ty = ty < 0 ? 0 : (ty >= ch.th ? ch.th - 1 : ty);
    return ch.tex[(size_t)ty * ch.tw + tx];
}

static KR_INLINE uint32_t tint_px(uint32_t t, uint32_t col) {
    return mul255(t & 255u, col & 255u) |
          (mul255((t >> 8) & 255u, (col >> 8) & 255u) << 8) |
          (mul255((t >> 16) & 255u, (col >> 16) & 255u) << 16);
}

// Returns true if the 4 verts were consumed as a rect (even if it produced nothing).
static bool emit_rect(const ImDrawVert& a, const ImDrawVert& b, const ImDrawVert& c, const ImDrawVert& d,
                      const Chunk& ch, std::vector<Prim>& out)
{
    const bool h = a.pos.y == b.pos.y && b.pos.x == c.pos.x && c.pos.y == d.pos.y && d.pos.x == a.pos.x;
    const bool v = a.pos.x == b.pos.x && b.pos.y == c.pos.y && c.pos.x == d.pos.x && d.pos.y == a.pos.y;
    if (!h && !v) return false;
    if (a.col != b.col || b.col != c.col || c.col != d.col) return false;

    const float x0 = std::min(a.pos.x, c.pos.x), x1 = std::max(a.pos.x, c.pos.x);
    const float y0 = std::min(a.pos.y, c.pos.y), y1 = std::max(a.pos.y, c.pos.y);
    if (!(x0 < x1) || !(y0 < y1)) return true;

    const bool has_tex = ch.tex != nullptr;
    float u[2] = {0, 0}, vv[2] = {0, 0};
    if (has_tex) {
        const ImDrawVert* vs[4] = {&a, &b, &c, &d};
        bool us[2] = {false, false}, vs_[2] = {false, false};
        for (int k = 0; k < 4; k++) {
            const int hx = vs[k]->pos.x == x1, hy = vs[k]->pos.y == y1;
            if (!us[hx])  { u[hx]  = vs[k]->uv.x; us[hx]  = true; } else if (u[hx]  != vs[k]->uv.x) return false;
            if (!vs_[hy]) { vv[hy] = vs[k]->uv.y; vs_[hy] = true; } else if (vv[hy] != vs[k]->uv.y) return false;
        }
    }

    Prim p{};
    p.x0 = std::max(ch.cx0, ceil_i(x0 - 0.5f)); p.x1 = std::min(ch.cx1, ceil_i(x1 - 0.5f));
    p.y0 = std::max(ch.cy0, ceil_i(y0 - 0.5f)); p.y1 = std::min(ch.cy1, ceil_i(y1 - 0.5f));
    if (p.x0 >= p.x1 || p.y0 >= p.y1) return true;

    const uint32_t col = a.col, ca = col >> 24;
    if (!has_tex) {
        if (ca == 0) return true;
        p.kind = K_RECT_SOLID; p.src = col & 0xFFFFFFu; p.a = ca;
    } else if (u[0] == u[1] && vv[0] == vv[1]) {           // white-pixel style rect
        const uint32_t t = sample_nearest(ch, u[0], vv[0]);
        p.a = mul255(ca, t >> 24);
        if (p.a == 0) return true;
        p.kind = K_RECT_SOLID; p.src = tint_px(t, col);
    } else {                                                // glyphs / images
        if (ca == 0) return true;
        const float tw = (float)ch.tw, th = (float)ch.th;
        const float dudx = (u[1] - u[0]) * tw / (x1 - x0);
        const float dvdy = (vv[1] - vv[0]) * th / (y1 - y0);
        p.kind  = K_RECT_TEX;
        p.tex = ch.tex; p.tw = ch.tw; p.th = ch.th;
        p.u_fp  = (uint32_t)f2fp(u[0] * tw + ((float)p.x0 + 0.5f - x0) * dudx);
        p.du_fp = f2fp(dudx);
        p.v0    = vv[0] * th + ((float)p.y0 + 0.5f - y0) * dvdy;
        p.dvdy  = dvdy;
        p.src   = col & 0xFFFFFFu; p.a = ca;
    }
    out.push_back(p);
    return true;
}

static void emit_tri(const ImDrawVert& v0, const ImDrawVert& v1, const ImDrawVert& v2,
                     const Chunk& ch, std::vector<Prim>& prims, std::vector<TriData>& tris)
{
    const ImVec2 &p0 = v0.pos, &p1 = v1.pos, &p2 = v2.pos;
    const float e1x = p1.x - p0.x, e1y = p1.y - p0.y, e2x = p2.x - p0.x, e2y = p2.y - p0.y;
    const float area = e1y * e2x - e1x * e2y;
    if (!(std::fabs(area) > 0.0f)) return;

    const float minY = std::min({p0.y, p1.y, p2.y}), maxY = std::max({p0.y, p1.y, p2.y});
    const float minX = std::min({p0.x, p1.x, p2.x}), maxX = std::max({p0.x, p1.x, p2.x});
    Prim p{};
    p.y0 = std::max(ch.cy0, ceil_i(minY - 0.5f)); p.y1 = std::min(ch.cy1, ceil_i(maxY - 0.5f));
    if (p.y0 >= p.y1) return;
    p.x0 = std::max(ch.cx0, ceil_i(minX - 0.5f)); p.x1 = std::min(ch.cx1, ceil_i(maxX - 0.5f) + 1);
    if (p.x0 >= p.x1) return;

    const bool has_tex = ch.tex != nullptr;
    const bool flat_col = v0.col == v1.col && v1.col == v2.col;
    const bool uvc = !has_tex || (v0.uv.x == v1.uv.x && v1.uv.x == v2.uv.x && v0.uv.y == v1.uv.y && v1.uv.y == v2.uv.y);

    uint32_t texel = 0xFFFFFFFFu;
    if (has_tex && uvc) texel = sample_nearest(ch, v0.uv.x, v0.uv.y);

    if (uvc && flat_col) {                        // flat color triangle
        p.a = mul255(v0.col >> 24, texel >> 24);
        if (p.a == 0) return;
        p.src = tint_px(texel, v0.col);
        p.kind = K_TRI_FLAT;
    } else {
        p.kind = uvc ? K_TRI_GOURAUD : K_TRI_TEX;
        p.tex = ch.tex; p.tw = ch.tw; p.th = ch.th;
    }

    TriData t;
    // y-sorted vertices
    const ImVec2 *T = &p0, *M = &p1, *B = &p2;
    if (T->y > M->y) std::swap(T, M);
    if (M->y > B->y) std::swap(M, B);
    if (T->y > M->y) std::swap(T, M);
    t.tx = T->x; t.ty = T->y; t.mx = M->x; t.my = M->y; t.bx = B->x; t.by = B->y;
    t.s_tb = B->y > T->y ? (B->x - T->x) / (B->y - T->y) : 0.0f;
    t.s_tm = M->y > T->y ? (M->x - T->x) / (M->y - T->y) : 0.0f;
    t.s_mb = B->y > M->y ? (B->x - M->x) / (B->y - M->y) : 0.0f;
    t.ox = p0.x; t.oy = p0.y;

    if (p.kind != K_TRI_FLAT) {
        const ImDrawVert* vs[3] = {&v0, &v1, &v2};
        float vals[6][3];
        const float f[4] = {(float)(texel & 255u) * (1.0f / 255.0f), (float)((texel >> 8) & 255u) * (1.0f / 255.0f),
                            (float)((texel >> 16) & 255u) * (1.0f / 255.0f), (float)(texel >> 24) * (1.0f / 255.0f)};
        for (int k = 0; k < 3; k++) {
            vals[0][k] = vs[k]->uv.x * (float)ch.tw;
            vals[1][k] = vs[k]->uv.y * (float)ch.th;
            const uint32_t c = vs[k]->col;
            vals[2][k] = (float)(c & 255u) * f[0];
            vals[3][k] = (float)((c >> 8) & 255u) * f[1];
            vals[4][k] = (float)((c >> 16) & 255u) * f[2];
            vals[5][k] = (float)(c >> 24) * f[3];
        }
        const float inv = 1.0f / area;
        for (int k = 0; k < 6; k++) {
            const float a1 = vals[k][1] - vals[k][0], a2 = vals[k][2] - vals[k][0];
            t.base[k] = vals[k][0];
            t.ddx[k]  = (a2 * e1y - a1 * e2y) * inv;
            t.ddy[k]  = (a1 * e2x - a2 * e1x) * inv;
        }
    }
    tris.push_back(t);
    p.tri = (int)tris.size() - 1;
    prims.push_back(p);
}

static void emit_range(const ImDrawVert* vtx, const ImDrawIdx* idx, unsigned count, const Chunk& ch,
                       std::vector<Prim>& P, std::vector<TriData>& T)
{
    unsigned i = 0;
    while (i + 3 <= count) {
        if (i + 6 <= count && idx[i] == idx[i + 3] && idx[i + 2] == idx[i + 4]) {   // ImGui quad: (0,1,2)(0,2,3)
            if (emit_rect(vtx[idx[i]], vtx[idx[i + 1]], vtx[idx[i + 2]], vtx[idx[i + 5]], ch, P)) { i += 6; continue; }
        }
        emit_tri(vtx[idx[i]], vtx[idx[i + 1]], vtx[idx[i + 2]], ch, P, T);
        i += 3;
    }
}

// ------------------------------------------------------------------------------------------
// Raster stage
// ------------------------------------------------------------------------------------------
template<bool TINT, bool ASCALE>
static void rect_tex_rows(const Prim& p, kr_px32* fb, int stride, int y0, int y1) {
    const int n = p.x1 - p.x0;
    const uint32_t col = p.src, ca = p.a;
    const int tw = p.tw, th = p.th;
    for (int y = y0; y < y1; y++) {
        int ty = (int)(p.v0 + (float)(y - p.y0) * p.dvdy);
        ty = ty < 0 ? 0 : (ty >= th ? th - 1 : ty);
        const kr_px32* trow = p.tex + (size_t)ty * tw;
        kr_px32* d = fb + (size_t)y * stride + p.x0;
        uint32_t u = p.u_fp;
        for (int i = 0; i < n; i++, u += (uint32_t)p.du_fp) {
            int tx = (int32_t)u >> 16;
            tx = tx < 0 ? 0 : (tx >= tw ? tw - 1 : tx);
            const uint32_t t = trow[tx];
            uint32_t a = t >> 24;
            if (ASCALE) a = mul255(a, ca);
            if (a == 0u) continue;
            const uint32_t s = TINT ? tint_px(t, col) : t;
            d[i] = (a >= 255u) ? (s | 0xFF000000u) : blend_px(d[i], s, a);
        }
    }
}

static KR_INLINE bool tri_span(const TriData& t, int y, int cx0, int cx1, int& sx, int& ex) {
    const float fy = (float)y + 0.5f;
    const float xa = t.tx + (fy - t.ty) * t.s_tb;
    const float xb = fy < t.my ? t.tx + (fy - t.ty) * t.s_tm : t.mx + (fy - t.my) * t.s_mb;
    const float xl = xa < xb ? xa : xb, xr = xa < xb ? xb : xa;
    sx = std::max(cx0, ceil_i(xl - 0.5f));
    ex = std::min(cx1, ceil_i(xr - 0.5f));
    return sx < ex;
}

template<bool TEX>
static void tri_rows(const Prim& p, const TriData& t, kr_px32* fb, int stride, int y0, int y1) {
    uint32_t step[6];
    for (int k = 0; k < 6; k++) step[k] = (uint32_t)f2fp(t.ddx[k]);
    const kr_px32* tex = p.tex; const int tw = p.tw, th = p.th;
    for (int y = y0; y < y1; y++) {
        int sx, ex;
        if (!tri_span(t, y, p.x0, p.x1, sx, ex)) continue;
        const float dy = ((float)y + 0.5f) - t.oy, dx = ((float)sx + 0.5f) - t.ox;
        uint32_t cur[6];
        for (int k = 0; k < 6; k++) cur[k] = (uint32_t)f2fp(t.base[k] + t.ddx[k] * dx + t.ddy[k] * dy);
        kr_px32* d = fb + (size_t)y * stride + sx;
        for (int x = sx; x < ex; x++, d++) {
            int a = clamp255((int32_t)cur[5] >> 16);
            if (a) {
                int r = clamp255((int32_t)cur[2] >> 16);
                int g = clamp255((int32_t)cur[3] >> 16);
                int b = clamp255((int32_t)cur[4] >> 16);
                if (TEX) {
                    int tx = (int32_t)cur[0] >> 16, ty = (int32_t)cur[1] >> 16;
                    tx = tx < 0 ? 0 : (tx >= tw ? tw - 1 : tx);
                    ty = ty < 0 ? 0 : (ty >= th ? th - 1 : ty);
                    const uint32_t tp = tex[(size_t)ty * tw + tx];
                    r = (int)mul255((uint32_t)r, tp & 255u);
                    g = (int)mul255((uint32_t)g, (tp >> 8) & 255u);
                    b = (int)mul255((uint32_t)b, (tp >> 16) & 255u);
                    a = (int)mul255((uint32_t)a, tp >> 24);
                }
                if (a) {
                    const uint32_t s = (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16);
                    *d = a >= 255 ? (s | 0xFF000000u) : blend_px(*d, s, (uint32_t)a);
                }
            }
            for (int k = 0; k < 6; k++) cur[k] += step[k];   // unused U,V steps are harmless
        }
    }
}

static void render_prim(const Prim& p, const TriData* tris, kr_px32* fb, int stride, int ylo, int yhi) {
    const int y0 = std::max(p.y0, ylo), y1 = std::min(p.y1, yhi);
    if (y0 >= y1) return;
    switch (p.kind) {
    case K_RECT_SOLID: {
        const int n = p.x1 - p.x0;
        for (int y = y0; y < y1; y++) span_solid(fb + (size_t)y * stride + p.x0, n, p.src, p.a);
    } break;
    case K_RECT_TEX: {
        const bool tint = (p.src & 0xFFFFFFu) != 0xFFFFFFu, as = p.a != 255u;
        if (tint) { if (as) rect_tex_rows<true,  true >(p, fb, stride, y0, y1); else rect_tex_rows<true,  false>(p, fb, stride, y0, y1); }
        else      { if (as) rect_tex_rows<false, true >(p, fb, stride, y0, y1); else rect_tex_rows<false, false>(p, fb, stride, y0, y1); }
    } break;
    case K_TRI_FLAT: {
        const TriData& t = tris[p.tri];
        for (int y = y0; y < y1; y++) {
            int sx, ex;
            if (tri_span(t, y, p.x0, p.x1, sx, ex)) span_solid(fb + (size_t)y * stride + sx, ex - sx, p.src, p.a);
        }
    } break;
    case K_TRI_GOURAUD: tri_rows<false>(p, tris[p.tri], fb, stride, y0, y1); break;
    case K_TRI_TEX:     tri_rows<true >(p, tris[p.tri], fb, stride, y0, y1); break;
    }
}

static int max_threads() {
#ifdef _OPENMP
    return omp_get_max_threads();
#else
    return 1;
#endif
}

static void render_segment(Ctx& c, int cb, int ce, kr_px32* fb, int W, int H) {
    const int n = ce - cb;
    if (n <= 0) return;
    const int nthreads = max_threads();

    // ---- 1. emit primitives (parallel over chunks) ----
    #pragma omp parallel for schedule(dynamic, 1) if(n > 1 && nthreads > 1)
    for (int k = cb; k < ce; k++) {
        const Chunk& ch = c.chunks[k];
        std::vector<Prim>& P = c.cprims[k]; std::vector<TriData>& T = c.ctris[k];
        P.clear(); T.clear();
        const ImDrawVert* vtx = ch.list->VtxBuffer.Data + ch.cmd->VtxOffset;
        const ImDrawIdx*  idx = ch.list->IdxBuffer.Data + ch.cmd->IdxOffset + ch.first;
        emit_range(vtx, idx, ch.count, ch, P, T);
    }

    // ---- 2. concatenate in order ----
    if (n == 1) {
        c.prims.swap(c.cprims[cb]); c.tris.swap(c.ctris[cb]);
    } else {
        c.poff.assign(n + 1, 0); c.toff.assign(n + 1, 0);
        for (int k = 0; k < n; k++) {
            c.poff[k + 1] = c.poff[k] + c.cprims[cb + k].size();
            c.toff[k + 1] = c.toff[k] + c.ctris[cb + k].size();
        }
        c.prims.resize(c.poff[n]); c.tris.resize(c.toff[n]);
        #pragma omp parallel for schedule(static) if(nthreads > 1)
        for (int k = 0; k < n; k++) {
            const std::vector<Prim>& P = c.cprims[cb + k]; const std::vector<TriData>& T = c.ctris[cb + k];
            if (!T.empty()) std::memcpy(&c.tris[c.toff[k]], T.data(), T.size() * sizeof(TriData));
            if (P.empty()) continue;
            Prim* dst = &c.prims[c.poff[k]];
            std::memcpy(dst, P.data(), P.size() * sizeof(Prim));
            const int toff = (int)c.toff[k];
            if (toff) for (size_t i = 0; i < P.size(); i++) if (dst[i].kind >= K_TRI_FLAT) dst[i].tri += toff;
        }
    }
    const size_t np = c.prims.size();
    if (np == 0) return;

    // ---- 3. render ----
    if (nthreads <= 1 || np < 32) {
        for (size_t i = 0; i < np; i++) render_prim(c.prims[i], c.tris.data(), fb, W, 0, H);
        return;
    }
    const int band = std::max(8, std::min(64, H / (nthreads * 4)));
    const int nb = (H + band - 1) / band;
    c.bstart.assign(nb + 1, 0);
    for (size_t i = 0; i < np; i++) {
        const Prim& p = c.prims[i];
        for (int b = p.y0 / band, e = (p.y1 - 1) / band; b <= e; b++) c.bstart[b + 1]++;
    }
    for (int b = 0; b < nb; b++) c.bstart[b + 1] += c.bstart[b];
    c.bitems.resize(c.bstart[nb]);
    c.bcur.assign(c.bstart.begin(), c.bstart.end() - 1);
    for (size_t i = 0; i < np; i++) {
        const Prim& p = c.prims[i];
        for (int b = p.y0 / band, e = (p.y1 - 1) / band; b <= e; b++) c.bitems[c.bcur[b]++] = (uint32_t)i;
    }
    const Prim* prims = c.prims.data(); const TriData* tris = c.tris.data();
    const uint32_t* bs = c.bstart.data(); const uint32_t* bi = c.bitems.data();
    #pragma omp parallel for schedule(dynamic, 1)
    for (int b = 0; b < nb; b++) {
        const int ylo = b * band, yhi = std::min(H, ylo + band);
        for (uint32_t k = bs[b]; k < bs[b + 1]; k++) render_prim(prims[bi[k]], tris, fb, W, ylo, yhi);
    }
}

} // namespace kr

// ------------------------------------------------------------------------------------------
// Public entry point (drop-in replacement)
// ------------------------------------------------------------------------------------------
inline void ImGui_ImplKitty_RenderDrawData(ImDrawData* draw_data, uint8_t* framebuffer, int fb_width, int fb_height) {
    using namespace kr;
    if (!framebuffer || fb_width <= 0 || fb_height <= 0) return;
    Ctx& c = ctx();
    kr_px32* fb = reinterpret_cast<kr_px32*>(framebuffer);

    // Build chunk list (callbacks act as ordering barriers).
    c.chunks.clear();
    const unsigned CHUNK = 6144;   // multiple of 6 so quads stay intact
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cl = draw_data->CmdLists[n];
        for (int ci = 0; ci < cl->CmdBuffer.Size; ci++) {
            const ImDrawCmd* pcmd = &cl->CmdBuffer[ci];
            Chunk ch{};
            ch.list = cl; ch.cmd = pcmd;
            if (pcmd->UserCallback) {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) continue;
                ch.callback = true;
                c.chunks.push_back(ch);
                continue;
            }
            ch.cx0 = std::max((int)pcmd->ClipRect.x, 0);
            ch.cy0 = std::max((int)pcmd->ClipRect.y, 0);
            ch.cx1 = std::min((int)pcmd->ClipRect.z, fb_width);
            ch.cy1 = std::min((int)pcmd->ClipRect.w, fb_height);
            if (ch.cx0 >= ch.cx1 || ch.cy0 >= ch.cy1) continue;
            rgb_texture* t = (rgb_texture*)pcmd->GetTexID();
            if (t && t->pixels) { ch.tex = (const kr_px32*)t->pixels; ch.tw = t->width; ch.th = t->height; }
            for (unsigned off = 0; off < pcmd->ElemCount; off += CHUNK) {
                ch.first = off; ch.count = std::min(CHUNK, pcmd->ElemCount - off);
                c.chunks.push_back(ch);
            }
        }
    }
    if (c.cprims.size() < c.chunks.size()) { c.cprims.resize(c.chunks.size()); c.ctris.resize(c.chunks.size()); }

    int seg = 0;
    const int total = (int)c.chunks.size();
    for (int i = 0; i < total; i++) {
        if (!c.chunks[i].callback) continue;
        render_segment(c, seg, i, fb, fb_width, fb_height);
        c.chunks[i].cmd->UserCallback(c.chunks[i].list, c.chunks[i].cmd);
        seg = i + 1;
    }
    render_segment(c, seg, total, fb, fb_width, fb_height);
}
