#ifndef GAME_HPP
#define GAME_HPP

#include <SDL.h>
#include <vector>
#include <string>
#include "entity.hpp"
#include "bullet.hpp"
#include "particle.hpp"
#include "audio.hpp"
#include "hud.hpp"

enum class GameState
{
    START_SCREEN,
    GET_READY,
    PLAYING,
    PAUSED,
    GAME_OVER,
    BOSS_WARNING
};

enum class PowerUpType
{
    WEAPON,
    SHIELD
};

struct PowerUp
{
    float x, y;
    float vy;
    float radius;
    PowerUpType type;
    bool dead;
};

class Game
{
public:
    Game();
    ~Game();

    bool init(SDL_Window *window, SDL_Renderer *renderer);
    void handleEvents();
    void update(float dt);
    void render();
    bool isRunning() const { return m_running; }

private:
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_renderer = nullptr;
    bool m_running = true;

    GameState m_state;
    Player *m_player = nullptr;
    std::vector<Enemy *> m_enemies;
    std::vector<Bullet> m_bullets;
    std::vector<PowerUp> m_powerups;

    ParticleSystem m_particles;
    AudioSystem m_audio;
    HUD m_hud;

    // Game stats
    int m_score;
    int m_highScore;
    int m_level;
    bool m_bossActive;
    bool m_bossSpawnedThisLevel;
    float m_stateTimer;
    float m_spawnTimer;
    float m_spawnInterval;
    int m_enemiesKilledThisLevel;

    // Screen Shake
    float m_shakeDuration;
    float m_shakeIntensity;
    float m_shakeOffsetX;
    float m_shakeOffsetY;
    bool m_spacePressed;

    void triggerScreenShake(float duration, float intensity);
    void spawnEnemy();
    void spawnBoss();
    void updateCollisions();
    void spawnPowerUp(float x, float y);
    void nextLevel();
    void resetGame();
    void loadHighScore();
    void saveHighScore();

    void drawPowerUp(const PowerUp &p);
};

#endif // GAME_HPP
