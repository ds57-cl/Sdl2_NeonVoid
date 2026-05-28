#include "hud.hpp"
#include "config.hpp"
#include <iostream>
#include <vector>
#include <sys/stat.h>

HUD::HUD() {}

HUD::~HUD() {
    if (m_fontSmall) {
        TTF_CloseFont(m_fontSmall);
    }
    if (m_fontLarge) {
        TTF_CloseFont(m_fontLarge);
    }
    TTF_Quit();
}

bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool HUD::init() {
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // List of fallback font paths on macOS
    std::vector<std::string> fontPaths = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial Black.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Geneva.ttf",
        "/System/Library/Fonts/Supplemental/Andale Mono.ttf",
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/System/Library/Fonts/Cache/Arial.ttf"
    };

    std::string selectedFont = "";
    for (const auto& path : fontPaths) {
        if (fileExists(path)) {
            selectedFont = path;
            break;
        }
    }

    if (selectedFont.empty()) {
        std::cerr << "Warning: No system fonts found from standard macOS paths! Using fallback." << std::endl;
        // Try a very generic path just in case
        selectedFont = "/System/Library/Fonts/Helvetica.ttc";
    }

    m_fontSmall = TTF_OpenFont(selectedFont.c_str(), 18);
    m_fontLarge = TTF_OpenFont(selectedFont.c_str(), 36);

    if (!m_fontSmall || !m_fontLarge) {
        std::cerr << "Failed to load font from: " << selectedFont << "! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

void HUD::renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, SDL_Color glowColor, bool center, bool large) {
    TTF_Font* font = large ? m_fontLarge : m_fontSmall;
    if (!font) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // 1. Render Glow Pass (draw offset shadow layers in glow color)
    SDL_Surface* glowSurface = TTF_RenderText_Blended(font, text.c_str(), glowColor);
    if (glowSurface) {
        SDL_Texture* glowTexture = SDL_CreateTextureFromSurface(renderer, glowSurface);
        if (glowTexture) {
            int w = glowSurface->w;
            int h = glowSurface->h;
            int tx = center ? x - w / 2 : x;
            int ty = y;

            // Draw at 4 offsets to create a bloom effect
            SDL_Rect dstRect;
            int offsets[4][2] = { {-2, 0}, {2, 0}, {0, -2}, {0, 2} };
            for (int i = 0; i < 4; ++i) {
                dstRect = { tx + offsets[i][0], ty + offsets[i][1], w, h };
                SDL_RenderCopy(renderer, glowTexture, nullptr, &dstRect);
            }
            
            SDL_DestroyTexture(glowTexture);
        }
        SDL_FreeSurface(glowSurface);
    }

    // 2. Render Core Pass (sharp text on top)
    SDL_Surface* coreSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (coreSurface) {
        SDL_Texture* coreTexture = SDL_CreateTextureFromSurface(renderer, coreSurface);
        if (coreTexture) {
            int w = coreSurface->w;
            int h = coreSurface->h;
            int tx = center ? x - w / 2 : x;
            int ty = y;
            SDL_Rect dstRect = { tx, ty, w, h };
            SDL_RenderCopy(renderer, coreTexture, nullptr, &dstRect);
            SDL_DestroyTexture(coreTexture);
        }
        SDL_FreeSurface(coreSurface);
    }
}

void HUD::drawGlowRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color color, SDL_Color glowColor) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Draw outer glow outline
    SDL_SetRenderDrawColor(renderer, glowColor.r, glowColor.g, glowColor.b, 60);
    SDL_Rect rOut = { rect.x - 1, rect.y - 1, rect.w + 2, rect.h + 2 };
    SDL_RenderDrawRect(renderer, &rOut);
    SDL_Rect rOut2 = { rect.x - 2, rect.y - 2, rect.w + 4, rect.h + 4 };
    SDL_RenderDrawRect(renderer, &rOut2);

    // Draw core outline
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}

void HUD::drawGlowProgressBar(SDL_Renderer* renderer, int x, int y, int width, int height, float value, SDL_Color color, SDL_Color glowColor) {
    value = std::clamp(value, 0.0f, 1.0f);

    // 1. Draw Border Rectangle (Glowing outline)
    SDL_Rect border = { x, y, width, height };
    drawGlowRect(renderer, border, COLOR_WHITE, COLOR_TEXT_GLOW);

    // 2. Draw Filled Progress
    if (value > 0.0f) {
        int fillWidth = static_cast<int>((width - 4) * value);
        if (fillWidth < 1) fillWidth = 1;
        
        SDL_Rect fillRect = { x + 2, y + 2, fillWidth, height - 4 };

        // Outer glow pass for the filling bar
        SDL_SetRenderDrawColor(renderer, glowColor.r, glowColor.g, glowColor.b, 70);
        SDL_Rect glowFill = { fillRect.x - 1, fillRect.y - 1, fillRect.w + 2, fillRect.h + 2 };
        SDL_RenderFillRect(renderer, &glowFill);

        // Core pass
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderFillRect(renderer, &fillRect);
    }
}

