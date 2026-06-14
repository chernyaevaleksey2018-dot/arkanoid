#pragma once

// отвечает только за счёт и количество потерь
class ScoreKeeper {
public:
    int score = 0;
    int losses = 0;

    void addScore(int pts = 1) { score += pts; }

    void addLoss() {
        losses++;
        score -= 3;
        if (score < 0) score = 0;
    }

    bool isGameOver() const { return losses >= 5; }

    void reset() {
        score = 0;
        losses = 0;
    }
};
