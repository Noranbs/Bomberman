#ifndef BOMBERMAN_AP_SFML_CAMERA_H
#define BOMBERMAN_AP_SFML_CAMERA_H

#include "bomberman/logic/Geometry.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace bomberman::sfml {

/**
 * @brief Converts normalized world coordinates to window pixels.
 */
class Camera final {
public:
    /**
     * @brief Creates a camera for the current window size.
     * @param windowSize Current window size in pixels.
     */
    explicit Camera(sf::Vector2u windowSize)
        : windowSize(windowSize)
    {
    }

    /**
     * @brief Converts a world position to a pixel position.
     * @param position Position in normalized world coordinates.
     * @return Position in pixels.
     */
    sf::Vector2f project(logic::Vec2 position) const
    {
        return {(position.x + 1.0F) * 0.5F * static_cast<float>(windowSize.x),
                (position.y + 1.0F) * 0.5F * static_cast<float>(windowSize.y)};
    }

    /**
     * @brief Converts a world size to a pixel size.
     * @param size Size in normalized world coordinates.
     * @return Size in pixels.
     */
    sf::Vector2f projectSize(logic::Vec2 size) const
    {
        return {size.x * 0.5F * static_cast<float>(windowSize.x),
                size.y * 0.5F * static_cast<float>(windowSize.y)};
    }

private:
    sf::Vector2u windowSize{}; ///< Window size in pixels.
};

}

#endif //BOMBERMAN_AP_SFML_CAMERA_H
