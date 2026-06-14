#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include "Constants.hpp"
#include "Ball.hpp"
#include "BrickGrid.hpp"
#include "BonusManager.hpp" // менеджер бонусов
#include "ScoreKeeper.hpp" // для счёта/потерь
#include "Floor.hpp"
#include "HUD.hpp"

enum class GameState { Playing, Win, GameOver };

class Game {
public:
    explicit Game(sf::RenderWindow& window, HUD& hud): window(window), hud(hud)
    {
        paddle.setFillColor(sf::Color(100, 180, 255));
        paddle.setOutlineColor(sf::Color(50, 120, 220));
        paddle.setOutlineThickness(2.f);
        restart();
    }

    void restart() {
        sk.reset();
        state = GameState::Playing;
        paddleW = PADDLE_W;
        paddle.setSize({paddleW, PADDLE_H});
        paddle.setPosition({(WIN_W - paddleW) / 2.f, PADDLE_Y});

        balls.clear();
        Ball b;
        resetBall(b);
        balls.push_back(b);

        bonusMgr.clear();
        oneFloor.deactivate();
        initBricks(bricks);
    }

    void handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space && state == GameState::Playing) {
                for (auto& ball : balls) {
                    if (!ball.launched) {
                        ball.launched = true;
                        ball.sticky = false;
                        ball.vel = {ball.speed * 0.5f, -ball.speed * 0.866f};
                    }
                }
            }
            if (key->code == sf::Keyboard::Key::R) {
                restart();
            }
        }
    }

    void update(float dt) {
        if (state != GameState::Playing) return;

        movePaddle(dt);
        clampPaddle();

        // мячи, у которых !launched, следуют за кареткой
        for (auto& ball : balls) {
            if (!ball.launched) {
                ball.shape.setPosition({ paddle.getPosition().x + paddleW / 2.f, PADDLE_Y - BALL_R - 1.f });
            } else {
                moveBall(ball, dt);
            }
        }

        // обновляем бонусы; если вернул true, то спавним второй шар
        bool spawnSecond = bonusMgr.update(
            dt, paddle, paddleW,
            balls[0].speed, balls[0].sticky, oneFloor.active);

        // синхронизируем скорость всех шаров после изменения бонусом (возможного)
        for (auto& ball : balls)
            if (ball.launched)
                ball.setSpeed(balls[0].speed);

        if (spawnSecond && balls.size() < 2) {
            Ball b2 = balls[0];
            if (b2.launched) {
                b2.vel.x = -b2.vel.x;
            } else {
                b2.launched = true;
                b2.vel = {-b2.speed * 0.5f, -b2.speed * 0.866f};
            }
            balls.push_back(b2);
        }
        
        checkWinLose();
    }

    void draw() {
        for (auto& b : bricks) {
            if (!b.active) continue;
            window.draw(b.shape);
            if (b.hasBonus && b.type == BrickType::Normal) {
                sf::RectangleShape marker({6.f, 6.f});
                marker.setFillColor(sf::Color::Yellow);
                auto pos = b.shape.getPosition();
                marker.setPosition({pos.x + BRICK_W - 8.f, pos.y + 2.f});
                window.draw(marker);
            }
        }

        if (oneFloor.active)
            window.draw(oneFloor.shape);

        bonusMgr.draw(window);
        window.draw(paddle);

        for (auto& ball : balls)
            window.draw(ball.shape);
    }

    // для HUD
    GameState getState() const { return state; }
    int getScore() const { return sk.score; }
    int getLosses() const { return sk.losses; }
    float getBallSpeed() const { return balls.empty() ? BALL_SPEED : balls[0].speed; }
    bool isStickyActive() const {
        for (const auto& b : balls) if (b.sticky) return true;
        return false;
    }
    bool isFloorActive() const { return oneFloor.active; }
    int ballCount() const { return static_cast<int>(balls.size()); }

