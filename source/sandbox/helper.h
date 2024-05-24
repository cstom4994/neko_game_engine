
#ifndef GAME_HELPER_H
#define GAME_HELPER_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "game_cvar.h"

class Vector2 {
public:
    int x_;
    int y_;

    Vector2();
    Vector2(int x, int y);
    Vector2(const Vector2& otherVec);

    ~Vector2();

    // Vector addition
    Vector2 operator+(const Vector2& otherVec);

    Vector2& operator+=(const Vector2& otherVec);

    Vector2 operator+(int s);

    Vector2& operator+=(int s);

    // Vector subtraction
    Vector2 operator-(const Vector2& otherVec);

    Vector2& operator-=(const Vector2& otherVec);

    Vector2 operator-(int s);

    Vector2& operator-=(int s);

    // Vector multiplication
    Vector2 operator*(int s);

    Vector2& operator*=(int s);

    // Vector division
    Vector2 operator/(int s);

    Vector2& operator/=(int s);

    // Useful Vector Operations
    int Length() const;

    Vector2& Normalize();

    int Distance(Vector2 otherVec) const;

    static int Dot(Vector2 v1, Vector2 v2);

    static int Cross(Vector2 v1, Vector2 v2);

    static Vector2 Lerp(Vector2 a, Vector2 b, float t);

    static Vector2 Zero();

private:
    friend std::ostream& operator<<(std::ostream& os, const Vector2& v);
};

std::ostream& operator<<(std::ostream& os, const Vector2& v);

// Vector comperison operators
bool operator<(const Vector2& lhs, const Vector2& rhs);

bool operator==(const Vector2& lhs, const Vector2& rhs);

bool operator!=(const Vector2& lhs, const Vector2& rhs);

bool operator<=(const Vector2& lhs, const Vector2& rhs);

bool operator>(const Vector2& lhs, const Vector2& rhs);

bool operator>=(const Vector2& lhs, const Vector2& rhs);

class Helper {
public:
    static neko_vec4_t Init(int XPos, int YPos, int Width, int Height) {
        neko_vec4_t Rect = {};
        Rect.z = Height;
        Rect.w = Width;
        Rect.x = XPos;
        Rect.y = YPos;

        return Rect;
    }

    static int GetIndex(int x, int y) { return x + (y << g_sim_chunk_width_Power2Expoent); }

    static Vector2 GetCords(int idx) { return Vector2((double)(idx & (g_sim_chunk_width - 1)), (double)(idx >> g_sim_chunk_width_Power2Expoent)); }

    static int GetChunk(int idx) { return GetChunk(GetCords(idx)); }

    static int GetChunk(Vector2 cord) {
        int x = cord.x_, y = cord.y_;
        return (y >> g_chunk_p2) * (g_sim_chunk_width >> g_chunk_p2) + (x >> g_chunk_p2);
    }

    static int GetChunk(int x, int y) { return (y >> g_chunk_p2) * (g_sim_chunk_width >> g_chunk_p2) + (x >> g_chunk_p2); }

    //    static int ScreenWidthPoint(int x) { return x * g_sim_chunk_width / 16; }
    //    static int ScreenHeightPoint(int y) { return y * g_sim_chunk_height / 9; }

    static double RandomDoubleOnInterval(double min, double max) { return std::uniform_real_distribution<double>(min, max)(rng); }

    static int RandomIntOnInterval(int min, int max) { return std::uniform_int_distribution<int>(min, max)(rng); }

    static bool CoinToss() { return std::uniform_int_distribution<int>(0, 1)(rng); }
    // http://oeis.org/A295344
    static std::vector<Vector2> LatticePointsOnCircle(int radius, Vector2 center = Vector2(0, 0)) {
        int sqrRadius = radius * radius;
        std::vector<Vector2> points = std::vector<Vector2>();
        for (int i = -radius; i <= radius; i++) {
            for (int j = -radius; j <= radius; j++) {
                if (i * i + j * j <= sqrRadius) {
                    points.push_back(center + Vector2(i, j));
                }
            }
        }
        return points;
    }
    // http://eugen.dedu.free.fr/projects/bresenham/
    static std::vector<Vector2> LineCover(Vector2 a, Vector2 b, bool super = false) {
        bool swap = false;
        int x = a.x_, y = a.y_;
        int dx = b.x_ - a.x_, dy = b.y_ - a.y_;
        std::vector<Vector2> points = std::vector<Vector2>();
        auto cmp = [](int a, int b) { return (a < b) ? -1 : 1; };
        if (cmp(abs(dx), abs(dy)) < 0) {
            swap = true;
            std::swap(x, y);
            std::swap(dx, dy);
        }
        int x_step = cmp(0, dx), y_step = cmp(0, dy);
        dx = abs(dx);
        dy = abs(dy);
        int ddx = 2 * dx, ddy = 2 * dy;
        int errorprev = dy, error = dy;
        Vector2 foo;
        for (int i = 0; i < dy; i++) {
            y += y_step;
            error += ddx;
            if (error > ddy) {
                x += x_step;
                error -= ddy;
                if (super) {
                    if (error + errorprev <= ddy) {
                        foo = Vector2(x - x_step, y);
                        points.push_back(a - foo);
                    }
                    if (error + errorprev >= ddy) {
                        foo = Vector2(x, y - y_step);
                        points.push_back(a - foo);
                    }
                }
            }
            foo = Vector2(x, y);
            points.push_back(a - foo);
        }
        if (swap) {
            for (int i = 0; i < points.size(); i++) {
                std::swap(points[i].x_, points[i].y_);
            }
        }
        return points;
    }

