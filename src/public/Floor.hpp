#pragma once
#include <SFML/Graphics.hpp>
#include "Constants.hpp"

// одноразовое дно (пункт 6): отражает мяч один раз, затем исчезает
struct OneTimeFloor {
    sf::RectangleShape shape;
    bool active = false;

    OneTimeFloor() {
        shape.setSize({static_cast<float>(WIN_W), 6.f});
        shape.setPosition({0.f, static_cast<float>(WIN_H) - 4.f});
        shape.setFillColor(sf::Color(255, 220, 50, 180));
    }

    void activate() { active = true; }
    void deactivate() { active = false; }
};
