#pragma once
#include "Bonus.hpp"
#include "Constants.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

// отвечает только за падающие бонусы (хранение, обновление, отрисовку)
class BonusManager {
public:
    std::vector<std::unique_ptr<Bonus>> bonuses;

    // создает случайный бонус в позиции pos
    void spawn(sf::Vector2f pos) {
        bonuses.push_back(makeRandomBonus(pos));
    }

    // обновляет все бонусы; возвращает true если нужно заспавнить второй шар
    bool update(float dt, sf::RectangleShape& paddle, float& paddleW, float& ballSpeed, bool& ballSticky, bool& floorActive) {
        bool spawnSecond = false;
        sf::FloatRect paddleRect = paddle.getGlobalBounds();

        for (auto& bon : bonuses) {
            if (!bon->active) continue;

            bon->update(dt);

            if (bon->shape.getGlobalBounds().findIntersection(paddleRect)) {
                bon->apply(paddle, paddleW, ballSpeed, ballSticky, floorActive, spawnSecond);
                paddle.setSize({paddleW, PADDLE_H});
                bon->active = false;
            }

            if (bon->shape.getPosition().y > WIN_H + 20.f)
                bon->active = false;
        }

        bonuses.erase(
            std::remove_if(bonuses.begin(), bonuses.end(), [](const auto& b) { return !b->active; }),
            bonuses.end());
            
        return spawnSecond;
    }

    void draw(sf::RenderWindow& window) const {
        for (const auto& bon : bonuses)
            if (bon->active)
                window.draw(bon->shape);
    }

    void clear() { bonuses.clear(); }
};
