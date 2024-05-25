
#include "game_physics_math.hpp"

#include <cstdio>

#include "engine/neko_graphics.h"

namespace neko {

neko_vec2 subtract(neko_vec2 a, neko_vec2 b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}
neko_vec2 negate(neko_vec2 v) {
    v.x = -v.x;
    v.y = -v.y;
    return v;
}
neko_vec2 perpendicular(neko_vec2 v) {
    neko_vec2 p = {v.y, -v.x};
    return p;
}
f32 dotProduct(neko_vec2 a, neko_vec2 b) { return a.x * b.x + a.y * b.y; }
f32 lengthSquared(neko_vec2 v) { return v.x * v.x + v.y * v.y; }

//-----------------------------------------------------------------------------
// Triple product expansion is used to calculate perpendicular normal vectors
// which kinda 'prefer' pointing towards the Origin in Minkowski space

neko_vec2 tripleProduct(neko_vec2 a, neko_vec2 b, neko_vec2 c) {

    neko_vec2 r;

    f32 ac = a.x * c.x + a.y * c.y;  // perform a.dot(c)
    f32 bc = b.x * c.x + b.y * c.y;  // perform b.dot(c)

    // perform b * a.dot(c) - a * b.dot(c)
    r.x = b.x * ac - a.x * bc;
    r.y = b.y * ac - a.y * bc;
    return r;
}

//-----------------------------------------------------------------------------
// This is to compute average center (roughly). It might be different from
// Center of Gravity, especially for bodies with nonuniform density,
// but this is ok as initial direction of simplex search in GJK.

neko_vec2 averagePoint(const neko_vec2 *vertices, size_t count) {
    neko_vec2 avg = {0.f, 0.f};
    for (size_t i = 0; i < count; i++) {
        avg.x += vertices[i].x;
        avg.y += vertices[i].y;
    }
    avg.x /= count;
    avg.y /= count;
    return avg;
}

//-----------------------------------------------------------------------------
// Get furthest vertex along a certain direction

size_t indexOfFurthestPoint(const neko_vec2 *vertices, size_t count, neko_vec2 d) {

    f32 maxProduct = dotProduct(d, vertices[0]);
    size_t index = 0;
    for (size_t i = 1; i < count; i++) {
        f32 product = dotProduct(d, vertices[i]);
        if (product > maxProduct) {
            maxProduct = product;
            index = i;
        }
    }
    return index;
}

//-----------------------------------------------------------------------------
// Minkowski sum support function for GJK

neko_vec2 support(const neko_vec2 *vertices1, size_t count1, const neko_vec2 *vertices2, size_t count2, neko_vec2 d) {

    // get furthest point of first body along an arbitrary direction
    size_t i = indexOfFurthestPoint(vertices1, count1, d);

    // get furthest point of second body along the opposite direction
    size_t j = indexOfFurthestPoint(vertices2, count2, negate(d));

    // subtract (Minkowski sum) the two points to see if bodies 'overlap'
    return subtract(vertices1[i], vertices2[j]);
}

s32 iter_count = 0;

s32 fast_c2(const neko_vec2 *vertices1, size_t count1, const neko_vec2 *vertices2, size_t count2) {

    size_t index = 0;  // index of current vertex of simplex
    neko_vec2 a, b, c, d, ao, ab, ac, abperp, acperp, simplex[3];

    neko_vec2 position1 = averagePoint(vertices1, count1);  // not a CoG but
    neko_vec2 position2 = averagePoint(vertices2, count2);  // it's ok for GJK )

    // initial direction from the center of 1st body to the center of 2nd body
    d = subtract(position1, position2);

    // if initial direction is zero – set it to any arbitrary axis (we choose X)
    if ((d.x == 0) && (d.y == 0)) d.x = 1.f;

    // set the first support as initial point of the new simplex
    a = simplex[0] = support(vertices1, count1, vertices2, count2, d);

    if (dotProduct(a, d) <= 0) return 0;  // no collision

    d = negate(a);  // The next search direction is always towards the origin, so the next search direction is negate(a)

    while (1) {
        iter_count++;

        a = simplex[++index] = support(vertices1, count1, vertices2, count2, d);

        if (dotProduct(a, d) <= 0) return 0;  // no collision

        ao = negate(a);  // from point A to Origin is just negative A

        // simplex has 2 points (a line segment, not a triangle yet)
        if (index < 2) {
            b = simplex[0];
            ab = subtract(b, a);            // from point A to B
            d = tripleProduct(ab, ao, ab);  // normal to AB towards Origin
            if (lengthSquared(d) == 0) d = perpendicular(ab);
            continue;  // skip to next iteration
        }

        b = simplex[1];
        c = simplex[0];
        ab = subtract(b, a);  // from point A to B
        ac = subtract(c, a);  // from point A to C

        acperp = tripleProduct(ab, ac, ac);

        if (dotProduct(acperp, ao) >= 0) {

            d = acperp;  // new direction is normal to AC towards Origin

        } else {

            abperp = tripleProduct(ac, ab, ab);

            if (dotProduct(abperp, ao) < 0) return 1;  // collision

            simplex[0] = simplex[1];  // swap first element (point C)

            d = abperp;  // new direction is normal to AB towards Origin
        }

        simplex[1] = simplex[2];  // swap element in the middle (point B)
        --index;
    }

    return 0;
}

#pragma region TPPL

#define TPPL_VERTEXTYPE_REGULAR 0
#define TPPL_VERTEXTYPE_START 1
#define TPPL_VERTEXTYPE_END 2
#define TPPL_VERTEXTYPE_SPLIT 3
#define TPPL_VERTEXTYPE_MERGE 4

TPPLPoly::TPPLPoly() {
    hole = false;
    numpoints = 0;
    points = NULL;
}

TPPLPoly::~TPPLPoly() {
    if (points) delete[] points;
}

void TPPLPoly::Clear() {
    if (points) delete[] points;
    hole = false;
    numpoints = 0;
    points = NULL;
}

void TPPLPoly::Init(s64 numpoints) {
    Clear();
    this->numpoints = numpoints;
    points = new TPPLPoint[numpoints];
}

void TPPLPoly::Triangle(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3) {
    Init(3);
    points[0] = p1;
    points[1] = p2;
    points[2] = p3;
}

TPPLPoly::TPPLPoly(const TPPLPoly &src) : TPPLPoly() {
    hole = src.hole;
    numpoints = src.numpoints;

    if (numpoints > 0) {
        points = new TPPLPoint[numpoints];
        memcpy(points, src.points, numpoints * sizeof(TPPLPoint));
    }
}

TPPLPoly &TPPLPoly::operator=(const TPPLPoly &src) {
    Clear();
    hole = src.hole;
    numpoints = src.numpoints;

    if (numpoints > 0) {
        points = new TPPLPoint[numpoints];
        memcpy(points, src.points, numpoints * sizeof(TPPLPoint));
    }

    return *this;
}

s32 TPPLPoly::GetOrientation() const {
    s64 i1, i2;
    tppl_float area = 0;
    for (i1 = 0; i1 < numpoints; i1++) {
        i2 = i1 + 1;
        if (i2 == numpoints) i2 = 0;
        area += points[i1].x * points[i2].y - points[i1].y * points[i2].x;
    }
    if (area > 0) return TPPL_CCW;
    if (area < 0) return TPPL_CW;
    return 0;
}

void TPPLPoly::SetOrientation(s32 orientation) {
    s32 polyorientation = GetOrientation();
    if (polyorientation && (polyorientation != orientation)) {
        Invert();
    }
}

void TPPLPoly::Invert() { std::reverse(points, points + numpoints); }

TPPLPartition::PartitionVertex::PartitionVertex() : previous(NULL), next(NULL) {}

TPPLPoint TPPLPartition::Normalize(const TPPLPoint &p) {
    TPPLPoint r;
    tppl_float n = sqrt(p.x * p.x + p.y * p.y);
    if (n != 0) {
        r = p / n;
    } else {
        r.x = 0;
        r.y = 0;
    }
    return r;
}

tppl_float TPPLPartition::Distance(const TPPLPoint &p1, const TPPLPoint &p2) {
    tppl_float dx, dy;
    dx = p2.x - p1.x;
    dy = p2.y - p1.y;
    return (sqrt(dx * dx + dy * dy));
}

// checks if two lines intersect
s32 TPPLPartition::Intersects(TPPLPoint &p11, TPPLPoint &p12, TPPLPoint &p21, TPPLPoint &p22) {
    if ((p11.x == p21.x) && (p11.y == p21.y)) return 0;
    if ((p11.x == p22.x) && (p11.y == p22.y)) return 0;
    if ((p12.x == p21.x) && (p12.y == p21.y)) return 0;
    if ((p12.x == p22.x) && (p12.y == p22.y)) return 0;

    TPPLPoint v1ort, v2ort, v;
    tppl_float dot11, dot12, dot21, dot22;

    v1ort.x = p12.y - p11.y;
    v1ort.y = p11.x - p12.x;

    v2ort.x = p22.y - p21.y;
    v2ort.y = p21.x - p22.x;

    v = p21 - p11;
    dot21 = v.x * v1ort.x + v.y * v1ort.y;
    v = p22 - p11;
    dot22 = v.x * v1ort.x + v.y * v1ort.y;

    v = p11 - p21;
    dot11 = v.x * v2ort.x + v.y * v2ort.y;
    v = p12 - p21;
    dot12 = v.x * v2ort.x + v.y * v2ort.y;

    if (dot11 * dot12 > 0) return 0;
    if (dot21 * dot22 > 0) return 0;

    return 1;
}

// removes holes from inpolys by merging them with non-holes
s32 TPPLPartition::RemoveHoles(TPPLPolyList *inpolys, TPPLPolyList *outpolys) {
    TPPLPolyList polys;
    TPPLPolyList::iterator holeiter, polyiter, iter, iter2;
    s64 i, i2, holepointindex, polypointindex;
    TPPLPoint holepoint, polypoint, bestpolypoint;
    TPPLPoint linep1, linep2;
    TPPLPoint v1, v2;
    TPPLPoly newpoly;
    bool hasholes;
    bool pointvisible;
    bool pointfound;

    // check for trivial case (no holes)
    hasholes = false;
    for (iter = inpolys->begin(); iter != inpolys->end(); iter++) {
        if (iter->IsHole()) {
            hasholes = true;
            break;
        }
    }
    if (!hasholes) {
        for (iter = inpolys->begin(); iter != inpolys->end(); iter++) {
            outpolys->push_back(*iter);
        }
        // std::cout << "a" << std::endl;
        return 1;
    }

    polys = *inpolys;

    while (1) {
        // find the hole point with the largest x
        hasholes = false;
        for (iter = polys.begin(); iter != polys.end(); iter++) {
            if (!iter->IsHole()) continue;

            if (!hasholes) {
                hasholes = true;
                holeiter = iter;
                holepointindex = 0;
            }

            for (i = 0; i < iter->GetNumPoints(); i++) {
                if (iter->GetPoint(i).x > holeiter->GetPoint(holepointindex).x) {
                    holeiter = iter;
                    holepointindex = i;
                }
            }
        }
        if (!hasholes) break;
        holepoint = holeiter->GetPoint(holepointindex);

        pointfound = false;
        for (iter = polys.begin(); iter != polys.end(); iter++) {
            if (iter->IsHole()) continue;
            for (i = 0; i < iter->GetNumPoints(); i++) {
                if (iter->GetPoint(i).x <= holepoint.x) continue;
                if (!InCone(iter->GetPoint((i + iter->GetNumPoints() - 1) % (iter->GetNumPoints())), iter->GetPoint(i), iter->GetPoint((i + 1) % (iter->GetNumPoints())), holepoint)) continue;
                polypoint = iter->GetPoint(i);
                if (pointfound) {
                    v1 = Normalize(polypoint - holepoint);
                    v2 = Normalize(bestpolypoint - holepoint);
                    if (v2.x > v1.x) continue;
                }
                pointvisible = true;
                for (iter2 = polys.begin(); iter2 != polys.end(); iter2++) {
                    if (iter2->IsHole()) continue;
                    for (i2 = 0; i2 < iter2->GetNumPoints(); i2++) {
                        linep1 = iter2->GetPoint(i2);
                        linep2 = iter2->GetPoint((i2 + 1) % (iter2->GetNumPoints()));
                        if (Intersects(holepoint, polypoint, linep1, linep2)) {
                            pointvisible = false;
                            break;
                        }
                    }
                    if (!pointvisible) break;
                }
                if (pointvisible) {
                    pointfound = true;
                    bestpolypoint = polypoint;
                    polyiter = iter;
                    polypointindex = i;
                }
            }
        }

        if (!pointfound) return 0;

        newpoly.Init(holeiter->GetNumPoints() + polyiter->GetNumPoints() + 2);
        i2 = 0;
        for (i = 0; i <= polypointindex; i++) {
            newpoly[i2] = polyiter->GetPoint(i);
            i2++;
        }
        for (i = 0; i <= holeiter->GetNumPoints(); i++) {
            newpoly[i2] = holeiter->GetPoint((i + holepointindex) % holeiter->GetNumPoints());
            i2++;
        }
        for (i = polypointindex; i < polyiter->GetNumPoints(); i++) {
            newpoly[i2] = polyiter->GetPoint(i);
            i2++;
        }

        polys.erase(holeiter);
        polys.erase(polyiter);
        polys.push_back(newpoly);
    }

    for (iter = polys.begin(); iter != polys.end(); iter++) {
        outpolys->push_back(*iter);
    }

    // std::cout << "b" << std::endl;
    return 1;
}

bool TPPLPartition::IsConvex(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3) {
    tppl_float tmp;
    tmp = (p3.y - p1.y) * (p2.x - p1.x) - (p3.x - p1.x) * (p2.y - p1.y);
    if (tmp > 0)
        return 1;
    else
        return 0;
}

bool TPPLPartition::IsReflex(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3) {
    tppl_float tmp;
    tmp = (p3.y - p1.y) * (p2.x - p1.x) - (p3.x - p1.x) * (p2.y - p1.y);
    if (tmp < 0)
        return 1;
    else
        return 0;
}

bool TPPLPartition::IsInside(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3, TPPLPoint &p) {
    if (IsConvex(p1, p, p2)) return false;
    if (IsConvex(p2, p, p3)) return false;
    if (IsConvex(p3, p, p1)) return false;
    return true;
}

bool TPPLPartition::InCone(TPPLPoint &p1, TPPLPoint &p2, TPPLPoint &p3, TPPLPoint &p) {
    bool convex;

    convex = IsConvex(p1, p2, p3);

    if (convex) {
        if (!IsConvex(p1, p2, p)) return false;
        if (!IsConvex(p2, p3, p)) return false;
        return true;
    } else {
        if (IsConvex(p1, p2, p)) return true;
        if (IsConvex(p2, p3, p)) return true;
        return false;
    }
}

bool TPPLPartition::InCone(PartitionVertex *v, TPPLPoint &p) {
    TPPLPoint p1, p2, p3;

    p1 = v->previous->p;
    p2 = v->p;
    p3 = v->next->p;

    return InCone(p1, p2, p3, p);
}

void TPPLPartition::UpdateVertexReflexity(PartitionVertex *v) {
    PartitionVertex *v1 = NULL, *v3 = NULL;
    v1 = v->previous;
    v3 = v->next;
    v->isConvex = !IsReflex(v1->p, v->p, v3->p);
}

void TPPLPartition::UpdateVertex(PartitionVertex *v, PartitionVertex *vertices, s64 numvertices) {
    s64 i;
    PartitionVertex *v1 = NULL, *v3 = NULL;
    TPPLPoint vec1, vec3;

    v1 = v->previous;
    v3 = v->next;

    v->isConvex = IsConvex(v1->p, v->p, v3->p);

    vec1 = Normalize(v1->p - v->p);
    vec3 = Normalize(v3->p - v->p);
    v->angle = vec1.x * vec3.x + vec1.y * vec3.y;

    if (v->isConvex) {
        v->isEar = true;
        for (i = 0; i < numvertices; i++) {
            if ((vertices[i].p.x == v->p.x) && (vertices[i].p.y == v->p.y)) continue;
            if ((vertices[i].p.x == v1->p.x) && (vertices[i].p.y == v1->p.y)) continue;
            if ((vertices[i].p.x == v3->p.x) && (vertices[i].p.y == v3->p.y)) continue;
            if (IsInside(v1->p, v->p, v3->p, vertices[i].p)) {
                v->isEar = false;
                break;
            }
        }
    } else {
        v->isEar = false;
    }
}

// triangulation by ear removal
s32 TPPLPartition::Triangulate_EC(TPPLPoly *poly, TPPLPolyList *triangles) {
    if (!poly->Valid()) return 0;

    s64 numvertices;
    PartitionVertex *vertices = NULL;
    PartitionVertex *ear = NULL;
    TPPLPoly triangle;
    s64 i, j;
    bool earfound;

    if (poly->GetNumPoints() < 3) return 0;
    if (poly->GetNumPoints() == 3) {
        triangles->push_back(*poly);
        return 1;
    }

    numvertices = poly->GetNumPoints();

    vertices = new PartitionVertex[numvertices];
    for (i = 0; i < numvertices; i++) {
        vertices[i].isActive = true;
        vertices[i].p = poly->GetPoint(i);
        if (i == (numvertices - 1))
            vertices[i].next = &(vertices[0]);
        else
            vertices[i].next = &(vertices[i + 1]);
        if (i == 0)
            vertices[i].previous = &(vertices[numvertices - 1]);
        else
            vertices[i].previous = &(vertices[i - 1]);
    }
    for (i = 0; i < numvertices; i++) {
        UpdateVertex(&vertices[i], vertices, numvertices);
    }

    for (i = 0; i < numvertices - 3; i++) {
        earfound = false;
        // find the most extruded ear
        for (j = 0; j < numvertices; j++) {
            if (!vertices[j].isActive) continue;
            if (!vertices[j].isEar) continue;
            if (!earfound) {
                earfound = true;
                ear = &(vertices[j]);
            } else {
                if (vertices[j].angle > ear->angle) {
                    ear = &(vertices[j]);
                }
            }
        }
        if (!earfound) {
            delete[] vertices;
            return 0;
        }

        triangle.Triangle(ear->previous->p, ear->p, ear->next->p);
        triangles->push_back(triangle);

        ear->isActive = false;
        ear->previous->next = ear->next;
        ear->next->previous = ear->previous;

        if (i == numvertices - 4) break;

        UpdateVertex(ear->previous, vertices, numvertices);
        UpdateVertex(ear->next, vertices, numvertices);
    }
    for (i = 0; i < numvertices; i++) {
        if (vertices[i].isActive) {
            triangle.Triangle(vertices[i].previous->p, vertices[i].p, vertices[i].next->p);
            triangles->push_back(triangle);
            break;
        }
    }

    delete[] vertices;

    return 1;
}

s32 TPPLPartition::Triangulate_EC(TPPLPolyList *inpolys, TPPLPolyList *triangles) {
    TPPLPolyList outpolys;
    TPPLPolyList::iterator iter;

    if (!RemoveHoles(inpolys, &outpolys)) return 0;
    for (iter = outpolys.begin(); iter != outpolys.end(); iter++) {
        if (!Triangulate_EC(&(*iter), triangles)) return 0;
    }
    return 1;
}

s32 TPPLPartition::ConvexPartition_HM(TPPLPoly *poly, TPPLPolyList *parts) {
    if (!poly->Valid()) return 0;

    TPPLPolyList triangles;
    TPPLPolyList::iterator iter1, iter2;
    TPPLPoly *poly1 = NULL, *poly2 = NULL;
    TPPLPoly newpoly;
    TPPLPoint d1, d2, p1, p2, p3;
    s64 i11, i12, i21, i22, i13, i23, j, k;
    bool isdiagonal;
    s64 numreflex;

    // check if the poly is already convex
    numreflex = 0;
    for (i11 = 0; i11 < poly->GetNumPoints(); i11++) {
        if (i11 == 0)
            i12 = poly->GetNumPoints() - 1;
        else
            i12 = i11 - 1;
        if (i11 == (poly->GetNumPoints() - 1))
            i13 = 0;
        else
            i13 = i11 + 1;
        if (IsReflex(poly->GetPoint(i12), poly->GetPoint(i11), poly->GetPoint(i13))) {
            numreflex = 1;
            break;
        }
    }
    if (numreflex == 0) {
        parts->push_back(*poly);
        return 1;
    }

    if (!Triangulate_EC(poly, &triangles)) return 0;

    for (iter1 = triangles.begin(); iter1 != triangles.end(); iter1++) {
        poly1 = &(*iter1);
        for (i11 = 0; i11 < poly1->GetNumPoints(); i11++) {
            d1 = poly1->GetPoint(i11);
            i12 = (i11 + 1) % (poly1->GetNumPoints());
            d2 = poly1->GetPoint(i12);

            isdiagonal = false;
            for (iter2 = iter1; iter2 != triangles.end(); iter2++) {
                if (iter1 == iter2) continue;
                poly2 = &(*iter2);

                for (i21 = 0; i21 < poly2->GetNumPoints(); i21++) {
                    if ((d2.x != poly2->GetPoint(i21).x) || (d2.y != poly2->GetPoint(i21).y)) continue;
                    i22 = (i21 + 1) % (poly2->GetNumPoints());
                    if ((d1.x != poly2->GetPoint(i22).x) || (d1.y != poly2->GetPoint(i22).y)) continue;
                    isdiagonal = true;
                    break;
                }
                if (isdiagonal) break;
            }

            if (!isdiagonal) continue;

            p2 = poly1->GetPoint(i11);
            if (i11 == 0)
                i13 = poly1->GetNumPoints() - 1;
            else
                i13 = i11 - 1;
            p1 = poly1->GetPoint(i13);
            if (i22 == (poly2->GetNumPoints() - 1))
                i23 = 0;
            else
                i23 = i22 + 1;
            p3 = poly2->GetPoint(i23);

            if (!IsConvex(p1, p2, p3)) continue;

            p2 = poly1->GetPoint(i12);
            if (i12 == (poly1->GetNumPoints() - 1))
                i13 = 0;
            else
                i13 = i12 + 1;
            p3 = poly1->GetPoint(i13);
            if (i21 == 0)
                i23 = poly2->GetNumPoints() - 1;
            else
                i23 = i21 - 1;
            p1 = poly2->GetPoint(i23);

            if (!IsConvex(p1, p2, p3)) continue;

            newpoly.Init(poly1->GetNumPoints() + poly2->GetNumPoints() - 2);
            k = 0;
            for (j = i12; j != i11; j = (j + 1) % (poly1->GetNumPoints())) {
                newpoly[k] = poly1->GetPoint(j);
                k++;
            }
            for (j = i22; j != i21; j = (j + 1) % (poly2->GetNumPoints())) {
                newpoly[k] = poly2->GetPoint(j);
                k++;
            }

            triangles.erase(iter2);
            *iter1 = newpoly;
            poly1 = &(*iter1);
            i11 = -1;

            continue;
        }
    }

    for (iter1 = triangles.begin(); iter1 != triangles.end(); iter1++) {
        parts->push_back(*iter1);
    }

    return 1;
}

s32 TPPLPartition::ConvexPartition_HM(TPPLPolyList *inpolys, TPPLPolyList *parts) {
    TPPLPolyList outpolys;
    TPPLPolyList::iterator iter;

    if (!RemoveHoles(inpolys, &outpolys)) return 0;
    for (iter = outpolys.begin(); iter != outpolys.end(); iter++) {
        if (!ConvexPartition_HM(&(*iter), parts)) return 0;
    }
    return 1;
}

// minimum-weight polygon triangulation by dynamic programming
// O(n^3) time complexity
// O(n^2) space complexity
s32 TPPLPartition::Triangulate_OPT(TPPLPoly *poly, TPPLPolyList *triangles) {
    if (!poly->Valid()) return 0;

    s64 i, j, k, gap, n;
    DPState **dpstates = NULL;
    TPPLPoint p1, p2, p3, p4;
    s64 bestvertex;
    tppl_float weight, minweight, d1, d2;
    Diagonal diagonal, newdiagonal;
    DiagonalList diagonals;
    TPPLPoly triangle;
    s32 ret = 1;

    n = poly->GetNumPoints();
    dpstates = new DPState *[n];
    for (i = 1; i < n; i++) {
        dpstates[i] = new DPState[i];
    }

    // init states and visibility
    for (i = 0; i < (n - 1); i++) {
        p1 = poly->GetPoint(i);
        for (j = i + 1; j < n; j++) {
            dpstates[j][i].visible = true;
            dpstates[j][i].weight = 0;
            dpstates[j][i].bestvertex = -1;
            if (j != (i + 1)) {
                p2 = poly->GetPoint(j);

                // visibility check
                if (i == 0)
                    p3 = poly->GetPoint(n - 1);
                else
                    p3 = poly->GetPoint(i - 1);
                if (i == (n - 1))
                    p4 = poly->GetPoint(0);
                else
                    p4 = poly->GetPoint(i + 1);
                if (!InCone(p3, p1, p4, p2)) {
                    dpstates[j][i].visible = false;
                    continue;
                }

                if (j == 0)
                    p3 = poly->GetPoint(n - 1);
                else
                    p3 = poly->GetPoint(j - 1);
                if (j == (n - 1))
                    p4 = poly->GetPoint(0);
                else
                    p4 = poly->GetPoint(j + 1);
                if (!InCone(p3, p2, p4, p1)) {
                    dpstates[j][i].visible = false;
                    continue;
                }

                for (k = 0; k < n; k++) {
                    p3 = poly->GetPoint(k);
                    if (k == (n - 1))
                        p4 = poly->GetPoint(0);
                    else
                        p4 = poly->GetPoint(k + 1);
                    if (Intersects(p1, p2, p3, p4)) {
                        dpstates[j][i].visible = false;
                        break;
                    }
                }
            }
        }
    }
    dpstates[n - 1][0].visible = true;
    dpstates[n - 1][0].weight = 0;
    dpstates[n - 1][0].bestvertex = -1;

    for (gap = 2; gap < n; gap++) {
        for (i = 0; i < (n - gap); i++) {
            j = i + gap;
            if (!dpstates[j][i].visible) continue;
            bestvertex = -1;
            for (k = (i + 1); k < j; k++) {
                if (!dpstates[k][i].visible) continue;
                if (!dpstates[j][k].visible) continue;

                if (k <= (i + 1))
                    d1 = 0;
                else
                    d1 = Distance(poly->GetPoint(i), poly->GetPoint(k));
                if (j <= (k + 1))
                    d2 = 0;
                else
                    d2 = Distance(poly->GetPoint(k), poly->GetPoint(j));

                weight = dpstates[k][i].weight + dpstates[j][k].weight + d1 + d2;

                if ((bestvertex == -1) || (weight < minweight)) {
                    bestvertex = k;
                    minweight = weight;
                }
            }
            if (bestvertex == -1) {
                for (i = 1; i < n; i++) {
                    delete[] dpstates[i];
                }
                delete[] dpstates;

                return 0;
            }

            dpstates[j][i].bestvertex = bestvertex;
            dpstates[j][i].weight = minweight;
        }
    }

    newdiagonal.index1 = 0;
    newdiagonal.index2 = n - 1;
    diagonals.push_back(newdiagonal);
    while (!diagonals.empty()) {
        diagonal = *(diagonals.begin());
        diagonals.pop_front();
        bestvertex = dpstates[diagonal.index2][diagonal.index1].bestvertex;
        if (bestvertex == -1) {
            ret = 0;
            break;
        }
        triangle.Triangle(poly->GetPoint(diagonal.index1), poly->GetPoint(bestvertex), poly->GetPoint(diagonal.index2));
        triangles->push_back(triangle);
        if (bestvertex > (diagonal.index1 + 1)) {
            newdiagonal.index1 = diagonal.index1;
            newdiagonal.index2 = bestvertex;
            diagonals.push_back(newdiagonal);
        }
        if (diagonal.index2 > (bestvertex + 1)) {
            newdiagonal.index1 = bestvertex;
            newdiagonal.index2 = diagonal.index2;
            diagonals.push_back(newdiagonal);
        }
    }

    for (i = 1; i < n; i++) {
        delete[] dpstates[i];
    }
    delete[] dpstates;

    return ret;
}

void TPPLPartition::UpdateState(s64 a, s64 b, s64 w, s64 i, s64 j, DPState2 **dpstates) {
    Diagonal newdiagonal;
    DiagonalList *pairs = NULL;
    s64 w2;

    w2 = dpstates[a][b].weight;
    if (w > w2) return;

    pairs = &(dpstates[a][b].pairs);
    newdiagonal.index1 = i;
    newdiagonal.index2 = j;

    if (w < w2) {
        pairs->clear();
        pairs->push_front(newdiagonal);
        dpstates[a][b].weight = w;
    } else {
        if ((!pairs->empty()) && (i <= pairs->begin()->index1)) return;
        while ((!pairs->empty()) && (pairs->begin()->index2 >= j)) pairs->pop_front();
        pairs->push_front(newdiagonal);
    }
}

void TPPLPartition::TypeA(s64 i, s64 j, s64 k, PartitionVertex *vertices, DPState2 **dpstates) {
    DiagonalList *pairs = NULL;
    DiagonalList::iterator iter, lastiter;
    s64 top;
    s64 w;

    if (!dpstates[i][j].visible) return;
    top = j;
    w = dpstates[i][j].weight;
    if (k - j > 1) {
        if (!dpstates[j][k].visible) return;
        w += dpstates[j][k].weight + 1;
    }
    if (j - i > 1) {
        pairs = &(dpstates[i][j].pairs);
        iter = pairs->end();
        lastiter = pairs->end();
        while (iter != pairs->begin()) {
            iter--;
            if (!IsReflex(vertices[iter->index2].p, vertices[j].p, vertices[k].p))
                lastiter = iter;
            else
                break;
        }
        if (lastiter == pairs->end())
            w++;
        else {
            if (IsReflex(vertices[k].p, vertices[i].p, vertices[lastiter->index1].p))
                w++;
            else
                top = lastiter->index1;
        }
    }
    UpdateState(i, k, w, top, j, dpstates);
}

void TPPLPartition::TypeB(s64 i, s64 j, s64 k, PartitionVertex *vertices, DPState2 **dpstates) {
    DiagonalList *pairs = NULL;
    DiagonalList::iterator iter, lastiter;
    s64 top;
    s64 w;

    if (!dpstates[j][k].visible) return;
    top = j;
    w = dpstates[j][k].weight;

    if (j - i > 1) {
        if (!dpstates[i][j].visible) return;
        w += dpstates[i][j].weight + 1;
    }
    if (k - j > 1) {
        pairs = &(dpstates[j][k].pairs);

        iter = pairs->begin();
        if ((!pairs->empty()) && (!IsReflex(vertices[i].p, vertices[j].p, vertices[iter->index1].p))) {
            lastiter = iter;
            while (iter != pairs->end()) {
                if (!IsReflex(vertices[i].p, vertices[j].p, vertices[iter->index1].p)) {
                    lastiter = iter;
                    iter++;
                } else
                    break;
            }
            if (IsReflex(vertices[lastiter->index2].p, vertices[k].p, vertices[i].p))
                w++;
            else
                top = lastiter->index2;
        } else
            w++;
    }
    UpdateState(i, k, w, j, top, dpstates);
}

s32 TPPLPartition::ConvexPartition_OPT(TPPLPoly *poly, TPPLPolyList *parts) {
    if (!poly->Valid()) return 0;

    TPPLPoint p1, p2, p3, p4;
    PartitionVertex *vertices = NULL;
    DPState2 **dpstates = NULL;
    s64 i, j, k, n, gap;
    DiagonalList diagonals, diagonals2;
    Diagonal diagonal, newdiagonal;
    DiagonalList *pairs = NULL, *pairs2 = NULL;
    DiagonalList::iterator iter, iter2;
    s32 ret;
    TPPLPoly newpoly;
    std::vector<s64> indices;
    std::vector<s64>::iterator iiter;
    bool ijreal, jkreal;

    n = poly->GetNumPoints();
    vertices = new PartitionVertex[n];

    dpstates = new DPState2 *[n];
    for (i = 0; i < n; i++) {
        dpstates[i] = new DPState2[n];
    }

    // init vertex information
    for (i = 0; i < n; i++) {
        vertices[i].p = poly->GetPoint(i);
        vertices[i].isActive = true;
        if (i == 0)
            vertices[i].previous = &(vertices[n - 1]);
        else
            vertices[i].previous = &(vertices[i - 1]);
        if (i == (poly->GetNumPoints() - 1))
            vertices[i].next = &(vertices[0]);
        else
            vertices[i].next = &(vertices[i + 1]);
    }
    for (i = 1; i < n; i++) {
        UpdateVertexReflexity(&(vertices[i]));
    }

    // init states and visibility
    for (i = 0; i < (n - 1); i++) {
        p1 = poly->GetPoint(i);
        for (j = i + 1; j < n; j++) {
            dpstates[i][j].visible = true;
            if (j == i + 1) {
                dpstates[i][j].weight = 0;
            } else {
                dpstates[i][j].weight = 2147483647;
            }
            if (j != (i + 1)) {
                p2 = poly->GetPoint(j);

                // visibility check
                if (!InCone(&vertices[i], p2)) {
                    dpstates[i][j].visible = false;
                    continue;
                }
                if (!InCone(&vertices[j], p1)) {
                    dpstates[i][j].visible = false;
                    continue;
                }

                for (k = 0; k < n; k++) {
                    p3 = poly->GetPoint(k);
                    if (k == (n - 1))
                        p4 = poly->GetPoint(0);
                    else
                        p4 = poly->GetPoint(k + 1);
                    if (Intersects(p1, p2, p3, p4)) {
                        dpstates[i][j].visible = false;
                        break;
                    }
                }
            }
        }
    }
    for (i = 0; i < (n - 2); i++) {
        j = i + 2;
        if (dpstates[i][j].visible) {
            dpstates[i][j].weight = 0;
            newdiagonal.index1 = i + 1;
            newdiagonal.index2 = i + 1;
            dpstates[i][j].pairs.push_back(newdiagonal);
        }
    }

    dpstates[0][n - 1].visible = true;
    vertices[0].isConvex = false;  // by convention

    for (gap = 3; gap < n; gap++) {
        for (i = 0; i < n - gap; i++) {
            if (vertices[i].isConvex) continue;
            k = i + gap;
            if (dpstates[i][k].visible) {
                if (!vertices[k].isConvex) {
                    for (j = i + 1; j < k; j++) TypeA(i, j, k, vertices, dpstates);
                } else {
                    for (j = i + 1; j < (k - 1); j++) {
                        if (vertices[j].isConvex) continue;
                        TypeA(i, j, k, vertices, dpstates);
                    }
                    TypeA(i, k - 1, k, vertices, dpstates);
                }
            }
        }
        for (k = gap; k < n; k++) {
            if (vertices[k].isConvex) continue;
            i = k - gap;
            if ((vertices[i].isConvex) && (dpstates[i][k].visible)) {
                TypeB(i, i + 1, k, vertices, dpstates);
                for (j = i + 2; j < k; j++) {
                    if (vertices[j].isConvex) continue;
                    TypeB(i, j, k, vertices, dpstates);
                }
            }
        }
    }

    // recover solution
    ret = 1;
    newdiagonal.index1 = 0;
    newdiagonal.index2 = n - 1;
    diagonals.push_front(newdiagonal);
    while (!diagonals.empty()) {
        diagonal = *(diagonals.begin());
        diagonals.pop_front();
        if ((diagonal.index2 - diagonal.index1) <= 1) continue;
        pairs = &(dpstates[diagonal.index1][diagonal.index2].pairs);
        if (pairs->empty()) {
            ret = 0;
            break;
        }
        if (!vertices[diagonal.index1].isConvex) {
            iter = pairs->end();
            iter--;
            j = iter->index2;
            newdiagonal.index1 = j;
            newdiagonal.index2 = diagonal.index2;
            diagonals.push_front(newdiagonal);
            if ((j - diagonal.index1) > 1) {
                if (iter->index1 != iter->index2) {
                    pairs2 = &(dpstates[diagonal.index1][j].pairs);
                    while (1) {
                        if (pairs2->empty()) {
                            ret = 0;
                            break;
                        }
                        iter2 = pairs2->end();
                        iter2--;
                        if (iter->index1 != iter2->index1)
                            pairs2->pop_back();
                        else
                            break;
                    }
                    if (ret == 0) break;
                }
                newdiagonal.index1 = diagonal.index1;
                newdiagonal.index2 = j;
                diagonals.push_front(newdiagonal);
            }
        } else {
            iter = pairs->begin();
            j = iter->index1;
            newdiagonal.index1 = diagonal.index1;
            newdiagonal.index2 = j;
            diagonals.push_front(newdiagonal);
            if ((diagonal.index2 - j) > 1) {
                if (iter->index1 != iter->index2) {
                    pairs2 = &(dpstates[j][diagonal.index2].pairs);
                    while (1) {
                        if (pairs2->empty()) {
                            ret = 0;
                            break;
                        }
                        iter2 = pairs2->begin();
                        if (iter->index2 != iter2->index2)
                            pairs2->pop_front();
                        else
                            break;
                    }
                    if (ret == 0) break;
                }
                newdiagonal.index1 = j;
                newdiagonal.index2 = diagonal.index2;
                diagonals.push_front(newdiagonal);
            }
        }
    }

    if (ret == 0) {
        for (i = 0; i < n; i++) {
            delete[] dpstates[i];
        }
        delete[] dpstates;
        delete[] vertices;

        return ret;
    }

    newdiagonal.index1 = 0;
    newdiagonal.index2 = n - 1;
    diagonals.push_front(newdiagonal);
    while (!diagonals.empty()) {
        diagonal = *(diagonals.begin());
        diagonals.pop_front();
        if ((diagonal.index2 - diagonal.index1) <= 1) continue;

        indices.clear();
        diagonals2.clear();
        indices.push_back(diagonal.index1);
        indices.push_back(diagonal.index2);
        diagonals2.push_front(diagonal);

        while (!diagonals2.empty()) {
            diagonal = *(diagonals2.begin());
            diagonals2.pop_front();
            if ((diagonal.index2 - diagonal.index1) <= 1) continue;
            ijreal = true;
            jkreal = true;
            pairs = &(dpstates[diagonal.index1][diagonal.index2].pairs);
            if (!vertices[diagonal.index1].isConvex) {
                iter = pairs->end();
                iter--;
                j = iter->index2;
                if (iter->index1 != iter->index2) ijreal = false;
            } else {
                iter = pairs->begin();
                j = iter->index1;
                if (iter->index1 != iter->index2) jkreal = false;
            }

            newdiagonal.index1 = diagonal.index1;
            newdiagonal.index2 = j;
            if (ijreal) {
                diagonals.push_back(newdiagonal);
            } else {
                diagonals2.push_back(newdiagonal);
            }

            newdiagonal.index1 = j;
            newdiagonal.index2 = diagonal.index2;
            if (jkreal) {
                diagonals.push_back(newdiagonal);
            } else {
                diagonals2.push_back(newdiagonal);
            }

            indices.push_back(j);
        }

        std::sort(indices.begin(), indices.end());
        newpoly.Init((s64)indices.size());
        k = 0;
        for (iiter = indices.begin(); iiter != indices.end(); iiter++) {
            newpoly[k] = vertices[*iiter].p;
            k++;
        }
        parts->push_back(newpoly);
    }

    for (i = 0; i < n; i++) {
        delete[] dpstates[i];
    }
    delete[] dpstates;
    delete[] vertices;

    return ret;
}

// triangulates a set of polygons by first partitioning them into monotone polygons
// O(n*log(n)) time complexity, O(n) space complexity
// the algorithm used here is outlined in the book
//"Computational Geometry: Algorithms and Applications"
// by Mark de Berg, Otfried Cheong, Marc van Kreveld and Mark Overmars
s32 TPPLPartition::MonotonePartition(TPPLPolyList *inpolys, TPPLPolyList *monotonePolys) {
    TPPLPolyList::iterator iter;
    MonotoneVertex *vertices = NULL;
    s64 i, numvertices, vindex, vindex2, newnumvertices, maxnumvertices;
    s64 polystartindex, polyendindex;
    TPPLPoly *poly = NULL;
    MonotoneVertex *v = NULL, *v2 = NULL, *vprev = NULL, *vnext = NULL;
    ScanLineEdge newedge;
    bool error = false;

    numvertices = 0;
    for (iter = inpolys->begin(); iter != inpolys->end(); iter++) {
        if (!iter->Valid()) return 0;
        numvertices += iter->GetNumPoints();
    }

    maxnumvertices = numvertices * 3;
    vertices = new MonotoneVertex[maxnumvertices];
    newnumvertices = numvertices;

    polystartindex = 0;
    for (iter = inpolys->begin(); iter != inpolys->end(); iter++) {
        poly = &(*iter);
        polyendindex = polystartindex + poly->GetNumPoints() - 1;
        for (i = 0; i < poly->GetNumPoints(); i++) {
            vertices[i + polystartindex].p = poly->GetPoint(i);
            if (i == 0)
                vertices[i + polystartindex].previous = polyendindex;
            else
                vertices[i + polystartindex].previous = i + polystartindex - 1;
            if (i == (poly->GetNumPoints() - 1))
                vertices[i + polystartindex].next = polystartindex;
            else
                vertices[i + polystartindex].next = i + polystartindex + 1;
        }
        polystartindex = polyendindex + 1;
    }

    // construct the priority queue
    s64 *priority = new s64[numvertices];
    for (i = 0; i < numvertices; i++) priority[i] = i;
    std::sort(priority, &(priority[numvertices]), VertexSorter(vertices));

    // determine vertex types
    char *vertextypes = new char[maxnumvertices];
    for (i = 0; i < numvertices; i++) {
        v = &(vertices[i]);
        vprev = &(vertices[v->previous]);
        vnext = &(vertices[v->next]);

        if (PBelow(vprev->p, v->p) && PBelow(vnext->p, v->p)) {
            if (IsConvex(vnext->p, vprev->p, v->p)) {
                vertextypes[i] = TPPL_VERTEXTYPE_START;
            } else {
                vertextypes[i] = TPPL_VERTEXTYPE_SPLIT;
            }
        } else if (PBelow(v->p, vprev->p) && PBelow(v->p, vnext->p)) {
            if (IsConvex(vnext->p, vprev->p, v->p)) {
                vertextypes[i] = TPPL_VERTEXTYPE_END;
            } else {
                vertextypes[i] = TPPL_VERTEXTYPE_MERGE;
            }
        } else {
            vertextypes[i] = TPPL_VERTEXTYPE_REGULAR;
        }
    }

    // helpers
    s64 *helpers = new s64[maxnumvertices];

    // binary search tree that holds edges intersecting the scanline
    // note that while set doesn't actually have to be implemented as a tree
    // complexity requirements for operations are the same as for the balanced binary search tree
    std::set<ScanLineEdge> edgeTree;
    // store iterators to the edge tree elements
    // this makes deleting existing edges much faster
    std::set<ScanLineEdge>::iterator *edgeTreeIterators, edgeIter;
    edgeTreeIterators = new std::set<ScanLineEdge>::iterator[maxnumvertices];
    std::pair<std::set<ScanLineEdge>::iterator, bool> edgeTreeRet;
    for (i = 0; i < numvertices; i++) edgeTreeIterators[i] = edgeTree.end();

    // for each vertex
    for (i = 0; i < numvertices; i++) {
        vindex = priority[i];
        v = &(vertices[vindex]);
        vindex2 = vindex;
        v2 = v;

        // depending on the vertex type, do the appropriate action
        // comments in the following sections are copied from "Computational Geometry: Algorithms and Applications"
        switch (vertextypes[vindex]) {
            case TPPL_VERTEXTYPE_START:
                // Insert ei in T and set helper(ei) to vi.
                newedge.p1 = v->p;
                newedge.p2 = vertices[v->next].p;
                newedge.index = vindex;
                edgeTreeRet = edgeTree.insert(newedge);
                edgeTreeIterators[vindex] = edgeTreeRet.first;
                helpers[vindex] = vindex;
                break;

            case TPPL_VERTEXTYPE_END:
                if (edgeTreeIterators[v->previous] == edgeTree.end()) {
                    error = true;
                    break;
                }
                // if helper(ei-1) is a merge vertex
                if (vertextypes[helpers[v->previous]] == TPPL_VERTEXTYPE_MERGE) {
                    // Insert the diagonal connecting vi to helper(ei-1) in D.
                    AddDiagonal(vertices, &newnumvertices, vindex, helpers[v->previous], vertextypes, edgeTreeIterators, &edgeTree, helpers);
                }
                // Delete ei-1 from T
                edgeTree.erase(edgeTreeIterators[v->previous]);
                break;

            case TPPL_VERTEXTYPE_SPLIT:
                // Search in T to find the edge e j directly left of vi.
                newedge.p1 = v->p;
                newedge.p2 = v->p;
                edgeIter = edgeTree.lower_bound(newedge);
                if (edgeIter == edgeTree.begin()) {
                    error = true;
                    break;
                }
                edgeIter--;
                // Insert the diagonal connecting vi to helper(ej) in D.
                AddDiagonal(vertices, &newnumvertices, vindex, helpers[edgeIter->index], vertextypes, edgeTreeIterators, &edgeTree, helpers);
                vindex2 = newnumvertices - 2;
                v2 = &(vertices[vindex2]);
                // helper(e j)�vi
                helpers[edgeIter->index] = vindex;
                // Insert ei in T and set helper(ei) to vi.
                newedge.p1 = v2->p;
                newedge.p2 = vertices[v2->next].p;
                newedge.index = vindex2;
                edgeTreeRet = edgeTree.insert(newedge);
                edgeTreeIterators[vindex2] = edgeTreeRet.first;
                helpers[vindex2] = vindex2;
                break;

            case TPPL_VERTEXTYPE_MERGE:
                if (edgeTreeIterators[v->previous] == edgeTree.end()) {
                    error = true;
                    break;
                }
                // if helper(ei-1) is a merge vertex
                if (vertextypes[helpers[v->previous]] == TPPL_VERTEXTYPE_MERGE) {
                    // Insert the diagonal connecting vi to helper(ei-1) in D.
                    AddDiagonal(vertices, &newnumvertices, vindex, helpers[v->previous], vertextypes, edgeTreeIterators, &edgeTree, helpers);
                    vindex2 = newnumvertices - 2;
                    v2 = &(vertices[vindex2]);
                }
                // Delete ei-1 from T.
                edgeTree.erase(edgeTreeIterators[v->previous]);
                // Search in T to find the edge e j directly left of vi.
                newedge.p1 = v->p;
                newedge.p2 = v->p;
                edgeIter = edgeTree.lower_bound(newedge);
                if (edgeIter == edgeTree.begin()) {
                    error = true;
                    break;
                }
                edgeIter--;
                // if helper(ej) is a merge vertex
                if (vertextypes[helpers[edgeIter->index]] == TPPL_VERTEXTYPE_MERGE) {
                    // Insert the diagonal connecting vi to helper(e j) in D.
                    AddDiagonal(vertices, &newnumvertices, vindex2, helpers[edgeIter->index], vertextypes, edgeTreeIterators, &edgeTree, helpers);
                }
                // helper(e j)�vi
                helpers[edgeIter->index] = vindex2;
                break;

            case TPPL_VERTEXTYPE_REGULAR:
                // if the interior of P lies to the right of vi
                if (PBelow(v->p, vertices[v->previous].p)) {
                    if (edgeTreeIterators[v->previous] == edgeTree.end()) {
                        error = true;
                        break;
                    }
                    // if helper(ei-1) is a merge vertex
                    if (vertextypes[helpers[v->previous]] == TPPL_VERTEXTYPE_MERGE) {
                        // Insert the diagonal connecting vi to helper(ei-1) in D.
                        AddDiagonal(vertices, &newnumvertices, vindex, helpers[v->previous], vertextypes, edgeTreeIterators, &edgeTree, helpers);
                        vindex2 = newnumvertices - 2;
                        v2 = &(vertices[vindex2]);
                    }
                    // Delete ei-1 from T.
                    edgeTree.erase(edgeTreeIterators[v->previous]);
                    // Insert ei in T and set helper(ei) to vi.
                    newedge.p1 = v2->p;
                    newedge.p2 = vertices[v2->next].p;
                    newedge.index = vindex2;
                    edgeTreeRet = edgeTree.insert(newedge);
                    edgeTreeIterators[vindex2] = edgeTreeRet.first;
                    helpers[vindex2] = vindex;
                } else {
                    // Search in T to find the edge ej directly left of vi.
                    newedge.p1 = v->p;
                    newedge.p2 = v->p;
                    edgeIter = edgeTree.lower_bound(newedge);
                    if (edgeIter == edgeTree.begin()) {
                        error = true;
                        break;
                    }
                    edgeIter--;
                    // if helper(ej) is a merge vertex
                    if (vertextypes[helpers[edgeIter->index]] == TPPL_VERTEXTYPE_MERGE) {
                        // Insert the diagonal connecting vi to helper(e j) in D.
                        AddDiagonal(vertices, &newnumvertices, vindex, helpers[edgeIter->index], vertextypes, edgeTreeIterators, &edgeTree, helpers);
                    }
                    // helper(e j)�vi
                    helpers[edgeIter->index] = vindex;
                }
                break;
        }

        if (error) break;
    }

    char *used = new char[newnumvertices];
    memset(used, 0, newnumvertices * sizeof(char));

    if (!error) {
        // return result
        s64 size;
        TPPLPoly mpoly;
        for (i = 0; i < newnumvertices; i++) {
            if (used[i]) continue;
            v = &(vertices[i]);
            vnext = &(vertices[v->next]);
            size = 1;
            while (vnext != v) {
                vnext = &(vertices[vnext->next]);
                size++;
            }
            mpoly.Init(size);
            v = &(vertices[i]);
            mpoly[0] = v->p;
            vnext = &(vertices[v->next]);
            size = 1;
            used[i] = 1;
            used[v->next] = 1;
            while (vnext != v) {
                mpoly[size] = vnext->p;
                used[vnext->next] = 1;
                vnext = &(vertices[vnext->next]);
                size++;
            }
            monotonePolys->push_back(mpoly);
        }
    }

    // cleanup
    delete[] vertices;
    delete[] priority;
    delete[] vertextypes;
    delete[] edgeTreeIterators;
    delete[] helpers;
    delete[] used;

    if (error) {
        return 0;
    } else {
        return 1;
    }
}

// adds a diagonal to the doubly-connected list of vertices
void TPPLPartition::AddDiagonal(MonotoneVertex *vertices, s64 *numvertices, s64 index1, s64 index2, char *vertextypes, std::set<ScanLineEdge>::iterator *edgeTreeIterators,
                                std::set<ScanLineEdge> *edgeTree, s64 *helpers) {
    s64 newindex1, newindex2;

    newindex1 = *numvertices;
    (*numvertices)++;
    newindex2 = *numvertices;
    (*numvertices)++;

    vertices[newindex1].p = vertices[index1].p;
    vertices[newindex2].p = vertices[index2].p;

    vertices[newindex2].next = vertices[index2].next;
    vertices[newindex1].next = vertices[index1].next;

    vertices[vertices[index2].next].previous = newindex2;
    vertices[vertices[index1].next].previous = newindex1;

    vertices[index1].next = newindex2;
    vertices[newindex2].previous = index1;

    vertices[index2].next = newindex1;
    vertices[newindex1].previous = index2;

    // update all relevant structures
    vertextypes[newindex1] = vertextypes[index1];
    edgeTreeIterators[newindex1] = edgeTreeIterators[index1];
    helpers[newindex1] = helpers[index1];
    if (edgeTreeIterators[newindex1] != edgeTree->end()) edgeTreeIterators[newindex1]->index = newindex1;
    vertextypes[newindex2] = vertextypes[index2];
    edgeTreeIterators[newindex2] = edgeTreeIterators[index2];
    helpers[newindex2] = helpers[index2];
    if (edgeTreeIterators[newindex2] != edgeTree->end()) edgeTreeIterators[newindex2]->index = newindex2;
}

bool TPPLPartition::PBelow(TPPLPoint &p1, TPPLPoint &p2) {
    if (p1.y < p2.y)
        return true;
    else if (p1.y == p2.y) {
        if (p1.x < p2.x) return true;
    }
    return false;
}

// sorts in the falling order of y values, if y is equal, x is used instead
bool TPPLPartition::VertexSorter::operator()(s64 index1, s64 index2) {
    if (vertices[index1].p.y > vertices[index2].p.y)
        return true;
    else if (vertices[index1].p.y == vertices[index2].p.y) {
        if (vertices[index1].p.x > vertices[index2].p.x) return true;
    }
    return false;
}

bool TPPLPartition::ScanLineEdge::IsConvex(const TPPLPoint &p1, const TPPLPoint &p2, const TPPLPoint &p3) const {
    tppl_float tmp;
    tmp = (p3.y - p1.y) * (p2.x - p1.x) - (p3.x - p1.x) * (p2.y - p1.y);
    if (tmp > 0)
        return 1;
    else
        return 0;
}

bool TPPLPartition::ScanLineEdge::operator<(const ScanLineEdge &other) const {
    if (other.p1.y == other.p2.y) {
        if (p1.y == p2.y) {
            if (p1.y < other.p1.y)
                return true;
            else
                return false;
        }
        if (IsConvex(p1, p2, other.p1))
            return true;
        else
            return false;
    } else if (p1.y == p2.y) {
        if (IsConvex(other.p1, other.p2, p1))
            return false;
        else
            return true;
    } else if (p1.y < other.p1.y) {
        if (IsConvex(other.p1, other.p2, p1))
            return false;
        else
            return true;
    } else {
        if (IsConvex(p1, p2, other.p1))
            return true;
        else
            return false;
    }
}

// triangulates monotone polygon
// O(n) time, O(n) space complexity
s32 TPPLPartition::TriangulateMonotone(TPPLPoly *inPoly, TPPLPolyList *triangles) {
    if (!inPoly->Valid()) return 0;

    s64 i, i2, j, topindex, bottomindex, leftindex, rightindex, vindex;
    TPPLPoint *points = NULL;
    s64 numpoints;
    TPPLPoly triangle;

    numpoints = inPoly->GetNumPoints();
    points = inPoly->GetPoints();

    // trivial case
    if (numpoints == 3) {
        triangles->push_back(*inPoly);
        return 1;
    }

    topindex = 0;
    bottomindex = 0;
    for (i = 1; i < numpoints; i++) {
        if (PBelow(points[i], points[bottomindex])) bottomindex = i;
        if (PBelow(points[topindex], points[i])) topindex = i;
    }

    // check if the poly is really monotone
    i = topindex;
    while (i != bottomindex) {
        i2 = i + 1;
        if (i2 >= numpoints) i2 = 0;
        if (!PBelow(points[i2], points[i])) return 0;
        i = i2;
    }
    i = bottomindex;
    while (i != topindex) {
        i2 = i + 1;
        if (i2 >= numpoints) i2 = 0;
        if (!PBelow(points[i], points[i2])) return 0;
        i = i2;
    }

    char *vertextypes = new char[numpoints];
    s64 *priority = new s64[numpoints];

    // merge left and right vertex chains
    priority[0] = topindex;
    vertextypes[topindex] = 0;
    leftindex = topindex + 1;
    if (leftindex >= numpoints) leftindex = 0;
    rightindex = topindex - 1;
    if (rightindex < 0) rightindex = numpoints - 1;
    for (i = 1; i < (numpoints - 1); i++) {
        if (leftindex == bottomindex) {
            priority[i] = rightindex;
            rightindex--;
            if (rightindex < 0) rightindex = numpoints - 1;
            vertextypes[priority[i]] = -1;
        } else if (rightindex == bottomindex) {
            priority[i] = leftindex;
            leftindex++;
            if (leftindex >= numpoints) leftindex = 0;
            vertextypes[priority[i]] = 1;
        } else {
            if (PBelow(points[leftindex], points[rightindex])) {
                priority[i] = rightindex;
                rightindex--;
                if (rightindex < 0) rightindex = numpoints - 1;
                vertextypes[priority[i]] = -1;
            } else {
                priority[i] = leftindex;
                leftindex++;
                if (leftindex >= numpoints) leftindex = 0;
                vertextypes[priority[i]] = 1;
            }
        }
    }
    priority[i] = bottomindex;
    vertextypes[bottomindex] = 0;

    s64 *stack = new s64[numpoints];
    s64 stackptr = 0;

    stack[0] = priority[0];
    stack[1] = priority[1];
    stackptr = 2;

    // for each vertex from top to bottom trim as many triangles as possible
    for (i = 2; i < (numpoints - 1); i++) {
        vindex = priority[i];
        if (vertextypes[vindex] != vertextypes[stack[stackptr - 1]]) {
            for (j = 0; j < (stackptr - 1); j++) {
                if (vertextypes[vindex] == 1) {
                    triangle.Triangle(points[stack[j + 1]], points[stack[j]], points[vindex]);
                } else {
                    triangle.Triangle(points[stack[j]], points[stack[j + 1]], points[vindex]);
                }
                triangles->push_back(triangle);
            }
            stack[0] = priority[i - 1];
            stack[1] = priority[i];
            stackptr = 2;
        } else {
            stackptr--;
            while (stackptr > 0) {
                if (vertextypes[vindex] == 1) {
                    if (IsConvex(points[vindex], points[stack[stackptr - 1]], points[stack[stackptr]])) {
                        triangle.Triangle(points[vindex], points[stack[stackptr - 1]], points[stack[stackptr]]);
                        triangles->push_back(triangle);
                        stackptr--;
                    } else {
                        break;
                    }
                } else {
                    if (IsConvex(points[vindex], points[stack[stackptr]], points[stack[stackptr - 1]])) {
                        triangle.Triangle(points[vindex], points[stack[stackptr]], points[stack[stackptr - 1]]);
                        triangles->push_back(triangle);
                        stackptr--;
                    } else {
                        break;
                    }
                }
            }
            stackptr++;
            stack[stackptr] = vindex;
            stackptr++;
        }
    }
    vindex = priority[i];
    for (j = 0; j < (stackptr - 1); j++) {
        if (vertextypes[stack[j + 1]] == 1) {
            triangle.Triangle(points[stack[j]], points[stack[j + 1]], points[vindex]);
        } else {
            triangle.Triangle(points[stack[j + 1]], points[stack[j]], points[vindex]);
        }
        triangles->push_back(triangle);
    }

    delete[] priority;
    delete[] vertextypes;
    delete[] stack;

    return 1;
}

s32 TPPLPartition::Triangulate_MONO(TPPLPolyList *inpolys, TPPLPolyList *triangles) {
    TPPLPolyList monotone;
    TPPLPolyList::iterator iter;

    if (!MonotonePartition(inpolys, &monotone)) return 0;
    for (iter = monotone.begin(); iter != monotone.end(); iter++) {
        if (!TriangulateMonotone(&(*iter), triangles)) return 0;
    }
    return 1;
}

s32 TPPLPartition::Triangulate_MONO(TPPLPoly *poly, TPPLPolyList *triangles) {
    TPPLPolyList polys;
    polys.push_back(*poly);

    return Triangulate_MONO(&polys, triangles);
}

#pragma endregion TPPL

void simplify_section(const std::vector<neko_vec2> &pts, f32 tolerance, size_t i, size_t j, std::vector<bool> *mark_map, size_t omitted) {
    // make sure we always return 2 points
    if (pts.size() - omitted <= 2) return;

    assert(mark_map && mark_map->size() == pts.size());

    if ((i + 1) == j) {
        return;
    }

    f32 max_distance = -1.0f;
    size_t max_index = i;

    for (size_t k = i + 1; k < j; k++) {
        f32 distance = pDistance(pts[k].x, pts[k].y, pts[i].x, pts[i].y, pts[j].x, pts[j].y);

        if (distance > max_distance) {
            max_distance = distance;
            max_index = k;
        }
    }

    if (max_distance <= tolerance) {
        for (size_t k = i + 1; k < j; k++) {
            (*mark_map)[k] = false;
            ++omitted;
        }
    } else {
        simplify_section(pts, tolerance, i, max_index, mark_map, omitted);
        simplify_section(pts, tolerance, max_index, j, mark_map, omitted);
    }
}

std::vector<neko_vec2> simplify(const std::vector<neko_vec2> &vertices, f32 tolerance) {
    std::vector<bool> mark_map(vertices.size(), true);

    simplify_section(vertices, tolerance, 0, vertices.size() - 1, &mark_map);

    std::vector<neko_vec2> result;
    for (size_t i = 0; i != vertices.size(); ++i) {
        if (mark_map[i]) {
            result.push_back(vertices[i]);
        }
    }

    return result;
}

f32 pDistance(f32 x, f32 y, f32 x1, f32 y1, f32 x2, f32 y2) {

    f32 A = x - x1;
    f32 B = y - y1;
    f32 C = x2 - x1;
    f32 D = y2 - y1;

    f32 dot = A * C + B * D;
    f32 len_sq = C * C + D * D;
    f32 param = -1;
    if (len_sq != 0)  // in case of 0 length line
        param = dot / len_sq;

    f32 xx, yy;

    if (param < 0) {
        xx = x1;
        yy = y1;
    } else if (param > 1) {
        xx = x2;
        yy = y2;
    } else {
        xx = x1 + param * C;
        yy = y1 + param * D;
    }

    f32 dx = x - xx;
    f32 dy = y - yy;
    return std::sqrt(dx * dx + dy * dy);
}

namespace marching_squares {

bool operator==(const ms_direction &a, const ms_direction &b) { return a.x == b.x && a.y == b.y; }

ms_direction operator*(const ms_direction &direction, s32 multiplier) { return ms_direction(direction.x * multiplier, direction.y * multiplier); }

ms_direction operator+(const ms_direction &a, const ms_direction &b) { return ms_direction(a.x + b.x, a.y + b.y); }

ms_direction &operator+=(ms_direction &a, const ms_direction &b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

ms_direction make_direction(s32 x, s32 y) { return ms_direction(x, y); }

bool is_set(s32 x, s32 y, s32 width, s32 height, unsigned char *data) { return x <= 0 || x > width || y <= 0 || y > height ? false : data[(y - 1) * width + (x - 1)] != 0; }

s32 ms_value(s32 x, s32 y, s32 width, s32 height, unsigned char *data) {
    s32 sum = 0;
    if (is_set(x, y, width, height, data)) sum |= 1;
    if (is_set(x + 1, y, width, height, data)) sum |= 2;
    if (is_set(x, y + 1, width, height, data)) sum |= 4;
    if (is_set(x + 1, y + 1, width, height, data)) sum |= 8;
    return sum;
}

ms_result find_perimeter(s32 initialX, s32 initialY, s32 width, s32 height, unsigned char *data) {

    auto East = []() { return make_direction(1, 0); };
    auto Northeast = []() { return make_direction(1, 1); };
    auto North = []() { return make_direction(0, 1); };
    auto Northwest = []() { return make_direction(-1, 1); };
    auto West = []() { return make_direction(-1, 0); };
    auto Southwest = []() { return make_direction(-1, -1); };
    auto South = []() { return make_direction(0, -1); };
    auto Southeast = []() { return make_direction(1, -1); };

    if (initialX < 0) initialX = 0;
    if (initialX > width) initialX = width;
    if (initialY < 0) initialY = 0;
    if (initialY > height) initialY = height;

    s32 initial_value = ms_value(initialX, initialY, width, height, data);
    if (initial_value == 0 || initial_value == 15) {
        // std::ostringstream error;
        // error << "Supplied initial coordinates (" << initialX << ", " << initialY << ") do not lie on a perimeter.";
        // throw std::runtime_error(error.str());
        ms_result result;
        return result;
    }

    ms_result result;

    s32 x = initialX;
    s32 y = initialY;
    ms_direction previous = make_direction(0, 0);

    do {
        ms_direction direction;
        switch (ms_value(x, y, width, height, data)) {
            case 1:
                direction = North();
                break;
            case 2:
                direction = East();
                break;
            case 3:
                direction = East();
                break;
            case 4:
                direction = West();
                break;
            case 5:
                direction = North();
                break;
            case 6:
                direction = previous == North() ? West() : East();
                break;
            case 7:
                direction = East();
                break;
            case 8:
                direction = South();
                break;
            case 9:
                direction = previous == East() ? North() : South();
                break;
            case 10:
                direction = South();
                break;
            case 11:
                direction = South();
                break;
            case 12:
                direction = West();
                break;
            case 13:
                direction = North();
                break;
            case 14:
                direction = West();
                break;
            default:
                throw std::runtime_error("Illegal state");
        }
        if (direction == previous) {
            // compress
            result.directions.back() += direction;
        } else {
            result.directions.push_back(direction);
            previous = direction;
        }
        x += direction.x;
        y -= direction.y;  // accommodate change of basis
    } while (x != initialX || y != initialY);

    result.initial_x = initialX;
    result.initial_y = initialY;

    return result;
}

ms_result find_perimeter(s32 width, s32 height, unsigned char *data) {
    s32 size = width * height;
    for (s32 i = 0; i < size; i++) {
        if (data[i] != 0) {
            return find_perimeter(i % width, i / width, width, height, data);
        }
    }
    ms_result result;
    return result;
}

ms_result find_perimeter(s32 width, s32 height, unsigned char *data, s32 lookX, s32 lookY) {
    s32 size = width * height;
    for (s32 i = lookX + lookY * width; i < size; i++) {
        if (data[i] != 0) {
            // std::cout << (i%width) << " " << (i / width) << std::endl;
            return find_perimeter(i % width, i / width, width, height, data);
        }
    }
    ms_result result;
    return result;
}

ms_direction find_edge(s32 width, s32 height, unsigned char *data, s32 lookX, s32 lookY) {
    s32 size = width * height;
    for (s32 i = lookX + lookY * width; i < size; i++) {
        if (data[i] != 0) {
            // std::cout << (i%width) << " " << (i / width) << std::endl;
            s32 val = ms_value(i % width, i / width, width, height, data);
            if (val != 0 && val != 15) {
                return {i % width, i / width};
            }
        }
    }
    return {-1, -1};
}

}  // namespace marching_squares

}  // namespace neko

///////////////////////////////////////////////
//
//  位图边缘检测算法

#if 0

static neko_vec2 last_point;

void render_test() {

    u8* alpha = neko_tex_rgba_to_alpha((u8*)g_texture_buffer, ch->chunk_w, ch->chunk_h);
    u8* thresholded = neko_tex_alpha_to_thresholded(alpha, ch->chunk_w, ch->chunk_h, 90);
    u8* outlined = neko_tex_thresholded_to_outlined(thresholded, ch->chunk_w, ch->chunk_h);
    neko_safe_free(alpha);
    neko_safe_free(thresholded);

    neko_tex_point* outline = neko_tex_extract_outline_path(outlined, ch->chunk_w, ch->chunk_h, &l, 0);
    while (l) {
        s32 l0 = l;
        neko_tex_distance_based_path_simplification(outline, &l, 0.5f);
        // printf("simplified outline: %d -> %d\n", l0, l);

        l_check = l;

        for (s32 i = 0; i < l; i++) {
            // gfx->immediate.draw_line_ext(cb, neko_vec2_mul(last_point, {4.f, 4.f}), neko_vec2_mul(neko_vec2{(f32)outline[i].x, (f32)outline[i].y}, {4.f, 4.f}), 2.f, neko_color_white);

            last_point = {(f32)outline[i].x, (f32)outline[i].y};
        }

        outline = neko_tex_extract_outline_path(outlined, ch->chunk_w, ch->chunk_h, &l, outline);
    };

    neko_safe_free(outline);
    neko_safe_free(outlined);
}

#endif

// image manipulation functions
u8 *neko_tex_rgba_to_alpha(const u8 *data, s32 w, s32 h) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    s32 x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[(y * w + x) * 4 + 3];
    return result;
}

