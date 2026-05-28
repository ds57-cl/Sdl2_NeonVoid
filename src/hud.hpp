#ifndef HUD_HPP
#define HUD_HPP

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

class HUD {
public:
    HUD();
    ~HUD();

    bool init();
    void render(SDL_Renderer* renderer, int health, int maxHealth, int shield, int maxShield, int score, int highScore, int level, const std::string& weaponName, int activeWeaponIdx = 0, int maxUnlockedWeaponIdx = 0);
    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, SDL_Color glowColor, bool center = false, bool large = false);
    void renderMessage(SDL_Renderer* renderer, const std::string& title, const std::string& subtitle);

private:
    TTF_Font* m_fontSmall = nullptr;
    TTF_Font* m_fontLarge = nullptr;

    void drawGlowRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color color, SDL_Color glowColor);
    void drawGlowProgressBar(SDL_Renderer* renderer, int x, int y, int width, int height, float value, SDL_Color color, SDL_Color glowColor);
};

#endif // HUD_HPP
