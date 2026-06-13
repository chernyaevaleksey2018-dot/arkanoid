#pragma once
#include "Constants.hpp"
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>

enum class BrickType { Normal, Indestructible, SpeedUp, BonusBlock };

inline sf::Color colorByHp(int hp) {
  switch (hp) {
  case 1:
    return sf::Color(60, 210, 90);
  case 2:
    return sf::Color(220, 210, 50);
  case 3:
    return sf::Color(230, 130, 40);
  case 4:
    return sf::Color(210, 50, 50);
  default:
    return sf::Color(180, 180, 180);
  }
}

struct Brick {
  sf::RectangleShape shape;
  int hp = 1;
  bool active = true;
  BrickType type = BrickType::Normal;
  bool hasBonus = false; // при уничтожении выпадает бонус

  Brick() { shape.setOutlineThickness(1.f); }
};

// инициализирует сетку блоков (вызывать при старте/рестарте)
inline void initBricks(std::vector<Brick> &bricks) {
  bricks.clear();
  std::srand(static_cast<unsigned>(std::time(nullptr)));

  const int INNER_COLS = BRICK_COLS - 2; // 10
  const int INNER_ROWS = BRICK_ROWS - 1; // 5
  const int MIN_OPEN = 4;

  // предварительно строим сетку для внутренних ячеек
  std::vector<std::vector<BrickType>> typeGrid(INNER_ROWS, std::vector<BrickType>(INNER_COLS, BrickType::Normal));
  std::vector<std::vector<int>> hpGrid(INNER_ROWS, std::vector<int>(INNER_COLS, 1));
  std::vector<std::vector<bool>> bonusGrid(INNER_ROWS, std::vector<bool>(INNER_COLS, false));

  for (int ir = 0; ir < INNER_ROWS; ++ir) {
    //для случайного порядка
    std::vector<int> cols(INNER_COLS);
    for (int i = 0; i < INNER_COLS; ++i)
      cols[i] = i;
    for (int i = INNER_COLS - 1; i > 0; --i) {
      int j = std::rand() % (i + 1);
      std::swap(cols[i], cols[j]);
    }

    int indestrCount = 0;
    int maxIndestr = INNER_COLS - MIN_OPEN;

    for (int ic = 0; ic < INNER_COLS; ++ic) {
      float roll = static_cast<float>(std::rand()) / RAND_MAX;
      int col = cols[ic];

      if (roll < PROB_INDESTRUCTIBLE && indestrCount < maxIndestr) {
        typeGrid[ir][col] = BrickType::Indestructible;
        ++indestrCount;
      } else if (roll < PROB_INDESTRUCTIBLE + PROB_SPEEDUP) {
        typeGrid[ir][col] = BrickType::SpeedUp;
        hpGrid[ir][col] = 1;
      } else {
        typeGrid[ir][col] = BrickType::Normal;
        hpGrid[ir][col] = 1 + std::rand() % 4;

        // случайно добавляем бонус к обычному блоку
        float bonusRoll = static_cast<float>(std::rand()) / RAND_MAX;
        bonusGrid[ir][col] = (bonusRoll < PROB_BONUS_BLOCK);
      }
    }
  }

  // строим вектор блоков из сетки
  for (int r = 0; r < BRICK_ROWS; ++r) {
    for (int c = 0; c < BRICK_COLS; ++c) {
      Brick b;
      float x = BRICK_OFF_X + c * (BRICK_W + BRICK_PAD);
      float y = BRICK_OFF_Y + r * (BRICK_H + BRICK_PAD);
      b.shape.setSize({BRICK_W, BRICK_H});
      b.shape.setPosition({x, y});

      bool isBorder = (c == 0 || c == BRICK_COLS - 1 || r == 0);

      if (isBorder) {
        b.type = BrickType::Indestructible;
        b.hp = -1;
        b.shape.setFillColor(sf::Color(100, 100, 115));
        b.shape.setOutlineColor(sf::Color(60, 60, 75));
      } else {
        int ir = r - 1;
        int ic = c - 1;
        b.type = typeGrid[ir][ic];
        b.hasBonus = bonusGrid[ir][ic];

        switch (b.type) {
        case BrickType::Indestructible:
          b.hp = -1;
          b.shape.setFillColor(sf::Color(100, 100, 115));
          b.shape.setOutlineColor(sf::Color(60, 60, 75));
          break;
        case BrickType::SpeedUp:
          b.hp = 1;
          b.shape.setFillColor(sf::Color(180, 80, 220));
          b.shape.setOutlineColor(sf::Color(120, 40, 160));
          break;
        case BrickType::Normal:
        default:
          b.hp = hpGrid[ir][ic];
          b.shape.setFillColor(colorByHp(b.hp));
          b.shape.setOutlineColor(sf::Color(0, 0, 0, 80));
          break;
        }
      }
      bricks.push_back(b);
    }
  }
}

// возвращает true, если все разрушимые блоки уничтожены
inline bool allBricksCleared(const std::vector<Brick> &bricks) {
  for (const auto &b : bricks)
    if (b.active && b.hp != -1)
      return false;
  return true;
}
