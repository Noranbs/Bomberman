#ifndef BOMBERMAN_AP_SFML_SPRITE_VIEW_H
#define BOMBERMAN_AP_SFML_SPRITE_VIEW_H

#include "bomberman/logic/Entity.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <chrono>
#include <memory>

namespace bomberman::sfml {

/**
 * @brief SFML view for one logic entity.
 *
 * The view observes the entity and draws the correct sprite or fallback shape.
 */
class SpriteView final : public logic::Observer {
public:
    /**
     * @brief Creates a view for one entity.
     * @param entity Logic entity to observe and draw.
     * @param window Window where the sprite is drawn.
     * @param texture Main sprite sheet.
     * @param tileTexture Sprite sheet for tiles.
     */
    SpriteView(std::weak_ptr<logic::Entity> entity,
               std::shared_ptr<sf::RenderWindow> window,
               std::shared_ptr<sf::Texture> texture,
               std::shared_ptr<sf::Texture> tileTexture);

    /**
     * @brief Reacts to movement and death events from the entity.
     * @param event Event sent by the observed entity.
     */
    void onNotify(const logic::Event& event) override;

    /**
     * @brief Draws the entity if it should be visible.
     */
    void draw();

    /**
     * @brief Returns the drawing layer used for sorting.
     */
    int renderLayer() const;

private:
    void syncTransform();
    void configureSprite();
    void updateAnimation();
    float animationSeconds() const;
    bool recentlyMoved() const;

    std::weak_ptr<logic::Entity> entity; ///< Entity drawn by this view.
    std::shared_ptr<sf::RenderWindow> window; ///< Window used for drawing.
    std::shared_ptr<sf::Texture> texture; ///< Main sprite sheet.
    std::shared_ptr<sf::Texture> tileTexture; ///< Tile sprite sheet.
    sf::Sprite sprite; ///< Sprite used when a texture is available.
    sf::RectangleShape fallback; ///< Simple shape used as backup drawing.
    sf::Vector2f spriteSourceSize{24.0F, 24.0F}; ///< Size of the selected sprite frame.
    sf::Vector2f spriteOrigin{12.0F, 12.0F}; ///< Origin point of the sprite frame.
    logic::Vec2 lastPosition{}; ///< Last known entity position.
    std::chrono::steady_clock::time_point animationStart{std::chrono::steady_clock::now()}; ///< Animation start time.
    std::chrono::steady_clock::time_point lastMoveTime{}; ///< Last time the entity moved.
    std::chrono::steady_clock::time_point deathStart{}; ///< Start time of the death animation.
    float visualScale{1.0F}; ///< Extra scale used for animations.
    bool usesTexture{false}; ///< True when drawing a sprite instead of fallback.
    bool hasLastPosition{false}; ///< True after lastPosition is initialized.
    bool deathAnimationActive{false}; ///< True while death animation is playing.
    bool mirrored{false}; ///< True when the sprite is flipped horizontally.
};

}

#endif //BOMBERMAN_AP_SFML_SPRITE_VIEW_H
