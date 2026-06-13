#include "Ball.hpp"
#include "Bonus.hpp"
#include "BrickGrid.hpp"
#include "Constants.hpp"
#include "Floor.hpp"
#include "HUD.hpp"
#include <SFML/Graphics.hpp>

//вспомогательные штучки
static void resetBallOnPaddle(Ball &ball, const sf::RectangleShape &paddle, float paddleW) {
  ball.launched = false;
  ball.sticky = false;
  ball.vel = {0.f, 0.f};
  ball.speed = BALL_SPEED;
  ball.shape.setFillColor(sf::Color::White);
  ball.shape.setPosition({paddle.getPosition().x + paddleW / 2.f, PADDLE_Y - BALL_R - 1.f});
}

// ─────────────────────────────────────────────────────────────────────────────
int main() {
  sf::RenderWindow window(sf::VideoMode({WIN_W, WIN_H}), "Arkanoid", sf::Style::Titlebar | sf::Style::Close);
  window.setFramerateLimit(60);

  // шрифт
  HUD hud;
  hud.loadFontAuto();

  // каретка
  float paddleW = PADDLE_W;
  sf::RectangleShape paddle({paddleW, PADDLE_H});
  paddle.setFillColor(sf::Color(100, 180, 255));
  paddle.setOutlineColor(sf::Color(50, 120, 220));
  paddle.setOutlineThickness(2.f);
  paddle.setPosition({(WIN_W - paddleW) / 2.f, PADDLE_Y});

  // мячи
  std::vector<Ball> balls;
  {
    Ball b;
    resetBallOnPaddle(b, paddle, paddleW);
    balls.push_back(b);
  }

  // блоки
  std::vector<Brick> bricks;
  initBricks(bricks);

  // падающие бонусы
  std::vector<std::unique_ptr<Bonus>> bonuses;

  // одноразовое дно 
  OneTimeFloor oneFloor;

  // состояние игры
  int score = 0;
  int losses = 0;
  bool ballSticky = false;

  enum class State { Playing, Win, GameOver };
  State state = State::Playing;

  // лямда полного рестарта
  auto doRestart = [&]() {
    score = 0;
    losses = 0;
    state = State::Playing;
    ballSticky = false;
    paddleW = PADDLE_W;
    paddle.setSize({paddleW, PADDLE_H});
    paddle.setPosition({(WIN_W - paddleW) / 2.f, PADDLE_Y});
    bonuses.clear();
    oneFloor.deactivate();
    initBricks(bricks);
    balls.clear();
    Ball b;
    resetBallOnPaddle(b, paddle, paddleW);
    balls.push_back(b);
  };

  sf::Clock clock;

  // – игровой цикл –
  while (window.isOpen()) {
    float dt = clock.restart().asSeconds();
    if (dt > 0.1f)
      dt = 0.1f;

    // обработка событий
    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>())
        window.close();

      if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
        // запуск мяча
        if (key->code == sf::Keyboard::Key::Space && state == State::Playing) {
          for (auto &ball : balls) {
            if (!ball.launched) {
              ball.launched = true;
              ball.sticky = false;
              ball.vel = {ball.speed * 0.5f, -ball.speed * 0.866f};
            }
          }
        }
        // рестарт
        if (key->code == sf::Keyboard::Key::R)
          doRestart();
      }
    }

    // обновление в Playing
    if (state == State::Playing) {

      // движение каретки через стрелочки или A/D
      bool moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
      bool moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);

      if (moveLeft)
        paddle.move({-PADDLE_SPEED * dt, 0.f});
      if (moveRight)
        paddle.move({PADDLE_SPEED * dt, 0.f});

      // ограничение каретки
      if (paddle.getPosition().x < 0.f)
        paddle.setPosition({0.f, PADDLE_Y});
      if (paddle.getPosition().x + paddleW > WIN_W)
        paddle.setPosition({WIN_W - paddleW, PADDLE_Y});

      // физика мячей
      std::vector<int> lostBallIndices;

      for (int bi = 0; bi < static_cast<int>(balls.size()); ++bi) {
        Ball &ball = balls[bi];

        // незапущенный мяч следует за кареткой
        if (!ball.launched) {
          ball.shape.setPosition({paddle.getPosition().x + paddleW / 2.f, PADDLE_Y - BALL_R - 1.f});
          continue;
        }

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
          if (len > 0.f)
            ball.vel = ball.vel / len * ball.speed;

          if (ballSticky) {
            ball.launched = false;
            ball.sticky = false;
            ballSticky = false;
          }
        }

        // столкновения шаров между собой для 9го пункта
        for (int bj = bi + 1; bj < static_cast<int>(balls.size()); ++bj) {
          Ball &other = balls[bj];
          if (!other.launched)
            continue;
          if (ball.aabb().findIntersection(other.aabb())) {
            std::swap(ball.vel, other.vel);
          }
        }

        // коллизии с блоками
        for (auto &brick : bricks) {
          if (!brick.active)
            continue;
          if (!resolveCollision(ball, brick.shape.getGlobalBounds()))
            continue;

          if (brick.type == BrickType::Indestructible) {
            // только отскок
          } else if (brick.type == BrickType::SpeedUp) {
            brick.hp--;
            score++;
            if (brick.hp <= 0)
              brick.active = false;
            float newSpeed =
                std::min(ball.speed + BALL_SPEED_BOOST, BALL_SPEED_MAX);
            ball.setSpeed(newSpeed);
            ball.updateColor();
          } else { // бонусный блок
            brick.hp--;
            score++;
            if (brick.hp <= 0) {
              brick.active = false;
              if (brick.hasBonus) {
                sf::Vector2f center = brick.shape.getPosition();
                center.x += BRICK_W / 2.f;
                center.y += BRICK_H / 2.f;
                bonuses.push_back(makeRandomBonus(center));
              }
            } else {
              brick.shape.setFillColor(colorByHp(brick.hp));
            }
          }
        }

        // нижняя граница
        if (ball.shape.getPosition().y - BALL_R > WIN_H) {
          if (oneFloor.active) {
            // одноразовое дно отражает мяч
            ball.vel.y = -std::abs(ball.vel.y);
            ball.shape.setPosition({ball.shape.getPosition().x, static_cast<float>(WIN_H) - BALL_R - 1.f});
            oneFloor.deactivate();
          } else {
            lostBallIndices.push_back(bi);
          }
        }
      }

      // обработка потерянных шаров
      // вторые шары удаляются, первый сбрасывается
      for (int i = static_cast<int>(lostBallIndices.size()) - 1; i >= 0; --i) {
        int idx = lostBallIndices[i];
        if (idx == 0) {
          losses++;
          score -= 3;
          if (score < 0)
            score = 0;
          resetBallOnPaddle(balls[0], paddle, paddleW);
          // второй шар тоже убирается при потере основного
          if (balls.size() > 1)
            balls.erase(balls.begin() + 1, balls.end());
        } else {
          balls.erase(balls.begin() + idx);
        }
      }

      // падающие бонусы
      sf::FloatRect paddleRect = paddle.getGlobalBounds();
      for (auto &bon : bonuses) {
        if (!bon->active)
          continue;
        bon->update(dt);

        if (bon->shape.getGlobalBounds().findIntersection(paddleRect)) {
          bool spawnSecond = false;
          bon->apply(paddle, paddleW, balls[0].speed, ballSticky,
                     oneFloor.active, spawnSecond);

          // обновляем размер каретки
          paddle.setSize({paddleW, PADDLE_H});

          // применяем изменение скорости к запущенным шарам
          for (auto &ball : balls)
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

          bon->active = false;
        }

        if (bon->shape.getPosition().y > WIN_H + 20.f)
          bon->active = false;
      }
      bonuses.erase(std::remove_if(bonuses.begin(), bonuses.end(), [](const auto &b) { return !b->active; }), bonuses.end());

      // победа/поражение
      if (allBricksCleared(bricks))
        state = State::Win;
      if (losses >= 5)
        state = State::GameOver;
    }

    // HUD
    hud.update(score, losses, balls[0].speed, ballSticky, oneFloor.active,
               static_cast<int>(balls.size()));

    // отрисовка
    window.clear(sf::Color(20, 20, 35));

    // блоки
    for (auto &b : bricks) {
      if (!b.active)
        continue;
      window.draw(b.shape);

      // метка типа
      if (b.type == BrickType::SpeedUp) {
        // пурпурный отличается цветом, дополнительный маркер не нужен
      }
      // маркер бонусного блока
      if (b.hasBonus && b.active) {
        sf::RectangleShape dot({5.f, 5.f});
        dot.setFillColor(sf::Color::Yellow);
        dot.setPosition({b.shape.getPosition().x + BRICK_W - 7.f, b.shape.getPosition().y + 2.f});
        window.draw(dot);
      }
    }

    // одноразовое дно
    if (oneFloor.active)
      window.draw(oneFloor.shape);

    // падающие бонусы
    for (auto &bon : bonuses)
      if (bon->active)
        window.draw(bon->shape);

    // каретка
    window.draw(paddle);

    // шары
    for (auto &ball : balls)
      window.draw(ball.shape);

    // HUD
    hud.draw(window);

    // оверлей конца игры
    if (state == State::Win)
      hud.drawEnd(window, "  Win!\nScore: " + std::to_string(score) + "\n\n   R — restart");
    if (state == State::GameOver)
      hud.drawEnd(window, "GAME OVER\nScore: " + std::to_string(score) + "\n\n R — restart");

    window.display();
  }
}
