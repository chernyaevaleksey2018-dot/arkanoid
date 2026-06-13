#pragma once

// окно
constexpr int WIN_W = 800;
constexpr int WIN_H = 600;

// каретка
constexpr float PADDLE_W = 100.f;
constexpr float PADDLE_H = 14.f;
constexpr float PADDLE_Y = WIN_H - 40.f;
constexpr float PADDLE_SPEED = 420.f;
constexpr float PADDLE_MIN_W = 50.f;
constexpr float PADDLE_MAX_W = 200.f;

// мяч
constexpr float BALL_R = 8.f;
constexpr float BALL_SPEED = 320.f;
constexpr float BALL_SPEED_BOOST = 60.f;
constexpr float BALL_SPEED_MAX = 620.f;

// блоки
constexpr int BRICK_COLS  = 12;
constexpr int BRICK_ROWS  = 6;
constexpr float BRICK_W = 56.f;
constexpr float BRICK_H = 20.f;
constexpr float BRICK_PAD = 6.f;
constexpr float BRICK_OFF_X = 28.f;
constexpr float BRICK_OFF_Y = 50.f;

// бонусы
constexpr float BONUS_W = 14.f;
constexpr float BONUS_H = 14.f;
constexpr float BONUS_SPEED = 150.f;

// вероятности типов блоков
constexpr float PROB_INDESTRUCTIBLE = 0.10f;
constexpr float PROB_SPEEDUP = 0.10f;
constexpr float PROB_BONUS_BLOCK = 0.20f; // вероятность блока с бонусом
