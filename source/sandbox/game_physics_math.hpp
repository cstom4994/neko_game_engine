//
//
// http://www.tomgibara.com/computer-vision/marching-squares
// https://github.com/reunanen/cpp-marching-squares
// https://github.com/ivanfratric/polypartition
// https://gist.github.com/mieko/0275f2f4a3b18388ed5131b3364179fb

#ifndef NEKO_GAME_PM_HPP
#define NEKO_GAME_PM_HPP

#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <list>
#include <set>

#include "engine/neko.h"
#include "engine/neko.hpp"
#include "engine/neko_math.h"

namespace neko {

NEKO_INLINE bool IsValid(f32 x) { return isfinite(x); }

// NEKO_INLINE auto RectToPoint(const neko_vec2 &v) {
//     neko_vec2 a1 = {v.x / 2.0f, v.y / 2.0f};
//     neko_vec2 a2 = {v.x / -2.0f, v.y / 2.0f};
//     neko_vec2 b1 = {v.x / -2.0f, v.y / -2.0f};
//     neko_vec2 b2 = {v.x / 2.0f, v.y / -2.0f};
//     return std::initializer_list<neko_vec2>{a1, a2, b1, b2};
// }

#pragma region TPPL

typedef f64 tppl_float;

#define TPPL_CCW 1
#define TPPL_CW -1

// 2D point structure
struct TPPLPoint {
    tppl_float x;
    tppl_float y;
    // User-specified vertex identifier.  Note that this isn't used internally
    // by the library, but will be faithfully copied around.
    s32 id;

    TPPLPoint operator+(const TPPLPoint &p) const {
        TPPLPoint r;
        r.x = x + p.x;
        r.y = y + p.y;
        return r;
    }

    TPPLPoint operator-(const TPPLPoint &p) const {
        TPPLPoint r;
        r.x = x - p.x;
        r.y = y - p.y;
        return r;
    }

    TPPLPoint operator*(const tppl_float f) const {
        TPPLPoint r;
        r.x = x * f;
        r.y = y * f;
        return r;
    }

    TPPLPoint operator/(const tppl_float f) const {
        TPPLPoint r;
        r.x = x / f;
        r.y = y / f;
        return r;
    }

    bool operator==(const TPPLPoint &p) const {
        if ((x == p.x) && (y == p.y))
            return true;
        else
            return false;
    }

    bool operator!=(const TPPLPoint &p) const {
        if ((x == p.x) && (y == p.y))
            return false;
        else
            return true;
    }
};

// Polygon implemented as an array of points with a 'hole' flag
class TPPLPoly {
protected:
    TPPLPoint *points;
    s64 numpoints;
    bool hole;

public:
    // constructors/destructors
    TPPLPoly();
    ~TPPLPoly();

    TPPLPoly(const TPPLPoly &src);
    TPPLPoly &operator=(const TPPLPoly &src);

    // getters and setters
    s64 GetNumPoints() const { return numpoints; }

    bool IsHole() const { return hole; }

    void SetHole(bool hole) { this->hole = hole; }

    TPPLPoint &GetPoint(s64 i) { return points[i]; }

    const TPPLPoint &GetPoint(s64 i) const { return points[i]; }

    TPPLPoint *GetPoints() { return points; }

    TPPLPoint &operator[](s32 i) { return points[i]; }

    const TPPLPoint &operator[](s32 i) const { return points[i]; }

    // clears the polygon points
    void Clear();

    // inits the polygon with numpoints vertices
    void Init(s64 numpoints);

    // creates a triangle with points p1,p2,p3
    void Triangle(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3);

    // inverts the orfer of vertices
    void Invert();

    // returns the orientation of the polygon
    // possible values:
    //    TPPL_CCW : polygon vertices are in counter-clockwise order
    //    TPPL_CW : polygon vertices are in clockwise order
    //        0 : the polygon has no (measurable) area
    s32 GetOrientation() const;

    // sets the polygon orientation
    // orientation can be
    //    TPPL_CCW : sets vertices in counter-clockwise order
    //    TPPL_CW : sets vertices in clockwise order
    void SetOrientation(s32 orientation);

