

#include "engine/physics/neko_phy.h"

#include "engine/common/neko_util.h"

typedef std::map<neko_phy_arbiter_key, neko_phy_arbiter>::iterator neko_phy_arbiter_itor;
typedef std::pair<neko_phy_arbiter_key, neko_phy_arbiter> neko_phy_arbiter_pair;

bool accumulate_impulses = true;  // 累加冲量
bool do_hot_starting = true;      // 热启动 缓存计算中间成果
bool position_correction = true;  // 位置校准 配套的抗抖动

void neko_phy_world::neko_phy_create(neko_phy_body *body) { bodies.push_back(body); }

void neko_phy_world::neko_phy_create(neko_phy_joint *joint) { joints.push_back(joint); }

void neko_phy_world::neko_phy_clear() {
    bodies.clear();
    joints.clear();
    arbiters.clear();
}

// 遍历所有物体 把发生接触了的挑出来
void neko_phy_world::neko_phy_broadphase() {
    // O(n^2) broad-phase
    for (s32 i = 0; i < (s32)bodies.size(); ++i) {
        neko_phy_body *bi = bodies[i];

        for (s32 j = i + 1; j < (s32)bodies.size(); ++j) {
            neko_phy_body *bj = bodies[j];

            // 两个非动力学物体之间不发生碰撞
            if (bi->invMass == 0.0f && bj->invMass == 0.0f) continue;

            neko_phy_arbiter newArb(bi, bj);
            neko_phy_arbiter_key key(bi, bj);  // 显然是先做个排序然后生成key, 来避免两个物体顺序的影响

            if (newArb.numContacts > 0)  // 挑出来碰撞了的物体
            {
                // 去掉重复的pair
                neko_phy_arbiter_itor iter = arbiters.find(key);  // Map.find(key), 找不到就返回尾部迭代器
                if (iter == arbiters.end()) {
                    arbiters.insert(neko_phy_arbiter_pair(key, newArb));
                } else {
                    // first是key, second是value
                    iter->second.step(newArb.contacts, newArb.numContacts);
                }
            } else {
                arbiters.erase(key);
            }
        }
    }
}

void neko_phy_world::neko_phy_step(f32 dt) {
    f32 inv_dt = dt > 0.0f ? 1.0f / dt : 0.0f;

    // Determine overlapping(重叠) bodies and update contact points.
    neko_phy_broadphase();

    // Integrate(整合) forces. // 把力转换为速度
    // 生成力然后计算速度, 为嘛不用冲量?
    for (s32 i = 0; i < (s32)bodies.size(); ++i) {
        neko_phy_body *b = bodies[i];

        if (b->invMass == 0.0f) continue;

        // inv 是倒数.....
        b->velocity += dt * (gravity + b->invMass * b->force);
        b->angularVelocity += dt * b->invI * b->torque;  // 角速度
    }

    // Perform pre-steps.
    for (neko_phy_arbiter_itor arb = arbiters.begin(); arb != arbiters.end(); ++arb) {
        // 看来每一个Arbiter都有一个PreStep, 应该是用来做位置校准和抗抖动的吧
        arb->second.prestep(inv_dt);
    }

    for (s32 i = 0; i < (s32)joints.size(); ++i) {
        // 原来每一个关节也有呀
        joints[i]->prestep(inv_dt);
    }

    // Perform iterations
    for (s32 i = 0; i < iterations; ++i) {
        // 动力学的核心部分
        // 看来作者让force和impulse使用在了不同的场景
        // 按照常理 刚体碰撞应该是只能用impulse去解(因为碰撞力无限大, 碰撞时间无限小)
        // 力估计是用来处理堆叠稳定性的
        for (neko_phy_arbiter_itor arb = arbiters.begin(); arb != arbiters.end(); ++arb) {
            arb->second.apply_impulse();
        }

        for (s32 j = 0; j < (s32)joints.size(); ++j) {
            // 看来关节也只能用impulse
            // 如果用力的话就变成弹簧了
            // (弹性连接比关节,绳子,木棍等链接要容易很多)
            joints[j]->apply_impulse();
        }
    }

    // Integrate Velocities
    // 位移生效
    for (s32 i = 0; i < (s32)bodies.size(); ++i) {
        neko_phy_body *b = bodies[i];

        b->position += dt * b->velocity;
        b->rotation += dt * b->angularVelocity;

        b->force.set(0.0f, 0.0f);
        b->torque = 0.0f;  // 扭矩
    }
}

// 碰撞检测 (SAT分离轴定理)
// 生成碰撞流形 (SAT+剪裁)

// Box vertex and edge numbering:
//
//        ^ y
//        |
//        e1
//   v2 ------ v1
//    |        |
// e2 |        | e4  --> x
//    |        |
//   v3 ------ v4
//        e3

enum neko_phy_axis { FACE_A_X, FACE_A_Y, FACE_B_X, FACE_B_Y };

enum neko_phy_edgenumbers { NO_EDGE = 0, EDGE1, EDGE2, EDGE3, EDGE4 };

