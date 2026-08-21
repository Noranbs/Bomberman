#ifndef BOMBERMAN_AP_LOGIC_GEOMETRY_H
#define BOMBERMAN_AP_LOGIC_GEOMETRY_H

namespace bomberman::logic {

/**
 * @brief Simple 2D vector used by the logic layer.
 */
struct Vec2 {
    /// Horizontal coordinate.
    float x{0.0F};
    /// Vertical coordinate.
    float y{0.0F};

    /**
     * @brief Adds two vectors.
     * @param other Vector to add.
     * @return The added vector.
     */
    constexpr Vec2 operator+(const Vec2& other) const
    {
        return {x + other.x, y + other.y};
    }

    /**
     * @brief Multiplies this vector by a number.
     * @param scalar Number to multiply with.
     * @return The scaled vector.
     */
    constexpr Vec2 operator*(float scalar) const
    {
        return {x * scalar, y * scalar};
    }
};

/**
 * @brief Bounding box used for collision detection.
 */
struct Rect {
    /// Center point of the rectangle.
    Vec2 center{};
    /// Half width and half height.
    Vec2 halfSize{};

    /// Left side of the rectangle.
    constexpr float left() const { return center.x - halfSize.x; }
    /// Right side of the rectangle.
    constexpr float right() const { return center.x + halfSize.x; }
    /// Top side of the rectangle.
    constexpr float top() const { return center.y - halfSize.y; }
    /// Bottom side of the rectangle.
    constexpr float bottom() const { return center.y + halfSize.y; }

    /**
     * @brief Checks if this rectangle overlaps another rectangle.
     * @param other Rectangle to test against.
     * @return True if both rectangles overlap.
     */
    [[nodiscard]] constexpr bool intersects(const Rect& other) const
    {
        constexpr float epsilon = 0.0001F;
        return left() < other.right() - epsilon && right() > other.left() + epsilon &&
               top() < other.bottom() - epsilon && bottom() > other.top() + epsilon;
    }
};

}

#endif //BOMBERMAN_AP_LOGIC_GEOMETRY_H