    // checks whether a polygon is valid or not
    inline bool Valid() const { return this->numpoints >= 3; }
};

typedef std::list<TPPLPoly> TPPLPolyList;

class TPPLPartition {
protected:
    struct PartitionVertex {
        bool isActive;
        bool isConvex;
        bool isEar;

        TPPLPoint p;
        tppl_float angle;
        PartitionVertex *previous;
        PartitionVertex *next;

        PartitionVertex();
    };

    struct MonotoneVertex {
        TPPLPoint p;
        s64 previous;
        s64 next;
    };

    class VertexSorter {
        MonotoneVertex *vertices;

    public:
        VertexSorter(MonotoneVertex *v) : vertices(v) {}
        bool operator()(s64 index1, s64 index2);
    };

    struct Diagonal {
        s64 index1;
        s64 index2;
    };

    typedef std::list<Diagonal> DiagonalList;

    // dynamic programming state for minimum-weight triangulation
    struct DPState {
        bool visible;
        tppl_float weight;
        s64 bestvertex;
    };

    // dynamic programming state for convex partitioning
    struct DPState2 {
        bool visible;
        s64 weight;
        DiagonalList pairs;
    };

    // edge that intersects the scanline
    struct ScanLineEdge {
        mutable s64 index;
        TPPLPoint p1;
        TPPLPoint p2;

        // determines if the edge is to the left of another edge
        bool operator<(const ScanLineEdge &other) const;

        bool IsConvex(const TPPLPoint &p1, const TPPLPoint &p2, const TPPLPoint &p3) const;
    };

    // standard helper functions
    bool IsConvex(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3);
    bool IsReflex(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3);
    bool IsInside(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3, TPPLPoint &p);

    bool InCone(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3, TPPLPoint &p);
    bool InCone(PartitionVertex *v, TPPLPoint &p);

    s32 Intersects(TPPLPoint &p11, TPPLPoint &p12, TPPLPoint &p21, TPPLPoint &p22);

    TPPLPoint Normalize(const TPPLPoint &p);
    tppl_float Distance(const TPPLPoint &p1, const TPPLPoint &p2);

    // helper functions for Triangulate_EC
    void UpdateVertexReflexity(PartitionVertex *v);
    void UpdateVertex(PartitionVertex *v, PartitionVertex *vertices, s64 numvertices);

    // helper functions for ConvexPartition_OPT
    void UpdateState(s64 a, s64 b, s64 w, s64 i, s64 j, DPState2 **dpstates);
    void TypeA(s64 i, s64 j, s64 k, PartitionVertex *vertices, DPState2 **dpstates);
    void TypeB(s64 i, s64 j, s64 k, PartitionVertex *vertices, DPState2 **dpstates);

    // helper functions for MonotonePartition
    bool PBelow(TPPLPoint &p1, TPPLPoint &p2);
    void AddDiagonal(MonotoneVertex *vertices, s64 *numvertices, s64 index1, s64 index2, char *vertextypes, std::set<ScanLineEdge>::iterator *edgeTreeIterators, std::set<ScanLineEdge> *edgeTree,
                     s64 *helpers);

    // triangulates a monotone polygon, used in Triangulate_MONO
    s32 TriangulateMonotone(TPPLPoly *inPoly, TPPLPolyList *triangles);

public:
    // simple heuristic procedure for removing holes from a list of polygons
    // works by creating a diagonal from the rightmost hole vertex to some visible vertex
    // time complexity: O(h*(n^2)), h is the number of holes, n is the number of vertices
    // space complexity: O(n)
    // params:
    //    inpolys : a list of polygons that can contain holes
    //              vertices of all non-hole polys have to be in counter-clockwise order
    //              vertices of all hole polys have to be in clockwise order
    //    outpolys : a list of polygons without holes
    // returns 1 on success, 0 on failure
    s32 RemoveHoles(TPPLPolyList *inpolys, TPPLPolyList *outpolys);

    // triangulates a polygon by ear clipping
    // time complexity O(n^2), n is the number of vertices
    // space complexity: O(n)
    // params:
    //    poly : an input polygon to be triangulated
    //           vertices have to be in counter-clockwise order
    //    triangles : a list of triangles (result)
    // returns 1 on success, 0 on failure
    s32 Triangulate_EC(TPPLPoly *poly, TPPLPolyList *triangles);

