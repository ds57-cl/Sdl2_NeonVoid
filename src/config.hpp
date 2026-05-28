#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SDL.h>

// Screen configurations
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TARGET_FPS = 60;
const int FRAME_DELAY = 1000 / TARGET_FPS;

// Neon colors (RGBA)
const SDL_Color COLOR_BG = {10, 10, 15, 255};          // Deep space dark blue/black
const SDL_Color COLOR_PLAYER = {0, 255, 255, 255};     // Cyan
const SDL_Color COLOR_PLAYER_GLOW = {0, 255, 255, 60}; // Faded Cyan for glow
const SDL_Color COLOR_SHIELD = {0, 128, 255, 255};     // Blue shield
const SDL_Color COLOR_SHIELD_GLOW = {0, 128, 255, 40};

const SDL_Color COLOR_ENEMY_DRONE = {50, 255, 50, 255}; // Bright Green
const SDL_Color COLOR_ENEMY_DRONE_GLOW = {50, 255, 50, 60};

const SDL_Color COLOR_ENEMY_SCOUT = {255, 255, 0, 255}; // Neon Yellow
const SDL_Color COLOR_ENEMY_SCOUT_GLOW = {255, 255, 0, 60};

const SDL_Color COLOR_ENEMY_BOMBER = {255, 128, 0, 255}; // Neon Orange
const SDL_Color COLOR_ENEMY_BOMBER_GLOW = {255, 128, 0, 60};

const SDL_Color COLOR_BOSS = {255, 0, 128, 255}; // Neon Magenta
const SDL_Color COLOR_BOSS_GLOW = {255, 0, 128, 60};

const SDL_Color COLOR_BULLET_PLAYER = {0, 255, 255, 255}; // Cyan bullets
const SDL_Color COLOR_BULLET_ENEMY = {255, 50, 50, 255};  // Neon Red bullets

const SDL_Color COLOR_PARTICLE_FIRE = {255, 100, 0, 200};   // Thruster particles
const SDL_Color COLOR_PARTICLE_SPARK = {255, 220, 50, 200}; // Sparks
const SDL_Color COLOR_WHITE = {255, 255, 255, 255};
const SDL_Color COLOR_TEXT_GLOW = {0, 255, 255, 80};

// Gameplay constants
const float PLAYER_SPEED = 400.0f;
const float PLAYER_FRICTION = 0.90f;
const int PLAYER_MAX_HEALTH = 100;
const int PLAYER_MAX_SHIELD = 100;
const float SHIELD_REGEN_COOLDOWN = 4.0f; // Seconds before shield starts regenerating
const float SHIELD_REGEN_RATE = 25.0f;    // Shield regenerated per second

const float BULLET_SPEED_PLAYER = 700.0f;
const float BULLET_SPEED_ENEMY = 350.0f;

const float SPAWN_INTERVAL_INITIAL = 2.5f; // Initial enemy spawn delay in seconds
const float SPAWN_INTERVAL_MIN = 0.8f;     // Minimum spawn interval as levels progress

#endif // CONFIG_HPP