// 夹顶点
struct neko_phy_clip_vertex {
    neko_vec2 v;
    neko_phy_feature_pair fp = {.value = 0};
};

// 翻转
void neko_phy_flip(neko_phy_feature_pair &fp) {
    neko_swap(fp.e.inEdge1, fp.e.inEdge2);
    neko_swap(fp.e.outEdge1, fp.e.outEdge2);
}

// 把线段切到线上
s32 neko_phy_clip_segment_toline(neko_phy_clip_vertex vOut[2], neko_phy_clip_vertex vIn[2], const neko_vec2 &normal, f32 offset, char clipEdge) {
    // Start with no output points
    s32 numOut = 0;

    // Calculate the distance of end points to the line
    f32 distance0 = neko_vec2_dot(normal, vIn[0].v) - offset;
    f32 distance1 = neko_vec2_dot(normal, vIn[1].v) - offset;

    // If the points are behind the plane // 哪来的平面???
    if (distance0 <= 0.0f) vOut[numOut++] = vIn[0];
    if (distance1 <= 0.0f) vOut[numOut++] = vIn[1];

    // If the points are on different sides of the plane
    if (distance0 * distance1 < 0.0f) {
        // Find intersection point交叉点 of edge and plane
        f32 interp = distance0 / (distance0 - distance1);
        vOut[numOut].v = vIn[0].v + interp * (vIn[1].v - vIn[0].v);
        if (distance0 > 0.0f) {
            vOut[numOut].fp = vIn[0].fp;
            vOut[numOut].fp.e.inEdge1 = clipEdge;
            vOut[numOut].fp.e.inEdge2 = NO_EDGE;
        } else {
            vOut[numOut].fp = vIn[1].fp;
            vOut[numOut].fp.e.outEdge1 = clipEdge;
            vOut[numOut].fp.e.outEdge2 = NO_EDGE;
        }
        ++numOut;
    }

    return numOut;
}

static void neko_phy_compute_incident_edge(neko_phy_clip_vertex c[2], const neko_vec2 &h, const neko_vec2 &pos, const neko_mat22 &Rot, const neko_vec2 &normal) {
    // The normal is from the reference box. Convert it
    // to the incident boxe's frame and flip sign.
    neko_mat22 RotT = neko_mat22_transpose(Rot);
    neko_vec2 n = -(RotT * normal);
    neko_vec2 nAbs = neko_vec2_abs(n);

    if (nAbs.x > nAbs.y) {
        if (neko_sign(n.x) > 0.0f) {
            c[0].v.set(h.x, -h.y);
            c[0].fp.e.inEdge2 = EDGE3;
            c[0].fp.e.outEdge2 = EDGE4;

            c[1].v.set(h.x, h.y);
            c[1].fp.e.inEdge2 = EDGE4;
            c[1].fp.e.outEdge2 = EDGE1;
        } else {
            c[0].v.set(-h.x, h.y);
            c[0].fp.e.inEdge2 = EDGE1;
            c[0].fp.e.outEdge2 = EDGE2;

            c[1].v.set(-h.x, -h.y);
            c[1].fp.e.inEdge2 = EDGE2;
            c[1].fp.e.outEdge2 = EDGE3;
        }
    } else {
        if (neko_sign(n.y) > 0.0f) {
            c[0].v.set(h.x, h.y);
            c[0].fp.e.inEdge2 = EDGE4;
            c[0].fp.e.outEdge2 = EDGE1;

            c[1].v.set(-h.x, h.y);
            c[1].fp.e.inEdge2 = EDGE1;
            c[1].fp.e.outEdge2 = EDGE2;
        } else {
            c[0].v.set(-h.x, -h.y);
            c[0].fp.e.inEdge2 = EDGE2;
            c[0].fp.e.outEdge2 = EDGE3;

            c[1].v.set(h.x, -h.y);
            c[1].fp.e.inEdge2 = EDGE3;
            c[1].fp.e.outEdge2 = EDGE4;
        }
    }

    c[0].v = pos + Rot * c[0].v;
    c[1].v = pos + Rot * c[1].v;
}