private:
    sf::RenderWindow& window;
    HUD&hud;

    sf::RectangleShape paddle;
    float paddleW = PADDLE_W;

    std::vector<Ball> balls;
    std::vector<Brick> bricks;
    BonusManager bonusMgr;   // вместо вектора unique_ptr и updateBonuses()
    ScoreKeeper sk;   // вместо int score/losses и addLoss()
    OneTimeFloor oneFloor;

    GameState state = GameState::Playing;

    // каретка 
    void movePaddle(float dt) {
        bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
        bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
        if (left) paddle.move({-PADDLE_SPEED * dt, 0.f});
        if (right) paddle.move({ PADDLE_SPEED * dt, 0.f});
    }

    void clampPaddle() {
        float px = paddle.getPosition().x;
        if (px < 0.f) paddle.setPosition({0.f, PADDLE_Y});
        if (px + paddleW > WIN_W)  paddle.setPosition({WIN_W - paddleW,  PADDLE_Y});
    }

    void resetBall(Ball& ball) {
        ball.launched = false;
        ball.sticky = false;
        ball.vel = {0.f, 0.f};
        ball.speed = BALL_SPEED;
        ball.shape.setFillColor(sf::Color::White);
        ball.shape.setPosition({ paddle.getPosition().x + paddleW / 2.f, PADDLE_Y - BALL_R - 1.f });
    }

    // физика мяча
    void moveBall(Ball& ball, float dt) {
        ball.shape.move(ball.vel * dt);

        // стены
        if (ball.shape.getPosition().x - BALL_R < 0.f) {
            ball.shape.setPosition({BALL_R, ball.shape.getPosition().y});
            ball.vel.x = std::abs(ball.vel.x);
        }
        if (ball.shape.getPosition().x + BALL_R > WIN_W) {
            ball.shape.setPosition({WIN_W - BALL_R, ball.shape.getPosition().y});
            ball.vel.x = -std::abs(ball.vel.x);
        }
        // потолок
        if (ball.shape.getPosition().y - BALL_R < 0.f) {
            ball.shape.setPosition({ball.shape.getPosition().x, BALL_R});
            ball.vel.y = std::abs(ball.vel.y);
        }

        // каретка
        if (resolveCollision(ball, paddle.getGlobalBounds())) {
            float hitX = ball.shape.getPosition().x - (paddle.getPosition().x + paddleW / 2.f);
            float norm = hitX / (paddleW / 2.f);
            float speed = std::hypot(ball.vel.x, ball.vel.y);
            ball.vel.x = norm * speed * 0.9f;
            ball.vel.y = -std::abs(ball.vel.y);
            float len = std::hypot(ball.vel.x, ball.vel.y);
            if (len > 0.f) ball.vel = ball.vel / len * ball.speed;

            if (ball.sticky) {
                ball.launched = false;
                ball.sticky   = false;
            }
        }

        // столкновения шаров между собой
        for (auto& other : balls) {
            if (&other == &ball || !other.launched) continue;
            if (ball.aabb().findIntersection(other.aabb())) {
                ball.vel.x  = -ball.vel.x;
                other.vel.x = -other.vel.x;
            }
        }

        // блоки
        for (auto& brick : bricks) {
            if (!brick.active) continue;
            if (!resolveCollision(ball, brick.shape.getGlobalBounds())) continue;
            onBallHitBrick(ball, brick);
        }

        // нижняя граница (учёт потери через ScoreKeeper)
        if (ball.shape.getPosition().y - BALL_R > WIN_H) {
            if (oneFloor.active) {
                ball.vel.y = -std::abs(ball.vel.y);
                ball.shape.setPosition({ ball.shape.getPosition().x, static_cast<float>(WIN_H) - BALL_R - 1.f });
                oneFloor.deactivate();
            } else {
                // потеря фиксируется тут один раз
                sk.addLoss();
                resetBall(ball);

                // при потере основного шара убираются все дополнительные
                if (&ball == &balls[0] && balls.size() > 1)
                    balls.erase(balls.begin() + 1, balls.end());
            }
        }
    }

    // попадание мяча в блок
    void onBallHitBrick(Ball& ball, Brick& brick) {
        switch (brick.type) {
        case BrickType::Indestructible:
            break;

        case BrickType::SpeedUp:
            brick.hp--;
            sk.addScore();
            if (brick.hp <= 0) brick.active = false;
            {
                float newSpeed = std::min(ball.speed + BALL_SPEED_BOOST, BALL_SPEED_MAX);
                ball.setSpeed(newSpeed);
                ball.updateColor();
            }
            break;

        default:  // Normal или BonusBlock
            brick.hp--;
            sk.addScore();
            if (brick.hp <= 0) {
                brick.active = false;
                if (brick.hasBonus) {
                    sf::Vector2f center = brick.shape.getPosition();
                    center.x += BRICK_W / 2.f;
                    center.y += BRICK_H / 2.f;
                    bonusMgr.spawn(center);   // теперь через менеджер
                }
            } else {
                brick.shape.setFillColor(colorByHp(brick.hp));
            }
            break;
        }
    }

    // победа / поражение
    // метод теперь лишь проверяет условия; учёт потерь в moveBall()
    void checkWinLose() {
        // удаляем потерянные дополнительные шары
        if (balls.size() > 1) {
            balls.erase(
                std::remove_if(balls.begin() + 1, balls.end(), [](const Ball& b) { return b.shape.getPosition().y - BALL_R > WIN_H; }),
                balls.end());
        }

        if (allBricksCleared(bricks)) {
            state = GameState::Win;
            return;
        }

        if (sk.isGameOver()) {
            state = GameState::GameOver;
        }
    }
};