    // triangulates a list of polygons that may contain holes by ear clipping algorithm
    // first calls RemoveHoles to get rid of the holes, and then Triangulate_EC for each resulting polygon
    // time complexity: O(h*(n^2)), h is the number of holes, n is the number of vertices
    // space complexity: O(n)
    // params:
    //    inpolys : a list of polygons to be triangulated (can contain holes)
    //              vertices of all non-hole polys have to be in counter-clockwise order
    //              vertices of all hole polys have to be in clockwise order
    //    triangles : a list of triangles (result)
    // returns 1 on success, 0 on failure
    s32 Triangulate_EC(TPPLPolyList *inpolys, TPPLPolyList *triangles);

    // creates an optimal polygon triangulation in terms of minimal edge length
    // time complexity: O(n^3), n is the number of vertices
    // space complexity: O(n^2)
    // params:
    //    poly : an input polygon to be triangulated
    //           vertices have to be in counter-clockwise order
    //    triangles : a list of triangles (result)
    // returns 1 on success, 0 on failure
    s32 Triangulate_OPT(TPPLPoly *poly, TPPLPolyList *triangles);

    // triangulates a polygons by firstly partitioning it into monotone polygons
    // time complexity: O(n*log(n)), n is the number of vertices
    // space complexity: O(n)
    // params:
    //    poly : an input polygon to be triangulated
    //           vertices have to be in counter-clockwise order
    //    triangles : a list of triangles (result)
    // returns 1 on success, 0 on failure
    s32 Triangulate_MONO(TPPLPoly *poly, TPPLPolyList *triangles);

    // triangulates a list of polygons by firstly partitioning them into monotone polygons
    // time complexity: O(n*log(n)), n is the number of vertices
    // space complexity: O(n)
    // params:
    //    inpolys : a list of polygons to be triangulated (can contain holes)
    //              vertices of all non-hole polys have to be in counter-clockwise order
    //              vertices of all hole polys have to be in clockwise order
    //    triangles : a list of triangles (result)
    // returns 1 on success, 0 on failure
    s32 Triangulate_MONO(TPPLPolyList *inpolys, TPPLPolyList *triangles);

    // creates a monotone partition of a list of polygons that can contain holes
    // time complexity: O(n*log(n)), n is the number of vertices
    // space complexity: O(n)
    // params:
    //    inpolys : a list of polygons to be triangulated (can contain holes)
    //              vertices of all non-hole polys have to be in counter-clockwise order
    //              vertices of all hole polys have to be in clockwise order
    //    monotonePolys : a list of monotone polygons (result)
    // returns 1 on success, 0 on failure
    s32 MonotonePartition(TPPLPolyList *inpolys, TPPLPolyList *monotonePolys);

    // partitions a polygon into convex polygons by using Hertel-Mehlhorn algorithm
    // the algorithm gives at most four times the number of parts as the optimal algorithm
    // however, in practice it works much better than that and often gives optimal partition
    // uses triangulation obtained by ear clipping as intermediate result
    // time complexity O(n^2), n is the number of vertices
    // space complexity: O(n)
    // params:
    //    poly : an input polygon to be partitioned
    //           vertices have to be in counter-clockwise order
    //    parts : resulting list of convex polygons
    // returns 1 on success, 0 on failure
    s32 ConvexPartition_HM(TPPLPoly *poly, TPPLPolyList *parts);

    // partitions a list of polygons into convex parts by using Hertel-Mehlhorn algorithm
    // the algorithm gives at most four times the number of parts as the optimal algorithm
    // however, in practice it works much better than that and often gives optimal partition
    // uses triangulation obtained by ear clipping as intermediate result
    // time complexity O(n^2), n is the number of vertices
    // space complexity: O(n)
    // params:
    //    inpolys : an input list of polygons to be partitioned
    //              vertices of all non-hole polys have to be in counter-clockwise order
    //              vertices of all hole polys have to be in clockwise order
    //    parts : resulting list of convex polygons
    // returns 1 on success, 0 on failure
    s32 ConvexPartition_HM(TPPLPolyList *inpolys, TPPLPolyList *parts);