    static Vector2 LastPoint(Vector2 a, Vector2 b, bool super = false) {
        bool swap = false;
        int x = a.x_, y = a.y_;
        int dx = b.x_ - a.x_, dy = b.y_ - a.y_;
        Vector2 points = a;
        auto cmp = [](int a, int b) { return (a < b) ? -1 : 1; };
        if (cmp(abs(dx), abs(dy)) < 0) {
            swap = true;
            std::swap(x, y);
            std::swap(dx, dy);
        }
        int x_step = cmp(0, dx), y_step = cmp(0, dy);
        dx = abs(dx);
        dy = abs(dy);
        int ddx = 2 * dx, ddy = 2 * dy;
        int errorprev = dy, error = dy;
        Vector2 foo;
        for (int i = 0; i < dy; i++) {
            y += y_step;
            error += ddx;
            if (error > ddy) {
                x += x_step;
                error -= ddy;
                if (super) {
                    if (error + errorprev <= ddy) {
                        foo = Vector2(x - x_step, y);
                    }
                    if (error + errorprev >= ddy) {
                        foo = Vector2(x, y - y_step);
                    }
                }
            }
            foo = Vector2(x, y);
            points = a - foo;
        }
        if (swap) {
            std::swap(points.x_, points.y_);
        }
        return points;
    }
};

class Random {
public:
    static double DoubleOnInterval(double min, double max) { return std::uniform_real_distribution<double>(min, max)(rng); }

    static int IntOnInterval(int min, int max) { return std::uniform_int_distribution<int>(min, max)(rng); }

    static bool CoinToss() { return std::uniform_int_distribution<int>(0, 1)(rng); }
};

class Navigation {
public:
    static int dx[8];
    static int dy[8];
};

neko_inline neko_color_t neko_color_interpolate(neko_color_t x, neko_color_t y, float t) {
    u8 r = (u8)(x.r + (y.r - x.r) * t);
    u8 g = (u8)(x.g + (y.g - x.g) * t);
    u8 b = (u8)(x.b + (y.b - x.b) * t);
    u8 a = (u8)(x.a + (y.a - x.a) * t);
    return neko_color(r, g, b, a);
}

int g_random(void);
s32 random_val(s32 lower, s32 upper);
s32 random_tlb_val(s32 lower, s32 upper);

namespace neko {

template <typename T>
class creator {
public:
    virtual std::unique_ptr<T> create() = 0;
};

template <typename Type, typename ConcreteType, typename... Args>
class ConcreteCreator : public creator<Type> {
public:
    explicit ConcreteCreator(Args&&... args) : mArgs(std::forward<Args>(args)...) {}
    std::unique_ptr<Type> create() override { return createImpl(std::make_index_sequence<sizeof...(Args)>{}); }

private:
    template <size_t... Is>
    std::unique_ptr<Type> createImpl(std::index_sequence<Is...>) {
        return std::make_unique<ConcreteType>(std::get<Is>(mArgs)...);
    }
    std::tuple<Args...> mArgs;
};

template <typename T, typename I>
class Factory {
public:
    template <typename ConcreteType, typename... Args>
    void registerType(I identifier, Args&&... args) {
        static_assert(std::is_base_of<T, ConcreteType>::value, "Type must be a base of ConcreteType");
        creator_list[identifier] = std::make_unique<ConcreteCreator<T, ConcreteType, Args...>>(std::forward<Args>(args)...);
    }

    std::unique_ptr<T> create(I identifier) {
        if (creator_list.find(identifier) != creator_list.end()) {
            return creator_list[identifier]->create();
        }
        return nullptr;
    }

private:
    std::unordered_map<I, std::unique_ptr<creator<T>>> creator_list;
};

}  // namespace neko

#endif