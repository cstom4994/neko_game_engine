

#ifndef NEKO_PHY_H
#define NEKO_PHY_H

#include <map>
#include <vector>

#include "engine/math/neko_math.h"

extern bool accumulate_impulses;
extern bool do_hot_starting;
extern bool position_correction;

neko_inline constexpr f32 neko_mass_max = f32_max;
neko_inline constexpr f32 neko_position_correction_size = 0.4f;  // 碰撞修正量

typedef struct neko_phy_body {
    neko_phy_body() {
        position.set(0.0f, 0.0f);
        rotation = 0.0f;
        velocity.set(0.0f, 0.0f);
        angularVelocity = 0.0f;
        force.set(0.0f, 0.0f);
        torque = 0.0f;
        friction = 0.2f;

        width.set(1.0f, 1.0f);
        mass = neko_mass_max;
        invMass = 0.0f;
        I = neko_mass_max;
        invI = 0.0f;
    }

    neko_vec2 position;
    f32 rotation;

    neko_vec2 velocity;
    f32 angularVelocity;

    neko_vec2 force;
    f32 torque;

    neko_vec2 width;

    f32 friction;
    f32 mass, invMass;
    f32 I, invI;
} neko_phy_body;

neko_inline void neko_body_add_force(neko_phy_body *body, const neko_vec2 &f) { body->force += f; }

neko_inline void neko_body_set(neko_phy_body *body, const neko_vec2 &w, f32 m) {
    body->position.set(0.0f, 0.0f);
    body->rotation = 0.0f;
    body->velocity.set(0.0f, 0.0f);
    body->angularVelocity = 0.0f;
    body->force.set(0.0f, 0.0f);
    body->torque = 0.0f;
    body->friction = 0.2f;

    body->width = w;
    body->mass = m;

    if (body->mass < neko_mass_max) {
        body->invMass = 1.0f / body->mass;
        body->I = body->mass * (body->width.x * body->width.x + body->width.y * body->width.y) / 12.0f;
        body->invI = 1.0f / body->I;
    } else {
        body->invMass = 0.0f;
        body->I = neko_mass_max;
        body->invI = 0.0f;
    }
}

struct neko_phy_joint {
    neko_phy_joint() : body1(0), body2(0), P{0.0f, 0.0f}, biasFactor(0.2f), softness(0.0f) {}

    void Set(neko_phy_body *b1, neko_phy_body *b2, const neko_vec2 &anchor) {
        body1 = b1;
        body2 = b2;

        neko_mat22 Rot1 = neko_mat22_ctor(body1->rotation);
        neko_mat22 Rot2 = neko_mat22_ctor(body2->rotation);
        neko_mat22 Rot1T = neko_mat22_transpose(Rot1);
        neko_mat22 Rot2T = neko_mat22_transpose(Rot2);

        localAnchor1 = Rot1T * (anchor - body1->position);
        localAnchor2 = Rot2T * (anchor - body2->position);

        P.set(0.0f, 0.0f);

        softness = 0.0f;
        biasFactor = 0.2f;
    }