    // optimal convex partitioning (in terms of number of resulting convex polygons)
    // using the Keil-Snoeyink algorithm
    // M. Keil, J. Snoeyink, "On the time bound for convex decomposition of simple polygons", 1998
    // time complexity O(n^3), n is the number of vertices
    // space complexity: O(n^3)
    //    poly : an input polygon to be partitioned
    //           vertices have to be in counter-clockwise order
    //    parts : resulting list of convex polygons
    // returns 1 on success, 0 on failure
    s32 ConvexPartition_OPT(TPPLPoly *poly, TPPLPolyList *parts);
};

#pragma endregion TPPL

void simplify_section(const std::vector<neko_vec2> &pts, f32 tolerance, size_t i, size_t j, std::vector<bool> *mark_map, size_t omitted = 0);
std::vector<neko_vec2> simplify(const std::vector<neko_vec2> &vertices, f32 tolerance);
f32 pDistance(f32 x, f32 y, f32 x1, f32 y1, f32 x2, f32 y2);

//  * A simple implementation of the marching squares algorithm that can identify
//  * perimeters in an supplied byte array. The array of data over which this
//  * instances of this class operate is not cloned by this class's constructor
//  * (for obvious efficiency reasons) and should therefore not be modified while
//  * the object is in use. It is expected that the data elements supplied to the
//  * algorithm have already been thresholded. The algorithm only distinguishes
//  * between zero and non-zero values.

namespace marching_squares {
struct ms_direction {
    ms_direction() : x(0), y(0) {}
    ms_direction(s32 x, s32 y) : x(x), y(y) {}
    ms_direction(neko_vec2 vec) : x(vec.x), y(vec.y) {}
    s32 x;
    s32 y;
};

bool operator==(const ms_direction &a, const ms_direction &b);
ms_direction operator*(const ms_direction &direction, s32 multiplier);
ms_direction operator+(const ms_direction &a, const ms_direction &b);
ms_direction &operator+=(ms_direction &a, const ms_direction &b);

ms_direction make_direction(s32 x, s32 y);

bool is_set(s32 x, s32 y, s32 width, s32 height, unsigned char *data);
s32 ms_value(s32 x, s32 y, s32 width, s32 height, unsigned char *data);

struct ms_result {
    s32 initial_x = -1;
    s32 initial_y = -1;
    std::vector<ms_direction> directions;
};

/**
 * Finds the perimeter between a set of zero and non-zero values which
 * begins at the specified data element. If no initial point is known,
 * consider using the convenience method supplied. The paths returned by
 * this method are always closed.
 *
 * The length of the supplied data array must exceed width * height,
 * with the data elements in row major order and the top-left-hand data
 * element at index zero.
 *
 * @param initial_x
 *            the column of the data matrix at which to start tracing the
 *            perimeter
 * @param initial_y
 *            the row of the data matrix at which to start tracing the
 *            perimeter
 * @param width
 *            the width of the data matrix
 * @param height
 *            the width of the data matrix
 * @param data
 *            the data elements
 *
 * @return a closed, anti-clockwise path that is a perimeter of between a
 *         set of zero and non-zero values in the data.
 * @throws std::runtime_error
 *             if there is no perimeter at the specified initial point.
 */
ms_result find_perimeter(s32 initialX, s32 initialY, s32 width, s32 height, unsigned char *data);

/**
 * A convenience method that locates at least one perimeter in the data with
 * which this object was constructed. If there is no perimeter (ie. if all
 * elements of the supplied array are identically zero) then null is
 * returned.
 *
 * @return a perimeter path obtained from the data, or null
 */
ms_result find_perimeter(s32 width, s32 height, unsigned char *data);
ms_result find_perimeter(s32 width, s32 height, unsigned char *data, s32 lookX, s32 lookY);
ms_direction find_edge(s32 width, s32 height, unsigned char *data, s32 lookX, s32 lookY);
}  // namespace marching_squares

}  // namespace neko

///////////////////////////////////////////////
//
//  位图边缘检测算法