void HUD::render(SDL_Renderer* renderer, int health, int maxHealth, int shield, int maxShield, int score, int highScore, int level, const std::string& weaponName, int activeWeaponIdx, int maxUnlockedWeaponIdx) {
    (void)weaponName; // Unused now that we render the visual panel

    // 1. Shield bar (Left side)
    renderText(renderer, "SHIELD", 20, 15, COLOR_WHITE, COLOR_SHIELD_GLOW);
    float shieldVal = static_cast<float>(shield) / maxShield;
    drawGlowProgressBar(renderer, 20, 38, 200, 16, shieldVal, COLOR_SHIELD, COLOR_SHIELD_GLOW);

    // 2. Health bar (Left side)
    renderText(renderer, "ARMOR", 20, 58, COLOR_WHITE, COLOR_ENEMY_DRONE_GLOW);
    float healthVal = static_cast<float>(health) / maxHealth;
    drawGlowProgressBar(renderer, 20, 81, 200, 16, healthVal, COLOR_ENEMY_DRONE, COLOR_ENEMY_DRONE_GLOW);

    // 3. Stage indicator (Center top)
    renderText(renderer, "STAGE " + std::to_string(level), SCREEN_WIDTH / 2, 15, COLOR_WHITE, COLOR_TEXT_GLOW, true);

    // 4. Scores (Right side)
    std::string scoreStr = "SCORE: " + std::to_string(score);
    std::string hiScoreStr = "HIGH: " + std::to_string(highScore);
    renderText(renderer, scoreStr, SCREEN_WIDTH - 220, 15, COLOR_WHITE, COLOR_TEXT_GLOW);
    renderText(renderer, hiScoreStr, SCREEN_WIDTH - 220, 42, COLOR_ENEMY_SCOUT, COLOR_ENEMY_SCOUT_GLOW);

    // 5. Weapon Selection Panel (Right side)
    int panelX = SCREEN_WIDTH - 220;
    int panelY = 90;
    int panelWidth = 200;
    int panelHeight = 175;

    // Draw panel container
    SDL_Rect panelRect = { panelX, panelY, panelWidth, panelHeight };
    drawGlowRect(renderer, panelRect, COLOR_WHITE, COLOR_TEXT_GLOW);
    
    // Draw header inside panel container
    renderText(renderer, "WEAPONS ARMORY", panelX + 10, panelY + 8, COLOR_WHITE, COLOR_TEXT_GLOW);
    
    // Separator line
    SDL_SetRenderDrawColor(renderer, COLOR_WHITE.r, COLOR_WHITE.g, COLOR_WHITE.b, 60);
    SDL_RenderDrawLine(renderer, panelX + 10, panelY + 30, panelX + panelWidth - 10, panelY + 30);

    const std::vector<std::string> weaponNames = {
        "1: SINGLE CANNON",
        "2: DUAL BLASTER",
        "3: SPREAD SHOT",
        "4: PLASMA WAVE"
    };

    for (int i = 0; i < 4; ++i) {
        int slotY = panelY + 38 + i * 32;
        SDL_Rect slotRect = { panelX + 10, slotY, panelWidth - 20, 26 };
        
        bool unlocked = (i <= maxUnlockedWeaponIdx);
        bool active = (i == activeWeaponIdx);
        
        if (active) {
            // Neon cyan background fill for active slot
            SDL_SetRenderDrawColor(renderer, COLOR_PLAYER.r, COLOR_PLAYER.g, COLOR_PLAYER.b, 35);
            SDL_RenderFillRect(renderer, &slotRect);
            
            // Neon cyan outline
            drawGlowRect(renderer, slotRect, COLOR_PLAYER, COLOR_PLAYER_GLOW);
            
            // Active text
            renderText(renderer, weaponNames[i], panelX + 26, slotY + 4, COLOR_PLAYER, COLOR_PLAYER_GLOW);
            
            // Draw select pointer '>'
            SDL_Color pointerColor = COLOR_PLAYER;
            SDL_SetRenderDrawColor(renderer, pointerColor.r, pointerColor.g, pointerColor.b, 255);
            SDL_RenderDrawLine(renderer, panelX + 16, slotY + 8, panelX + 20, slotY + 13);
            SDL_RenderDrawLine(renderer, panelX + 20, slotY + 13, panelX + 16, slotY + 18);
        } else if (unlocked) {
            // Unlocked but inactive slot outline (faded white/gray)
            SDL_SetRenderDrawColor(renderer, COLOR_WHITE.r, COLOR_WHITE.g, COLOR_WHITE.b, 80);
            SDL_RenderDrawRect(renderer, &slotRect);
            
            // Unlocked text
            renderText(renderer, weaponNames[i], panelX + 26, slotY + 4, COLOR_WHITE, COLOR_TEXT_GLOW);
        } else {
            // Locked slot outline (faded red/dark)
            SDL_Color lockedColor = { 100, 100, 100, 255 };
            SDL_Color lockedGlow = { 255, 0, 0, 30 };
            SDL_SetRenderDrawColor(renderer, lockedColor.r, lockedColor.g, lockedColor.b, 60);
            SDL_RenderDrawRect(renderer, &slotRect);
            
            // Locked text
            renderText(renderer, "LOCKED", panelX + 26, slotY + 4, lockedColor, lockedGlow);
        }
    }
}

void HUD::renderMessage(SDL_Renderer* renderer, const std::string& title, const std::string& subtitle) {
    // Semi-transparent overlay background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 5, 5, 8, 160);
    SDL_Rect overlay = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &overlay);

    // Draw central box glowing outline
    SDL_Rect box = { SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2 - 100, 500, 200 };
    drawGlowRect(renderer, box, COLOR_PLAYER, COLOR_PLAYER_GLOW);

    // Draw Title
    renderText(renderer, title, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 60, COLOR_WHITE, COLOR_PLAYER_GLOW, true, true);

    // Draw Subtitle (flashing effect using sine of ticks)
    Uint32 ticks = SDL_GetTicks();
    Uint8 alpha = static_cast<Uint8>(120 + 135 * sin(ticks * 0.006));
    SDL_Color subColor = { 255, 255, 255, alpha };
    SDL_Color subGlow = COLOR_PLAYER_GLOW;
    subGlow.a = static_cast<Uint8>(alpha * 0.25f);
    renderText(renderer, subtitle, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 25, subColor, subGlow, true, false);
}
