#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Constants.hpp"

class HUD {
public:
    bool fontLoaded = false;

    HUD() = default;

    bool loadFont(const std::string& path) {
        fontLoaded = font.openFromFile(path);
        if (fontLoaded) initTexts();
        return fontLoaded;
    }

    // шрифты разными путями
    bool loadFontAuto() {
        static const char* paths[] = {
            "/usr/share/fonts/TTF/HackNerdFont-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
            "/Library/Fonts/Arial.ttf",
            "C:/Windows/Fonts/arial.ttf",
            nullptr
        };
        for (int i = 0; paths[i]; ++i) {
            if (font.openFromFile(paths[i])) {
                fontLoaded = true;
                initTexts();
                return true;
            }
        }
        return false;
    }

    const sf::Font& getFont() const { return font; }

    void update(int score, int losses, float ballSpeed, bool sticky, bool floor, int ballCount) {
        if (!fontLoaded) return;
        scoreText.setString("Score: " + std::to_string(score));
        lossText.setString("Misses: " + std::to_string(losses) + " / 5");

        std::string spd = std::to_string(ballSpeed / BALL_SPEED);
        spd = spd.substr(0, spd.find('.') + 2);
        speedText.setString("Speed: x" + spd);

        std::string status;
        if (sticky)  status += " [STICKY]";
        if (floor)   status += " [FLOOR]";
        if (ballCount > 1) status += " [x" + std::to_string(ballCount) + "]";
        statusText.setString(status);
    }

    void draw(sf::RenderWindow& window) const {
        if (!fontLoaded) return;
        window.draw(scoreText);
        window.draw(lossText);
        window.draw(speedText);
        window.draw(statusText);
    }

    void drawEnd(sf::RenderWindow& window, const std::string& msg) {
        if (!fontLoaded) return;
        endText.setString(msg);
        sf::FloatRect bounds = endText.getLocalBounds();
        endText.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
        endText.setPosition({WIN_W / 2.f, WIN_H / 2.f});
        window.draw(endText);
    }

private:
    sf::Font font;
    sf::Text scoreText{font}, lossText{font}, speedText{font}, statusText{font}, endText{font};

    void initTexts() {
        auto setup = [&](sf::Text& t, unsigned size, sf::Color color = sf::Color::White) {
            t.setFont(font);
            t.setCharacterSize(size);
            t.setFillColor(color);
        };
        setup(scoreText, 20);
        setup(lossText, 20);
        setup(speedText, 20);
        setup(statusText, 16, sf::Color(200, 200, 100));
        setup(endText, 36, sf::Color::Yellow);

        scoreText.setPosition({10.f, 10.f});
        lossText.setPosition({WIN_W - 160.f, 10.f});
        speedText.setPosition({WIN_W / 2.f - 50.f, 10.f});
        statusText.setPosition({10.f, WIN_H - 22.f});
    }
};