typedef struct {
    short x, y;
} neko_tex_point;

typedef s32 neko_tex_bool;

// 2d point type helpers
#define __neko_tex_point_add(result, a, b) \
    {                                      \
        (result).x = (a).x + (b).x;        \
        (result).y = (a).y + (b).y;        \
    }
#define __neko_tex_point_sub(result, a, b) \
    {                                      \
        (result).x = (a).x - (b).x;        \
        (result).y = (a).y - (b).y;        \
    }
#define __neko_tex_point_is_inside(a, w, h) ((a).x >= 0 && (a).y >= 0 && (a).x < (w) && (a).y < (h))
#define __neko_tex_point_is_next_to(a, b) ((a).x - (b).x <= 1 && (a).x - (b).x >= -1 && (a).y - (b).y <= 1 && (a).y - (b).y >= -1)

// direction type
typedef s32 neko_tex_direction;  // 8 cw directions: >, _|, v, |_, <, |", ^, "|
#define __neko_tex_direction_opposite(dir) ((dir + 4) & 7)
static const neko_tex_point neko_tex_direction_to_pixel_offset[] = {{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};

// image manipulation functions
u8 *neko_tex_rgba_to_alpha(const u8 *data, s32 w, s32 h);
u8 *neko_tex_alpha_to_thresholded(const u8 *data, s32 w, s32 h, u8 threshold);
u8 *neko_tex_dilate_thresholded(const u8 *data, s32 w, s32 h);
u8 *neko_tex_thresholded_to_outlined(const u8 *data, s32 w, s32 h);

// outline path procedures
static neko_tex_bool neko_tex_find_first_filled_pixel(const u8 *data, s32 w, s32 h, neko_tex_point *first);
static neko_tex_bool neko_tex_find_next_filled_pixel(const u8 *data, s32 w, s32 h, neko_tex_point current, neko_tex_direction *dir, neko_tex_point *next);
neko_tex_point *neko_tex_extract_outline_path(u8 *data, s32 w, s32 h, s32 *point_count, neko_tex_point *reusable_outline);

void neko_tex_distance_based_path_simplification(neko_tex_point *outline, s32 *outline_length, f32 distance_threshold);

#if 1

// color brightness as perceived:
f32 brightness(neko_color_t c);
f32 color_num(neko_color_t c);

#define __check_hsv(c0, c1, p_func)                                            \
    do {                                                                       \
        hsv_t hsv0 = rgb_to_hsv(c0);                                           \
        hsv_t hsv1 = rgb_to_hsv(c1);                                           \
        f32 d = abs(color_num(c0) - color_num(c1)) + hue_dist(hsv0.h, hsv1.h); \
        if (d < min_dist) {                                                    \
            min_dist = d;                                                      \
            p = p_func();                                                      \
        }                                                                      \
    } while (0)

#define __check_dist_euclidean(c0, c1, p_func)                                \
    do {                                                                      \
        neko_vec4 c0_vec = neko_vec4{(f32)c0.r, (f32)c0.g, (f32)c0.b, 255.f}; \
        neko_vec4 c1_vec = neko_vec4{(f32)c1.r, (f32)c1.g, (f32)c1.b, 255.f}; \
        f32 d = neko_vec4_dist(c0_vec, c1_vec);                               \
        if (d < min_dist) {                                                   \
            min_dist = d;                                                     \
            p = p_func();                                                     \
        }                                                                     \
    } while (0)

#define __check_dist(c0, c1, p_func)                                          \
    do {                                                                      \
        f32 rd = (f32)c0.r - (f32)c1.r;                                       \
        f32 gd = (f32)c0.g - (f32)c1.g;                                       \
        f32 bd = (f32)c0.b - (f32)c1.b;                                       \
        f32 sd = rd * rd + gd * gd + bd * bd;                                 \
        f32 d = pow(rd * 0.299, 2) + pow(gd * 0.587, 2) + pow(bd * 0.114, 2); \
        if (d < min_dist) {                                                   \
            min_dist = d;                                                     \
            p = p_func();                                                     \
        }                                                                     \
    } while (0)

// particle_t get_closest_particle_from_color(neko_color_t c);

#endif

#endif