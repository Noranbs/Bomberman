#ifndef BOMBERMAN_AP_SFML_SPRITE_VIEW_H
#define BOMBERMAN_AP_SFML_SPRITE_VIEW_H

#include "bomberman/logic/Entity.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <memory>

namespace bomberman::sfml {

class SpriteView final : public logic::Observer {
public:
    SpriteView(std::weak_ptr<logic::Entity> entity,
               std::shared_ptr<sf::RenderWindow> window,
               std::shared_ptr<sf::Texture> texture);

    void onNotify(const logic::Event& event) override;
    void draw();
    int renderLayer() const;

private:
    void syncTransform();
    void configureSprite();

    std::weak_ptr<logic::Entity> entity;
    std::shared_ptr<sf::RenderWindow> window;
    std::shared_ptr<sf::Texture> texture;
    sf::Sprite sprite;
    sf::RectangleShape fallback;
    sf::Vector2f spriteSourceSize{24.0F, 24.0F};
    sf::Vector2f spriteOrigin{12.0F, 12.0F};
    bool usesTexture{false};
};

}

#endif //BOMBERMAN_AP_SFML_SPRITE_VIEW_H
