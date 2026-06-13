#pragma once
#include "Constants.hpp"
#include <SFML/Graphics.hpp>

// класс для падающих бонусов после уничтожения блоков
class Bonus {
public:
  sf::RectangleShape shape;
  bool active = true;

  explicit Bonus(sf::Vector2f pos, sf::Color color) {
    shape.setSize({BONUS_W, BONUS_H});
    shape.setOrigin({BONUS_W / 2.f, BONUS_H / 2.f});
    shape.setPosition(pos);
    shape.setFillColor(color);
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(1.f);
  }

  virtual ~Bonus() = default;
  void update(float dt) { shape.move({0.f, BONUS_SPEED * dt}); }

  // применить эффект (реализуется в наследниках)
  // принимает каретку и мяч/флаги по ссылке
  virtual void apply(sf::RectangleShape &paddle, float &paddleW, float &ballSpeed, bool &ballSticky, bool &hasFloor, bool &spawnSecondBall) = 0;

  // название для отладки / HUD
  virtual std::string name() const = 0;
};

// бонус расширения каретки
class BonusPaddleGrow : public Bonus {
public:
  explicit BonusPaddleGrow(sf::Vector2f pos)
      : Bonus(pos, sf::Color(50, 200, 100)) {}

  void apply(sf::RectangleShape &paddle, float &paddleW, float & /*ballSpeed*/, bool & /*sticky*/, bool & /*floor*/, bool & /*secondBall*/) override {
    paddleW = std::min(paddleW + 30.f, PADDLE_MAX_W);
    paddle.setSize({paddleW, PADDLE_H});
  }
  std::string name() const override { return "PaddleGrow"; }
};

// бонус уменьшения каретки
class BonusPaddleShrink : public Bonus {
public:
  explicit BonusPaddleShrink(sf::Vector2f pos) : Bonus(pos, sf::Color(220, 80, 80)) {}

  void apply(sf::RectangleShape &paddle, float &paddleW, float & /*ballSpeed*/, bool & /*sticky*/, bool & /*floor*/, bool & /*secondBall*/) override {
    paddleW = std::max(paddleW - 30.f, PADDLE_MIN_W);
    paddle.setSize({paddleW, PADDLE_H});
  }
  std::string name() const override { return "PaddleShrink"; }
};

// бонус ускорения мяча
class BonusSpeedUp : public Bonus {
public:
  explicit BonusSpeedUp(sf::Vector2f pos) : Bonus(pos, sf::Color(255, 160, 20)) {}

  void apply(sf::RectangleShape & /*paddle*/, float &paddleW, float &ballSpeed, bool & /*sticky*/, bool & /*floor*/, bool & /*secondBall*/) override {
    (void)paddleW;
    ballSpeed = std::min(ballSpeed + BALL_SPEED_BOOST, BALL_SPEED_MAX);
  }
  std::string name() const override { return "SpeedUp"; }
};

// бонус замедления мяча
class BonusSlowDown : public Bonus {
public:
  explicit BonusSlowDown(sf::Vector2f pos) : Bonus(pos, sf::Color(80, 180, 255)) {}

  void apply(sf::RectangleShape & /*paddle*/, float &paddleW, float &ballSpeed, bool & /*sticky*/, bool & /*floor*/, bool & /*secondBall*/) override {
    (void)paddleW;
    ballSpeed = std::max(ballSpeed - BALL_SPEED_BOOST, BALL_SPEED);
  }
  std::string name() const override { return "SlowDown"; }
};

// бонус (прилипание мяча к каретке)
class BonusSticky : public Bonus {
public:
  explicit BonusSticky(sf::Vector2f pos) : Bonus(pos, sf::Color(180, 80, 220)) {}

  void apply(sf::RectangleShape & /*paddle*/, float &paddleW, float & /*ballSpeed*/, bool &sticky, bool & /*floor*/, bool & /*secondBall*/) override {
    (void)paddleW;
    sticky = true;
  }
  std::string name() const override { return "Sticky"; }
};

// бонус (одноразовое дно)
class BonusFloor : public Bonus {
public:
  explicit BonusFloor(sf::Vector2f pos) : Bonus(pos, sf::Color(255, 220, 50)) {}

  void apply(sf::RectangleShape & /*paddle*/, float &paddleW, float & /*ballSpeed*/, bool & /*sticky*/, bool &hasFloor, bool & /*secondBall*/) override {
    (void)paddleW;
    hasFloor = true;
  }
  std::string name() const override { return "Floor"; }
};

// бонус (второй шарик)
class BonusSecondBall : public Bonus {
public:
  explicit BonusSecondBall(sf::Vector2f pos) : Bonus(pos, sf::Color(255, 100, 200)) {}

  void apply(sf::RectangleShape & /*paddle*/, float &paddleW, float & /*ballSpeed*/, bool & /*sticky*/, bool & /*floor*/, bool &spawnSecondBall) override {
    (void)paddleW;
    spawnSecondBall = true;
  }
  std::string name() const override { return "SecondBall"; }
};

// создаёт случайный бонус в позиции pos
inline std::unique_ptr<Bonus> makeRandomBonus(sf::Vector2f pos) {
  int roll = std::rand() % 6;
  switch (roll) {
  case 0:
    return std::make_unique<BonusPaddleGrow>(pos);
  case 1:
    return std::make_unique<BonusPaddleShrink>(pos);
  case 2:
    return std::make_unique<BonusSpeedUp>(pos);
  case 3:
    return std::make_unique<BonusSlowDown>(pos);
  case 4:
    return std::make_unique<BonusSticky>(pos);
  case 5:
    return std::make_unique<BonusFloor>(pos);
  default:
    return std::make_unique<BonusSecondBall>(pos);
  }
}

// создаёт бонус (второй шарик)
inline std::unique_ptr<Bonus> makeSecondBallBonus(sf::Vector2f pos) {
  return std::make_unique<BonusSecondBall>(pos);
}
