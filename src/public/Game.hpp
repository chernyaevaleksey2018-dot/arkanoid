#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include "Constants.hpp"
#include "Ball.hpp"
//#include "Brick.hpp"
#include "BrickGrid.hpp"
#include "Bonus.hpp"
//#include "BonusFactory.hpp"
#include "Floor.hpp"
#include "HUD.hpp"

enum class GameState { Playing, Win, GameOver };

class Game {
public:
    explicit Game(sf::RenderWindow& window, HUD& hud)
        : window(window), hud(hud)
    {
        // Каретка
        paddle.setSize({paddleW, PADDLE_H});
        paddle.setFillColor(sf::Color(100, 180, 255));
        paddle.setOutlineColor(sf::Color(50, 120, 220));
        paddle.setOutlineThickness(2.f);

        restart();
    }

    void restart() {
        score = 0;
        losses = 0;
        state = GameState::Playing;
        paddleW = PADDLE_W;
        paddle.setSize({paddleW, PADDLE_H});
        paddle.setPosition({(WIN_W - paddleW) / 2.f, PADDLE_Y});

        balls.clear();
        Ball b;
        resetBall(b);
        balls.push_back(b);

        bonuses.clear();
        floor.deactivate();
        initBricks(bricks);
    }

    void handleEvent(const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Space && state == GameState::Playing) {
                for (auto& ball : balls) {
                    if (!ball.launched) {
                        ball.launched = true;
                        ball.sticky   = false;
                        // угол около 60 градусов вверх
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

        for (auto& ball : balls) {
            if (!ball.launched) {
                // если мяч прилип, следует за кареткой
                ball.shape.setPosition({ paddle.getPosition().x + paddleW / 2.f, PADDLE_Y - BALL_R - 1.f });
            } else {
                moveBall(ball, dt);
            }
        }

        updateBonuses(dt);
        checkWinLose();
    }

    void draw() {
        // блоки
        for (auto& b : bricks) {
            if (!b.active) continue;
            window.draw(b.shape);
            // иконка бонуса на блоке
            if (b.hasBonus && b.type == BrickType::Normal) {
                // маленький маркер в углу блока
                sf::RectangleShape marker({6.f, 6.f});
                marker.setFillColor(sf::Color::Yellow);
                auto pos = b.shape.getPosition();
                marker.setPosition({pos.x + BRICK_W - 8.f, pos.y + 2.f});
                window.draw(marker);
            }
        }

        // одноразовое дно
        if (floor.active)
            window.draw(floor.shape);

        // падающие бонусы
        for (auto& bon : bonuses) {
            if (bon->active)
                window.draw(bon->shape);
        }

        // каретка
        window.draw(paddle);

        // мячи
        for (auto& ball : balls)
            window.draw(ball.shape);
    }

    GameState getState() const { return state; }
    int getScore() const { return score; }
    int getLosses() const { return losses; }
    float getBallSpeed() const { return balls.empty() ? BALL_SPEED : balls[0].speed; }
    bool isStickyActive() const {
        for (const auto& b : balls) if (b.sticky) return true;
        return false;
    }
    bool isFloorActive() const { return floor.active; }
    int ballCount() const { return static_cast<int>(balls.size()); }

private:
    sf::RenderWindow&window;
    HUD&hud;

    sf::RectangleShape paddle;
    float paddleW = PADDLE_W;

    std::vector<Ball> balls;
    std::vector<Brick> bricks;
    std::vector<std::unique_ptr<Bonus>> bonuses;
    OneTimeFloor floor;

    int score  = 0;
    int losses = 0;
    GameState state = GameState::Playing;

    // движение каретки стрелочками и A/D
    void movePaddle(float dt) {
        bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
        bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
        if (left) paddle.move({-PADDLE_SPEED * dt, 0.f});
        if (right) paddle.move({ PADDLE_SPEED * dt, 0.f});
    }

    void clampPaddle() {
        float px = paddle.getPosition().x;
        if (px < 0.f) paddle.setPosition({0.f, PADDLE_Y});
        if (px + paddleW > WIN_W) paddle.setPosition({WIN_W - paddleW, PADDLE_Y});
    }

    void resetBall(Ball& ball) {
        ball.launched = false;
        ball.sticky = false;
        ball.vel = {0.f, 0.f};
        ball.speed = BALL_SPEED;
        ball.shape.setFillColor(sf::Color::White);
        ball.shape.setPosition({
            paddle.getPosition().x + paddleW / 2.f,
            PADDLE_Y - BALL_R - 1.f
        });
    }

    // движение и физика мяча
    void moveBall(Ball& ball, float dt) {
        ball.shape.move(ball.vel * dt);

        // левая/правая стена
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

        // коллизия с кареткой
        if (resolveCollision(ball, paddle.getGlobalBounds())) {
            float hitX = ball.shape.getPosition().x - (paddle.getPosition().x + paddleW / 2.f);
            float norm = hitX / (paddleW / 2.f);
            float speed = std::hypot(ball.vel.x, ball.vel.y);
            ball.vel.x = norm * speed * 0.9f;
            ball.vel.y = -std::abs(ball.vel.y);
            float len = std::hypot(ball.vel.x, ball.vel.y);
            if (len > 0.f) ball.vel = ball.vel / len * ball.speed;

            // прилипание
            if (ball.sticky) {
                ball.launched = false;
                ball.sticky = false; // одноразовое
            }
        }

        // коллизии между шарами (только если их > 1)
        for (auto& other : balls) {
            if (&other == &ball) continue;
            if (!other.launched) continue;
            auto aabb1 = ball.aabb();
            auto aabb2 = other.aabb();
            if (aabb1.findIntersection(aabb2)) {
                // отскок: меняем направления по x
                ball.vel.x = -ball.vel.x;
                other.vel.x = -other.vel.x;
            }
        }

        // коллизии с блоками
        for (auto& brick : bricks) {
            if (!brick.active) continue;
            if (!resolveCollision(ball, brick.shape.getGlobalBounds())) continue;
            onBallHitBrick(ball, brick);
        }

        // нижняя граница
        if (ball.shape.getPosition().y - BALL_R > WIN_H) {
            // проверяем одноразовое дно
            if (floor.active) {
                ball.vel.y = -std::abs(ball.vel.y);
                ball.shape.setPosition({ball.shape.getPosition().x, WIN_H - BALL_R - 1.f});
                floor.deactivate();
            } else {
                // мяч потерян
                ball.launched = false;
                ball.speed = BALL_SPEED;
                ball.shape.setFillColor(sf::Color::White);
                resetBall(ball);
            }
        }
    }

    void onBallHitBrick(Ball& ball, Brick& brick) {
        switch (brick.type) {
        case BrickType::Indestructible:
            break; // только отскок

        case BrickType::SpeedUp:
            brick.hp--;
            score++;
            if (brick.hp <= 0) brick.active = false;
            {
                float newSpeed = std::min(ball.speed + BALL_SPEED_BOOST, BALL_SPEED_MAX);
                ball.setSpeed(newSpeed);
                ball.updateColor();
            }
            break;

        case BrickType::Normal:
        case BrickType::BonusBlock:
        default:
            brick.hp--;
            score++;
            if (brick.hp <= 0) {
                brick.active = false;
                if (brick.hasBonus) {
                    // выпадает бонус из центра блока
                    auto center = brick.shape.getPosition();
                    center.x += BRICK_W / 2.f;
                    center.y += BRICK_H / 2.f;
                    bonuses.push_back(makeRandomBonus(center));
                }
            } else {
                brick.shape.setFillColor(colorByHp(brick.hp));
            }
            break;
        }
    }

    // бонусы
    void updateBonuses(float dt) {
        sf::FloatRect paddleRect = paddle.getGlobalBounds();

        for (auto& bon : bonuses) {
            if (!bon->active) continue;
            bon->update(dt);

            if (bon->shape.getGlobalBounds().findIntersection(paddleRect)) {
                bool spawnSecond = false;
                bon->apply(paddle, paddleW, balls[0].speed, balls[0].sticky, floor.active, spawnSecond);

                if (spawnSecond && balls.size() == 1) {
                    // создаём второй шарик рядом с первым
                    Ball b2 = balls[0];
                    b2.vel.x = -b2.vel.x; // зеркальное направление
                    if (!b2.launched) {
                        b2.launched = true;
                        b2.vel = {-b2.speed * 0.5f, -b2.speed * 0.866f};
                    }
                    balls.push_back(b2);
                }

                // обновляем скорость первого шара
                if (balls[0].launched)
                    balls[0].setSpeed(balls[0].speed);

                bon->active = false;
            }

            // вышел за нижнюю границу — удаляем
            if (bon->shape.getPosition().y > WIN_H + 20.f)
                bon->active = false;
        }

        // чистим неактивные бонусы
        bonuses.erase(
            std::remove_if(bonuses.begin(), bonuses.end(), [](const auto& b) { return !b->active; }),
            bonuses.end());
    }

    // победа / поражение ───────────────────────────────────────────────────
    void checkWinLose() {
        // считаем потери (мячи вышедшие за нижнюю границу и сброшенные)
        // потеря уже учитывается в moveBall, здесь только проверяем количество
        // незапущенных + запущенных > 0
        int lostThisFrame = 0;
        for (auto& ball : balls) {
            if (ball.shape.getPosition().y - BALL_R > WIN_H && !floor.active) {
                lostThisFrame++;
            }
        }

        // удаляем потерянные шары; первый всегда остаётся
        if (balls.size() > 1) {
            balls.erase(
                std::remove_if(balls.begin() + 1, balls.end(), [](const Ball& b) { return b.shape.getPosition().y - BALL_R > WIN_H; }),
                balls.end());
        }

        // засчитываем потерю если основной мяч был сброшен
        if (!balls.empty() && !balls[0].launched) {
            // он уже сброшен в moveBall; добавляем штраф
            static bool wasCounted = false;
            if (!wasCounted) {
                // для простоты: потеря считается когда шар не запущен и не только что сброшен
                // потерю считаем через позицию ниже экрана до resetBall
            }
        }

        // победа
        if (allBricksCleared(bricks)) {
            state = GameState::Win;
            return;
        }

        // Gameover
        if (losses >= 5) {
            state = GameState::GameOver;
        }
    }

    // увеличить счётчик потерь (вызывается снаружи при необходимости)
public:
    void addLoss() {
        losses++;
        score -= 3;
    }
};