// 竟然用分离轴....
// The normal points from A to B (惯例, 法线都设为从A指向B)
s32 neko_phy_collide(neko_phy_contact *contacts, neko_phy_body *bodyA, neko_phy_body *bodyB) {
    // Setup
    neko_vec2 hA = 0.5f * bodyA->width;
    neko_vec2 hB = 0.5f * bodyB->width;

    neko_vec2 posA = bodyA->position;
    neko_vec2 posB = bodyB->position;

    neko_mat22 RotA = neko_mat22_ctor(bodyA->rotation), RotB = neko_mat22_ctor(bodyB->rotation);

    neko_mat22 RotAT = neko_mat22_transpose(RotA);
    neko_mat22 RotBT = neko_mat22_transpose(RotB);

    neko_vec2 dp = posB - posA;
    neko_vec2 dA = RotAT * dp;
    neko_vec2 dB = RotBT * dp;

    neko_mat22 C = RotAT * RotB;
    neko_mat22 absC = neko_mat22_abs(C);
    neko_mat22 absCT = neko_mat22_transpose(absC);

    // Box A faces
    neko_vec2 faceA = neko_vec2_abs(dA) - hA - absC * hB;
    if (faceA.x > 0.0f || faceA.y > 0.0f) return 0;

    // Box B faces
    neko_vec2 faceB = neko_vec2_abs(dB) - absCT * hA - hB;
    if (faceB.x > 0.0f || faceB.y > 0.0f) return 0;

    // Find best axis
    neko_phy_axis axis;
    f32 separation;
    neko_vec2 normal;

    // Box A faces
    axis = FACE_A_X;
    separation = faceA.x;
    normal = dA.x > 0.0f ? RotA.col1 : -RotA.col1;

    const f32 relativeTol = 0.95f;
    const f32 absoluteTol = 0.01f;

    if (faceA.y > relativeTol * separation + absoluteTol * hA.y) {
        axis = FACE_A_Y;
        separation = faceA.y;
        normal = dA.y > 0.0f ? RotA.col2 : -RotA.col2;
    }

    // Box B faces
    if (faceB.x > relativeTol * separation + absoluteTol * hB.x) {
        axis = FACE_B_X;
        separation = faceB.x;
        normal = dB.x > 0.0f ? RotB.col1 : -RotB.col1;
    }

    if (faceB.y > relativeTol * separation + absoluteTol * hB.y) {
        axis = FACE_B_Y;
        separation = faceB.y;
        normal = dB.y > 0.0f ? RotB.col2 : -RotB.col2;
    }

    // Setup clipping plane data based on the separating axis
    neko_vec2 frontNormal, sideNormal;
    neko_phy_clip_vertex incidentEdge[2];
    f32 front, negSide, posSide;
    char negEdge, posEdge;

    // Compute the clipping lines and the line segment to be clipped.
    switch (axis) {
        case FACE_A_X: {
            frontNormal = normal;
            front = neko_vec2_dot(posA, frontNormal) + hA.x;
            sideNormal = RotA.col2;
            f32 side = neko_vec2_dot(posA, sideNormal);
            negSide = -side + hA.y;
            posSide = side + hA.y;
            negEdge = EDGE3;
            posEdge = EDGE1;
            neko_phy_compute_incident_edge(incidentEdge, hB, posB, RotB, frontNormal);
        } break;

        case FACE_A_Y: {
            frontNormal = normal;
            front = neko_vec2_dot(posA, frontNormal) + hA.y;
            sideNormal = RotA.col1;
            f32 side = neko_vec2_dot(posA, sideNormal);
            negSide = -side + hA.x;
            posSide = side + hA.x;
            negEdge = EDGE2;
            posEdge = EDGE4;
            neko_phy_compute_incident_edge(incidentEdge, hB, posB, RotB, frontNormal);
        } break;

        case FACE_B_X: {
            frontNormal = -normal;
            front = neko_vec2_dot(posB, frontNormal) + hB.x;
            sideNormal = RotB.col2;
            f32 side = neko_vec2_dot(posB, sideNormal);
            negSide = -side + hB.y;
            posSide = side + hB.y;
            negEdge = EDGE3;
            posEdge = EDGE1;
            neko_phy_compute_incident_edge(incidentEdge, hA, posA, RotA, frontNormal);
        } break;

        case FACE_B_Y: {
            frontNormal = -normal;
            front = neko_vec2_dot(posB, frontNormal) + hB.y;
            sideNormal = RotB.col1;
            f32 side = neko_vec2_dot(posB, sideNormal);
            negSide = -side + hB.x;
            posSide = side + hB.x;
            negEdge = EDGE2;
            posEdge = EDGE4;
            neko_phy_compute_incident_edge(incidentEdge, hA, posA, RotA, frontNormal);
        } break;
    }

    // clip other face with 5 box planes (1 face plane, 4 edge planes)

    neko_phy_clip_vertex clipPoints1[2];
    neko_phy_clip_vertex clipPoints2[2];
    s32 np;

    // Clip to box side 1
    np = neko_phy_clip_segment_toline(clipPoints1, incidentEdge, -sideNormal, negSide, negEdge);

    if (np < 2) return 0;

    // Clip to negative box side 1
    np = neko_phy_clip_segment_toline(clipPoints2, clipPoints1, sideNormal, posSide, posEdge);

    if (np < 2) return 0;

    // Now clipPoints2 contains the clipping points.
    // Due to roundoff, it is possible that clipping removes all points.

    s32 numContacts = 0;
    for (s32 i = 0; i < 2; ++i) {
        f32 separation = neko_vec2_dot(frontNormal, clipPoints2[i].v) - front;

        if (separation <= 0) {
            contacts[numContacts].separation = separation;
            contacts[numContacts].normal = normal;
            // slide contact point onto reference face (easy to cull)
            contacts[numContacts].position = clipPoints2[i].v - separation * frontNormal;
            contacts[numContacts].feature = clipPoints2[i].fp;
            if (axis == FACE_B_X || axis == FACE_B_Y) neko_phy_flip(contacts[numContacts].feature);
            ++numContacts;
        }
    }

    return numContacts;
}