    void prestep(f32 inv_dt) {
        // 预先计算锚点 质量矩阵和偏差
        neko_mat22 Rot1 = neko_mat22_ctor(body1->rotation);
        neko_mat22 Rot2 = neko_mat22_ctor(body2->rotation);

        r1 = Rot1 * localAnchor1;
        r2 = Rot2 * localAnchor2;

        // deltaV = deltaV0 + K * impulse
        // invM = [(1/m1 + 1/m2) * eye(2) - skew(r1) * invI1 * skew(r1) - skew(r2) * invI2 * skew(r2)]
        //      = [1/m1+1/m2     0    ] + invI1 * [r1.y*r1.y -r1.x*r1.y] + invI2 * [r1.y*r1.y -r1.x*r1.y]
        //        [    0     1/m1+1/m2]           [-r1.x*r1.y r1.x*r1.x]           [-r1.x*r1.y r1.x*r1.x]
        neko_mat22 K1;
        K1.col1.x = body1->invMass + body2->invMass;
        K1.col2.x = 0.0f;
        K1.col1.y = 0.0f;
        K1.col2.y = body1->invMass + body2->invMass;

        neko_mat22 K2;
        K2.col1.x = body1->invI * r1.y * r1.y;
        K2.col2.x = -body1->invI * r1.x * r1.y;
        K2.col1.y = -body1->invI * r1.x * r1.y;
        K2.col2.y = body1->invI * r1.x * r1.x;

        neko_mat22 K3;
        K3.col1.x = body2->invI * r2.y * r2.y;
        K3.col2.x = -body2->invI * r2.x * r2.y;
        K3.col1.y = -body2->invI * r2.x * r2.y;
        K3.col2.y = body2->invI * r2.x * r2.x;

        neko_mat22 K = K1 + K2 + K3;
        K.col1.x += softness;
        K.col2.y += softness;

        M = neko_mat22_invert(K);

        neko_vec2 p1 = body1->position + r1;
        neko_vec2 p2 = body2->position + r2;
        neko_vec2 dp = p2 - p1;

        if (position_correction) {
            bias = -biasFactor * inv_dt * dp;
        } else {
            bias.set(0.0f, 0.0f);
        }

        if (do_hot_starting) {
            // Apply accumulated impulse.
            body1->velocity -= body1->invMass * P;
            body1->angularVelocity -= body1->invI * neko_vec2_cross(r1, P);

            body2->velocity += body2->invMass * P;
            body2->angularVelocity += body2->invI * neko_vec2_cross(r2, P);
        } else {
            P.set(0.0f, 0.0f);
        }
    }

    void apply_impulse() {
        neko_vec2 dv = body2->velocity + neko_vec2_cross(body2->angularVelocity, r2) - body1->velocity - neko_vec2_cross(body1->angularVelocity, r1);

        neko_vec2 impulse;

        impulse = M * (bias - dv - softness * P);

        body1->velocity -= body1->invMass * impulse;
        body1->angularVelocity -= body1->invI * neko_vec2_cross(r1, impulse);

        body2->velocity += body2->invMass * impulse;
        body2->angularVelocity += body2->invI * neko_vec2_cross(r2, impulse);

        P += impulse;
    }

    neko_mat22 M;
    neko_vec2 localAnchor1, localAnchor2;
    neko_vec2 r1, r2;
    neko_vec2 bias;
    neko_vec2 P;  // accumulated impulse
    neko_phy_body *body1;
    neko_phy_body *body2;
    f32 biasFactor;
    f32 softness;
};

// 特征对
union neko_phy_feature_pair {
    struct ddges {
        char inEdge1;
        char outEdge1;
        char inEdge2;
        char outEdge2;
    } e;  // ???
    s32 value;
};

struct neko_phy_contact {
    neko_phy_contact() : Pn(0.0f), Pt(0.0f), Pnb(0.0f) {}

    neko_vec2 position;  //
    neko_vec2 normal;
    neko_vec2 r1, r2;  // 碰撞半径, 碰撞点到物体重心的距离
    f32 separation;
    f32 Pn;   // accumulated normal impulse  法线冲量
    f32 Pt;   // accumulated tangent impulse 切线冲量
    f32 Pnb;  // accumulated normal impulse for position bias 法线位置偏移冲量
    f32 massNormal, massTangent;
    f32 bias;
    neko_phy_feature_pair feature;
};

struct neko_phy_arbiter_key {
    neko_phy_arbiter_key(neko_phy_body *b1, neko_phy_body *b2) {
        if (b1 < b2)  // 这个就非常玄学
        {
            body1 = b1;
            body2 = b2;
        } else {
            body1 = b2;
            body2 = b1;
        }
    }

    neko_phy_body *body1;
    neko_phy_body *body2;
};

s32 neko_phy_collide(neko_phy_contact *contacts, neko_phy_body *body1, neko_phy_body *body2);

