#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Constants.hpp"

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f vel;
    bool launched = false;
    bool sticky = false; // прилип к каретке после отскока
    float speed = BALL_SPEED;

    Ball() {
        shape.setRadius(BALL_R);
        shape.setOrigin({BALL_R, BALL_R});
        shape.setFillColor(sf::Color::White);
    }

    sf::FloatRect aabb() const {
        float x = shape.getPosition().x;
        float y = shape.getPosition().y;
        return {{x - BALL_R, y - BALL_R}, {BALL_R * 2.f, BALL_R * 2.f}};
    }

    // установить скорость, сохраняя направление
    void setSpeed(float newSpeed) {
        float len = std::hypot(vel.x, vel.y);
        if (len > 0.f)
            vel = vel / len * newSpeed;
        speed = newSpeed;
    }

    // обновить цвет в зависимости от скорости
    void updateColor() {
        float t = (speed - BALL_SPEED) / (BALL_SPEED_MAX - BALL_SPEED);
        t = std::clamp(t, 0.f, 1.f);
        shape.setFillColor(sf::Color(
            255,
            static_cast<uint8_t>(255 * (1.f - t * 0.8f)),
            static_cast<uint8_t>(255 * (1.f - t))
        ));
    }
};

// разрешить коллизию мяча с прямоугольником; возвращает true, если было пересечение
inline bool resolveCollision(Ball& ball, const sf::FloatRect& rect) {
    sf::FloatRect bRect = ball.aabb();
    if (!bRect.findIntersection(rect))
        return false;

    float overlapLeft = (bRect.position.x + bRect.size.x) - rect.position.x;
    float overlapRight = (rect.position.x  + rect.size.x)  - bRect.position.x;
    float overlapTop = (bRect.position.y + bRect.size.y) - rect.position.y;
    float overlapBottom = (rect.position.y  + rect.size.y)  - bRect.position.y;

    bool fromLeft = std::abs(overlapLeft) < std::abs(overlapRight);
    bool fromTop = std::abs(overlapTop) < std::abs(overlapBottom);

    float minOverlapX = fromLeft ? overlapLeft : overlapRight;
    float minOverlapY = fromTop ? overlapTop : overlapBottom;

    if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
        ball.vel.x = -ball.vel.x;
        float push = fromLeft ? -overlapLeft : overlapRight;
        ball.shape.move({push, 0.f});
    } else {
        ball.vel.y = -ball.vel.y;
        float push = fromTop ? -overlapTop : overlapBottom;
        ball.shape.move({0.f, push});
    }
    return true;
}
