
#include "helper.h"

int Navigation::dx[8] = {0, 0, 1, -1, 1, -1, 1, -1};
int Navigation::dy[8] = {1, -1, 0, 0, 1, 1, -1, -1};

Vector2::Vector2() = default;
Vector2::Vector2(int x, int y) : x_(x), y_(y) {}
Vector2::Vector2(const Vector2& otherVec) : x_(otherVec.x_), y_(otherVec.y_) {}

Vector2::~Vector2() = default;

// Vector addition
Vector2 Vector2::operator+(const Vector2& otherVec) {
    Vector2 temp;
    temp.x_ = x_ + otherVec.x_;
    temp.y_ = y_ + otherVec.y_;
    return temp;
}

Vector2& Vector2::operator+=(const Vector2& otherVec) {
    x_ += otherVec.x_;
    y_ += otherVec.y_;
    return *this;
}

Vector2 Vector2::operator+(int s) {
    Vector2 temp;
    temp.x_ = x_ + s;
    temp.y_ = y_ + s;
    return temp;
}

Vector2& Vector2::operator+=(int s) {
    x_ += s;
    y_ += s;
    return *this;
}

// Vector subtraction
Vector2 Vector2::operator-(const Vector2& otherVec) {
    Vector2 temp;
    temp.x_ = x_ - otherVec.x_;
    temp.y_ = y_ - otherVec.y_;
    return temp;
}

Vector2& Vector2::operator-=(const Vector2& otherVec) {
    x_ -= otherVec.x_;
    y_ -= otherVec.y_;
    return *this;
}

Vector2 Vector2::operator-(int s) {
    Vector2 temp;
    temp.x_ = x_ - s;
    temp.y_ = y_ - s;
    return temp;
}

Vector2& Vector2::operator-=(int s) {
    x_ -= s;
    y_ -= s;
    return *this;
}

// Vector multiplication
Vector2 Vector2::operator*(int s) {
    Vector2 temp;
    temp.x_ = x_ * s;
    temp.y_ = y_ * s;
    return temp;
}

Vector2& Vector2::operator*=(int s) {
    x_ *= s;
    y_ *= s;
    return *this;
}

// Vector division
Vector2 Vector2::operator/(int s) {
    Vector2 temp;
    temp.x_ = x_ / s;
    temp.y_ = y_ / s;
    return temp;
}

Vector2& Vector2::operator/=(int s) {
    x_ /= s;
    y_ /= s;
    return *this;
}

// Useful Vector Operations
int Vector2::Length() const { return std::sqrt(x_ * x_ + y_ * y_); }

Vector2& Vector2::Normalize() {
    int magnitude = Length();
    if (magnitude == 0) return *this;
    return (*this) /= magnitude;
}

int Vector2::Distance(Vector2 otherVec) const {
    Vector2 d(otherVec.x_ - x_, otherVec.y_ - y_);
    return d.Length();
}

int Vector2::Dot(Vector2 v1, Vector2 v2) { return (v1.x_ * v2.x_) + (v1.y_ * v2.y_); }

int Vector2::Cross(Vector2 v1, Vector2 v2) { return (v1.x_ * v2.y_) - (v1.y_ * v2.x_); }

Vector2 Vector2::Lerp(Vector2 a, Vector2 b, float t) { return (a * (1 - t) + b * t); }

Vector2 Vector2::Zero() { return Vector2(0, 0); }

std::ostream& operator<<(std::ostream& os, const Vector2& v) { return os << "(" << v.x_ << ", " << v.y_ << ")"; }

bool operator<(const Vector2& lhs, const Vector2& rhs) { return ((lhs.x_ < rhs.x_) || (lhs.x_ == rhs.x_ && lhs.y_ < rhs.y_)); }

bool operator>(const Vector2& lhs, const Vector2& rhs) { return operator<(rhs, lhs); }

bool operator==(const Vector2& lhs, const Vector2& rhs) { return (lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_); }

bool operator!=(const Vector2& lhs, const Vector2& rhs) { return !operator==(lhs, rhs); }

bool operator<=(const Vector2& lhs, const Vector2& rhs) { return !operator>(lhs, rhs); }

bool operator>=(const Vector2& lhs, const Vector2& rhs) { return !operator<(lhs, rhs); }

// clang-format off
unsigned char random_table[256] = {
    246, 44, 11, 243, 99, 68, 235, 255, 40, 141, 188, 125, 228, 115, 33,
    248, 60, 235, 232, 58, 81, 140, 8, 68, 88, 143, 44, 100, 149, 214, 39,
    199, 52, 250, 217, 55, 231, 108, 68, 241, 50, 200, 121, 22, 51, 189,
    203, 193, 105, 72, 42, 235, 242, 142, 12, 139, 134, 87, 241, 239, 128,
    221, 225, 251, 248, 255, 123, 32, 148, 182, 193, 204, 79, 154, 182,
    26, 152, 54, 16, 162, 78, 220, 90, 65, 66, 242, 25, 79, 187, 74, 217,
    236, 56, 6, 125, 208, 132, 161, 193, 111, 52, 49, 218, 197, 111, 250,
    192, 230, 229, 204, 168, 230, 78, 58, 165, 131, 54, 94, 118, 29, 62,
    112, 180, 146, 71, 148, 5, 54, 158, 158, 91, 17, 95, 107, 38, 91, 198,
    47, 70, 228, 15, 175, 222, 225, 83, 80, 20, 253, 77, 163, 134, 158, 6,
    248, 111, 212, 81, 98, 168, 131, 158, 208, 30, 255, 208, 120, 175, 149,
    7, 124, 49, 142, 71, 211, 162, 26, 154, 194, 51, 128, 217, 162, 31,
    191, 158, 82, 223, 115, 108, 209, 62, 168, 51, 235, 212, 199, 151,
    11, 182, 245, 5, 10, 188, 231, 122, 166, 149, 54, 251, 153, 143, 107,
    98, 154, 90, 171, 78, 24, 41, 64, 187, 237, 58, 208, 106, 226, 176,
    228, 167, 17, 4, 148, 219, 18, 124, 70, 214, 105, 231, 206, 195, 127,
    182, 11, 208, 224, 56, 197, 11, 62, 138, 218, 18, 234, 245, 242
};
// clang-format on

neko_global int rnd_index = 0;

int g_random(void) {
    rnd_index = (rnd_index + 1) & 0xff;
    return random_table[rnd_index];
}

s32 random_val(s32 lower, s32 upper) {
    if (upper < lower) {
        s32 tmp = lower;
        lower = upper;
        upper = tmp;
    }
    return (rand() % (upper - lower + 1) + lower);
}

s32 random_tlb_val(s32 lower, s32 upper) {
    s32 rand_num = g_random();
    if (upper < lower) {
        s32 tmp = lower;
        lower = upper;
        upper = tmp;
    }
    return (rand_num % (upper - lower + 1) + lower);
}