struct neko_phy_arbiter {
    enum { MAX_POINTS = 2 };  // 流形最多两个点

    neko_phy_arbiter(neko_phy_body *b1, neko_phy_body *b2) {
        if (b1 < b2) {
            body1 = b1;
            body2 = b2;
        } else {
            body1 = b2;
            body2 = b1;
        }

        numContacts = neko_phy_collide(contacts, body1, body2);

        // 摩擦模型
        // 毕竟刚体碰撞的碰撞力是无限大 对应的最大静摩擦力也就变成了无限大
        // 可以使用均方根
        friction = sqrtf(body1->friction * body2->friction);
    }

    // 更新一个value怎么会这么长
    // 以及
    // 为什么不直接每次清空
    void step(neko_phy_contact *newContacts, s32 numNewContacts) {
        neko_phy_contact mergedContacts[2];

        for (s32 i = 0; i < numNewContacts; ++i) {
            neko_phy_contact *cNew = newContacts + i;  // 取数组索引的 newContacts是数组首地址
            s32 k = -1;
            for (s32 j = 0; j < numContacts; ++j) {
                neko_phy_contact *cOld = contacts + j;  // 直接取索引
                if (cNew->feature.value == cOld->feature.value) {
                    k = j;
                    break;
                }
            }

            if (k > -1) {
                neko_phy_contact *c = mergedContacts + i;
                neko_phy_contact *cOld = contacts + k;
                *c = *cNew;
                if (do_hot_starting) {
                    c->Pn = cOld->Pn;
                    c->Pt = cOld->Pt;
                    c->Pnb = cOld->Pnb;
                } else {
                    c->Pn = 0.0f;
                    c->Pt = 0.0f;
                    c->Pnb = 0.0f;
                }
            } else {
                mergedContacts[i] = newContacts[i];
            }
        }

        for (s32 i = 0; i < numNewContacts; ++i) contacts[i] = mergedContacts[i];

        numContacts = numNewContacts;
    }

    void prestep(f32 inv_dt) {
        const f32 k_allowedPenetration = 0.01f;                                         // 允许渗透值 用来减小抖动
        f32 k_biasFactor = position_correction ? neko_position_correction_size : 0.0f;  // 增加额外的间隙

        for (s32 i = 0; i < numContacts; ++i) {
            neko_phy_contact *c = contacts + i;

            neko_vec2 r1 = c->position - body1->position;
            neko_vec2 r2 = c->position - body2->position;

            // 预先计算法向质量 切线质量和偏差
            f32 rn1 = neko_vec2_dot(r1, c->normal);  // r1在法线上的投影
            f32 rn2 = neko_vec2_dot(r2, c->normal);
            f32 kNormal = body1->invMass + body2->invMass;
            kNormal += body1->invI * (neko_vec2_dot(r1, r1) - rn1 * rn1) + body2->invI * (neko_vec2_dot(r2, r2) - rn2 * rn2);
            c->massNormal = 1.0f / kNormal;

            neko_vec2 tangent = neko_vec2_cross(c->normal, 1.0f);  // 获取法线的垂线(碰撞切线)
            f32 rt1 = neko_vec2_dot(r1, tangent);
            f32 rt2 = neko_vec2_dot(r2, tangent);
            f32 kTangent = body1->invMass + body2->invMass;
            kTangent += body1->invI * (neko_vec2_dot(r1, r1) - rt1 * rt1) + body2->invI * (neko_vec2_dot(r2, r2) - rt2 * rt2);
            c->massTangent = 1.0f / kTangent;

            c->bias = -k_biasFactor * inv_dt * neko_min(0.0f, c->separation + k_allowedPenetration);

            if (accumulate_impulses) {
                // 施加普通与摩擦冲量
                neko_vec2 P = c->Pn * c->normal + c->Pt * tangent;

                body1->velocity -= body1->invMass * P;
                body1->angularVelocity -= body1->invI * neko_vec2_cross(r1, P);

                body2->velocity += body2->invMass * P;
                body2->angularVelocity += body2->invI * neko_vec2_cross(r2, P);
            }
        }
    }