u8 *neko_tex_alpha_to_thresholded(const u8 *data, s32 w, s32 h, u8 threshold) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    s32 x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[y * w + x] >= threshold ? 255 : 0;
    return result;
}

u8 *neko_tex_dilate_thresholded(const u8 *data, s32 w, s32 h) {
    s32 x, y, dx, dy, cx, cy;
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            result[y * w + x] = 0;
            for (dy = -1; dy <= 1; dy++) {
                for (dx = -1; dx <= 1; dx++) {
                    cx = x + dx;
                    cy = y + dy;
                    if (cx >= 0 && cx < w && cy >= 0 && cy < h) {
                        if (data[cy * w + cx]) {
                            result[y * w + x] = 255;
                            dy = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    return result;
}

u8 *neko_tex_thresholded_to_outlined(const u8 *data, s32 w, s32 h) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    s32 x, y;
    for (x = 0; x < w; x++) {
        result[x] = data[x];
        result[(h - 1) * w + x] = data[(h - 1) * w + x];
    }
    for (y = 1; y < h - 1; y++) {
        result[y * w] = data[y * w];
        for (x = 1; x < w - 1; x++) {
            if (data[y * w + x] && (!data[y * w + x - 1] || !data[y * w + x + 1] || !data[y * w + x - w] || !data[y * w + x + w])) {
                result[y * w + x] = 255;
            } else {
                result[y * w + x] = 0;
            }
        }
        result[y * w + w - 1] = data[y * w + w - 1];
    }
    return result;
}

// outline path procedures
static neko_tex_bool neko_tex_find_first_filled_pixel(const u8 *data, s32 w, s32 h, neko_tex_point *first) {
    s32 x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (data[y * w + x]) {
                first->x = (short)x;
                first->y = (short)y;
                return 1;
            }
        }
    }
    return 0;
}

static neko_tex_bool neko_tex_find_next_filled_pixel(const u8 *data, s32 w, s32 h, neko_tex_point current, neko_tex_direction *dir, neko_tex_point *next) {
    // turn around 180°, then make a clockwise scan for a filled pixel
    *dir = __neko_tex_direction_opposite(*dir);
    s32 i;
    for (i = 0; i < 8; i++) {
        __neko_tex_point_add(*next, current, neko_tex_direction_to_pixel_offset[*dir]);

        if (__neko_tex_point_is_inside(*next, w, h) && data[next->y * w + next->x]) return 1;

        // move to next angle (clockwise)
        *dir = *dir - 1;
        if (*dir < 0) *dir = 7;
    }
    return 0;
}

neko_tex_point *neko_tex_extract_outline_path(u8 *data, s32 w, s32 h, s32 *point_count, neko_tex_point *reusable_outline) {
    neko_tex_point *outline = reusable_outline;
    if (!outline) outline = (neko_tex_point *)neko_safe_malloc(w * h * sizeof(neko_tex_point));

    neko_tex_point current, next;

restart:
    if (!neko_tex_find_first_filled_pixel(data, w, h, &current)) {
        *point_count = 0;
        return outline;
    }

    s32 count = 0;
    neko_tex_direction dir = 0;

    while (__neko_tex_point_is_inside(current, w, h)) {
        data[current.y * w + current.x] = 0;  // clear the visited path
        outline[count++] = current;           // add our current point to the outline
        if (!neko_tex_find_next_filled_pixel(data, w, h, current, &dir, &next)) {
            // find loop connection
            neko_tex_bool found = 0;
            s32 i;
            for (i = 0; i < count / 2; i++)  // only allow big loops
            {
                if (__neko_tex_point_is_next_to(current, outline[i])) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                break;
            } else {
                // go backwards until we see outline pixels again
                dir = __neko_tex_direction_opposite(dir);
                count--;  // back up
                s32 prev;
                for (prev = count; prev >= 0; prev--) {
                    current = outline[prev];
                    outline[count++] = current;  // add our current point to the outline again
                    if (neko_tex_find_next_filled_pixel(data, w, h, current, &dir, &next)) break;
                }
            }
        }
        current = next;
    }

    if (count <= 2)  // too small, discard and try again!
        goto restart;
    *point_count = count;
    return outline;
}

void neko_tex_distance_based_path_simplification(neko_tex_point *outline, s32 *outline_length, f32 distance_threshold) {
    s32 length = *outline_length;
    s32 l;
    for (l = length / 2 /*length - 1*/; l > 1; l--) {
        s32 a, b = l;
        for (a = 0; a < length; a++) {
            neko_tex_point ab;
            __neko_tex_point_sub(ab, outline[b], outline[a]);
            f32 lab = sqrtf((f32)(ab.x * ab.x + ab.y * ab.y));
            f32 ilab = 1.0f / lab;
            f32 abnx = ab.x * ilab, abny = ab.y * ilab;

            if (lab != 0.0f) {
                neko_tex_bool found = 1;
                s32 i = (a + 1) % length;
                while (i != b) {
                    neko_tex_point ai;
                    __neko_tex_point_sub(ai, outline[i], outline[a]);
                    f32 t = (abnx * ai.x + abny * ai.y) * ilab;
                    f32 distance = -abny * ai.x + abnx * ai.y;
                    if (t < 0.0f || t > 1.0f || distance > distance_threshold || -distance > distance_threshold) {
                        found = 0;
                        break;
                    }

                    if (++i == length) i = 0;
                }

                if (found) {
                    s32 i;
                    if (a < b) {
                        for (i = 0; i < length - b; i++) outline[a + i + 1] = outline[b + i];
                        length -= b - a - 1;
                    } else {
                        length = a - b + 1;
                        for (i = 0; i < length; i++) outline[i] = outline[b + i];
                    }
                    if (l >= length) l = length - 1;
                }
            }

            if (++b >= length) b = 0;
        }
    }
    *outline_length = length;
}

#if 1

// color brightness as perceived:
f32 brightness(neko_color_t c) { return ((f32)c.r * 0.299f + (f32)c.g * 0.587f + (f32)c.b * 0.114f) / 256.f; }

f32 color_num(neko_color_t c) {
    const f32 bright_factor = 100.0f;
    const f32 sat_factor = 0.1f;
    neko_hsv_t hsv = neko_rgb_to_hsv(c);
    return hsv.s * sat_factor + brightness(c) * bright_factor;
}

// particle_t get_closest_particle_from_color(neko_color_t c) {
//     particle_t p = particle_empty();
//     f32 min_dist = f32_max;
//     neko_vec4 c_vec = neko_vec4{(f32)c.r, (f32)c.g, (f32)c.b, (f32)c.a};
//     u8 id = mat_id_empty;
//
//     __check_dist_euclidean(c, mat_col_sand, particle_sand);
//     __check_dist_euclidean(c, mat_col_water, particle_water);
//     __check_dist_euclidean(c, mat_col_salt, particle_salt);
//     __check_dist_euclidean(c, mat_col_wood, particle_wood);
//     __check_dist_euclidean(c, mat_col_fire, particle_fire);
//     __check_dist_euclidean(c, mat_col_smoke, particle_smoke);
//     __check_dist_euclidean(c, mat_col_steam, particle_steam);
//     __check_dist_euclidean(c, mat_col_gunpowder, particle_gunpowder);
//     __check_dist_euclidean(c, mat_col_oil, particle_oil);
//     __check_dist_euclidean(c, mat_col_lava, particle_lava);
//     __check_dist_euclidean(c, mat_col_stone, particle_stone);
//     __check_dist_euclidean(c, mat_col_acid, particle_acid);
//
//     return p;
// }

#if 0
void drop_file_callback(void* platform_window, s32 count, const char** file_paths) {
    if (count < 1) return;

    // Just do first one for now...
    if (count > 1) count = 1;

    // We'll place at the mouse position as well, for shiggles
    neko_vec2 mp = calculate_mouse_position();

    for (s32 i = 0; i < count; ++i) {
        // Need to verify this IS an image first.
        char temp_file_extension_buffer[16] = {0};
        neko_util_get_file_extension(temp_file_extension_buffer, sizeof(temp_file_extension_buffer), file_paths[0]);
        if (neko_string_compare_equal(temp_file_extension_buffer, "png") || neko_string_compare_equal(temp_file_extension_buffer, "jpg") ||
            neko_string_compare_equal(temp_file_extension_buffer, "jpeg") || neko_string_compare_equal(temp_file_extension_buffer, "bmp")) {
            // Load texture into memory
            s32 _w, _h;
            u32 _n;
            void* texture_data = NULL;

            // Force texture data to 3 components
            neko_util_load_texture_data_from_file(file_paths[i], &_w, &_h, &_n, &texture_data, false);
            _n = 3;

            // Not sure what the format should be, so this is ...blah. Need to find a way to determine this beforehand.
            u8* data = (u8*)texture_data;

            s32 sx = (ch->render_w - _w) / 2;
            s32 sy = (ch->render_h - _h) / 2;

            // Now we need to process the data and place it into our particle/color buffers
            for (u32 h = 0; h < _h; ++h) {
                for (u32 w = 0; w < _w; ++w) {
                    neko_color_t c = {data[(h * _w + w) * _n + 0], data[(h * _w + w) * _n + 1], data[(h * _w + w) * _n + 2], 255};

                    // Get color of this pixel in the image
                    particle_t p = get_closest_particle_from_color(c);

                    // chunk_update_mask_t* chunk = neko_hash_table_getp(ch->chunk_data, 0);

                    // Let's place this thing in the center instead...
                    if (in_bounds(sx + w, sy + h)) {

                        u32 idx = compute_idx(sx + w, sy + h);

                        chunk_write_data(ch,idx, p);
                    }
                }
            }

            // Free texture data
            free(texture_data);
        }
    }
}
#endif

#endif

#if 0

#define STB_HBWANG_RAND() neko_rand_xorshf32()
#define STB_HBWANG_IMPLEMENTATION
#include "deps/stb/stb_herringbone_wang_tile.h"
#include "deps/stb/stb_image.h"
#include "deps/stb/stb_image_write.h"

void genwang(std::string filename, unsigned char* data, s32 xs, s32 ys, s32 w, s32 h) {
    stbhw_tileset ts;
    if (xs < 1 || xs > 1000) {
        fprintf(stderr, "xsize invalid or out of range\n");
        exit(1);
    }
    if (ys < 1 || ys > 1000) {
        fprintf(stderr, "ysize invalid or out of range\n");
        exit(1);
    }

    stbhw_build_tileset_from_image(&ts, data, w * 3, w, h);
    // allocate a buffer to create the final image to
    s32 yimg = ys + 4;
    auto buff = static_cast<unsigned char*>(malloc(3 * xs * yimg));
    stbhw_generate_image(&ts, NULL, buff, xs * 3, xs, yimg);
    stbi_write_png(filename.c_str(), xs, yimg, 3, buff, xs * 3);
    stbhw_free_tileset(&ts);
    free(buff);
}

void test_wang() {

    // mapgen {tile-file} {xsize} {ysize} {seed} [n]

    s32 xs = 128;
    s32 ys = 128;

    s32 n = 1;

    s32 w, h;

    unsigned char* data = stbi_load(neko_file_path("gamedir/assets/textures/wang_test.png"), &w, &h, NULL, 3);

    neko_assert(data);

    printf("Output size: %dx%d\n", xs, ys);

    {
        u32 seed = neko_rand_xorshf32();

        // s32 num = seed + xs + 11 * (xs / -11) - 12 * (seed / 12);
        s32 num = xs % 11 + seed % 12;

        for (s32 i = 0; i < n; ++i) {

            auto filename = std::string("output_wang");
            filename = filename.substr(0, filename.size() - 4);
            filename = filename + std::to_string(seed) + "#" + std::to_string(i) + ".png";

            genwang(filename, data, xs, ys, w, h);
        }
    }

    free(data);
}

#endif