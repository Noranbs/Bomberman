#ifndef BOMBERMAN_AP_LOGIC_GEOMETRY_H
#define BOMBERMAN_AP_LOGIC_GEOMETRY_H

namespace bomberman::logic {

struct Vec2 {
    float x{0.0F};
    float y{0.0F};

    constexpr Vec2 operator+(const Vec2& other) const
    {
        return {x + other.x, y + other.y};
    }

    constexpr Vec2 operator*(float scalar) const
    {
        return {x * scalar, y * scalar};
    }
};

// Bounding box used for collision detection
struct Rect {
    Vec2 center{};
    Vec2 halfSize{};

    constexpr float left() const { return center.x - halfSize.x; }
    constexpr float right() const { return center.x + halfSize.x; }
    constexpr float top() const { return center.y - halfSize.y; }
    constexpr float bottom() const { return center.y + halfSize.y; }

    [[nodiscard]] constexpr bool intersects(const Rect& other) const
    {
        constexpr float epsilon = 0.0001F;
        return left() < other.right() - epsilon && right() > other.left() + epsilon &&
               top() < other.bottom() - epsilon && bottom() > other.top() + epsilon;
    }
};

}

#endif //BOMBERMAN_AP_LOGIC_GEOMETRY_H