    void apply_impulse() {
        neko_phy_body *b1 = body1;
        neko_phy_body *b2 = body2;

        for (s32 i = 0; i < numContacts; ++i) {
            neko_phy_contact *c = contacts + i;
            c->r1 = c->position - b1->position;
            c->r2 = c->position - b2->position;

            // Relative velocity at contact
            neko_vec2 dv = b2->velocity + neko_vec2_cross(b2->angularVelocity, c->r2) - b1->velocity - neko_vec2_cross(b1->angularVelocity, c->r1);

            // Compute normal impulse
            f32 vn = neko_vec2_dot(dv, c->normal);

            f32 dPn = c->massNormal * (-vn + c->bias);

            if (accumulate_impulses) {
                // Clamp the accumulated impulse
                f32 Pn0 = c->Pn;
                c->Pn = neko_max(Pn0 + dPn, 0.0f);
                dPn = c->Pn - Pn0;
            } else {
                dPn = neko_max(dPn, 0.0f);
            }

            // Apply contact impulse
            neko_vec2 Pn = dPn * c->normal;

            b1->velocity -= b1->invMass * Pn;
            b1->angularVelocity -= b1->invI * neko_vec2_cross(c->r1, Pn);

            b2->velocity += b2->invMass * Pn;
            b2->angularVelocity += b2->invI * neko_vec2_cross(c->r2, Pn);

            // Relative velocity at contact
            dv = b2->velocity + neko_vec2_cross(b2->angularVelocity, c->r2) - b1->velocity - neko_vec2_cross(b1->angularVelocity, c->r1);

            neko_vec2 tangent = neko_vec2_cross(c->normal, 1.0f);
            f32 vt = neko_vec2_dot(dv, tangent);
            f32 dPt = c->massTangent * (-vt);

            if (accumulate_impulses) {
                // Compute friction impulse
                f32 maxPt = friction * c->Pn;

                // Clamp friction
                f32 oldTangentImpulse = c->Pt;
                c->Pt = neko_clamp(oldTangentImpulse + dPt, -maxPt, maxPt);
                dPt = c->Pt - oldTangentImpulse;
            } else {
                f32 maxPt = friction * dPn;
                dPt = neko_clamp(dPt, -maxPt, maxPt);
            }

            // Apply contact impulse
            neko_vec2 Pt = dPt * tangent;

            b1->velocity -= b1->invMass * Pt;
            b1->angularVelocity -= b1->invI * neko_vec2_cross(c->r1, Pt);

            b2->velocity += b2->invMass * Pt;
            b2->angularVelocity += b2->invI * neko_vec2_cross(c->r2, Pt);
        }
    }

    neko_phy_contact contacts[MAX_POINTS];  // 接触点
    s32 numContacts;

    neko_phy_body *body1;
    neko_phy_body *body2;

    // 组合摩擦力
    f32 friction;
};

// 用于 std::set
inline bool operator<(const neko_phy_arbiter_key &a1, const neko_phy_arbiter_key &a2) {
    if (a1.body1 < a2.body1) return true;

    if (a1.body1 == a2.body1 && a1.body2 < a2.body2) return true;

    return false;
}

struct neko_phy_world {
    neko_phy_world(neko_vec2 gravity, s32 iterations) : gravity(gravity), iterations(iterations) {}

    void neko_phy_create(neko_phy_body *body);
    void neko_phy_create(neko_phy_joint *joint);
    void neko_phy_clear();

    void neko_phy_step(f32 dt);

    void neko_phy_broadphase();

    std::vector<neko_phy_body *> bodies;
    std::vector<neko_phy_joint *> joints;
    std::map<neko_phy_arbiter_key, neko_phy_arbiter> arbiters;
    neko_vec2 gravity;
    s32 iterations;
};

#endif
