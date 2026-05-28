#include "game.hpp"
#include "config.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

Game::Game()
    : m_state(GameState::START_SCREEN),
      m_score(0),
      m_highScore(0),
      m_level(1),
      m_bossActive(false),
      m_bossSpawnedThisLevel(false),
      m_stateTimer(0.0f),
      m_spawnTimer(0.0f),
      m_spawnInterval(SPAWN_INTERVAL_INITIAL),
      m_enemiesKilledThisLevel(0),
      m_shakeDuration(0.0f),
      m_shakeIntensity(0.0f),
      m_shakeOffsetX(0.0f),
      m_shakeOffsetY(0.0f),
      m_spacePressed(false) {}

Game::~Game()
{
    delete m_player;
    for (auto *enemy : m_enemies)
    {
        delete enemy;
    }
    m_enemies.clear();
}

bool Game::init(SDL_Window *window, SDL_Renderer *renderer)
{
    m_window = window;
    m_renderer = renderer;

    // Load High Score
    loadHighScore();

    // Init Audio
    if (!m_audio.init())
    {
        std::cerr << "Failed to initialize Audio System!" << std::endl;
        return false;
    }

    // Init HUD / Fonts
    if (!m_hud.init())
    {
        std::cerr << "Failed to initialize HUD System!" << std::endl;
        return false;
    }

    // Init Stars
    m_particles.initStars(80);

    // Create player
    m_player = new Player(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - 100.0f);

    return true;
}

void Game::resetGame()
{
    delete m_player;
    m_player = new Player(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - 100.0f);

    for (auto *enemy : m_enemies)
    {
        delete enemy;
    }
    m_enemies.clear();
    m_bullets.clear();
    m_powerups.clear();
    m_particles.clear();
    m_particles.initStars(80);

    m_score = 0;
    m_level = 1;
    m_bossActive = false;
    m_bossSpawnedThisLevel = false;
    m_spawnInterval = SPAWN_INTERVAL_INITIAL;
    m_enemiesKilledThisLevel = 0;
    m_spacePressed = false;
    m_stateTimer = 2.0f; // Get Ready duration
    m_state = GameState::GET_READY;
}

void Game::loadHighScore()
{
    std::ifstream file("highscore.txt");
    if (file.is_open())
    {
        file >> m_highScore;
        file.close();
    }
    else
    {
        m_highScore = 0;
    }
}

void Game::saveHighScore()
{
    if (m_score > m_highScore)
    {
        m_highScore = m_score;
        std::ofstream file("highscore.txt");
        if (file.is_open())
        {
            file << m_highScore;
            file.close();
        }
    }
}

void Game::handleEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            m_running = false;
        }
        else if (e.type == SDL_KEYDOWN)
        {
            if (e.key.keysym.sym == SDLK_ESCAPE)
            {
                if (m_state == GameState::PLAYING)
                {
                    m_state = GameState::PAUSED;
                }
                else if (m_state == GameState::PAUSED)
                {
                    m_state = GameState::PLAYING;
                }
            }
            else if (e.key.keysym.sym == SDLK_RETURN)
            {
                if (m_state == GameState::START_SCREEN || m_state == GameState::GAME_OVER)
                {
                    resetGame();
                }
                else if (m_state == GameState::PAUSED)
                {
                    m_state = GameState::PLAYING;
                }
            }
            else if (e.key.keysym.sym == SDLK_SPACE)
            {
                m_spacePressed = true;
            }
            else if (e.key.keysym.sym >= SDLK_1 && e.key.keysym.sym <= SDLK_4)
            {
                int index = e.key.keysym.sym - SDLK_1;
                m_player->selectWeapon(index);
            }
            else if (e.key.keysym.sym == SDLK_q)
            {
                int current = static_cast<int>(m_player->getWeaponType());
                int maxUnlocked = m_player->getMaxUnlockedWeapon();
                int prev = current - 1;
                if (prev < 0) prev = maxUnlocked;
                m_player->selectWeapon(prev);
            }
            else if (e.key.keysym.sym == SDLK_e)
            {
                int current = static_cast<int>(m_player->getWeaponType());
                int maxUnlocked = m_player->getMaxUnlockedWeapon();
                int next = current + 1;
                if (next > maxUnlocked) next = 0;
                m_player->selectWeapon(next);
            }
        }
        else if (e.type == SDL_KEYUP)
        {
            if (e.key.keysym.sym == SDLK_SPACE)
            {
                m_spacePressed = false;
            }
        }
        else if (e.type == SDL_WINDOWEVENT)
        {
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            {
                m_spacePressed = false;
                if (m_state == GameState::PLAYING)
                {
                    m_state = GameState::PAUSED;
                }
            }
        }
    }
}

void Game::triggerScreenShake(float duration, float intensity)
{
    m_shakeDuration = duration;
    m_shakeIntensity = intensity;
}

void Game::spawnEnemy()
{
    if (m_bossActive)
        return;

    float x = static_cast<float>(30 + rand() % (SCREEN_WIDTH - 60));
    float y = -30.0f;

    int r = rand() % 100;
    EnemyType type;

    // As levels increase, spawn harder enemies
    if (m_level == 1)
    {
        if (r < 75)
            type = EnemyType::DRONE;
        else
            type = EnemyType::SCOUT;
    }
    else if (m_level == 2)
    {
        if (r < 50)
            type = EnemyType::DRONE;
        else if (r < 85)
            type = EnemyType::SCOUT;
        else
            type = EnemyType::BOMBER;
    }
    else
    {
        if (r < 35)
            type = EnemyType::DRONE;
        else if (r < 70)
            type = EnemyType::SCOUT;
        else
            type = EnemyType::BOMBER;
    }

    m_enemies.push_back(new Enemy(x, y, type));
}

void Game::spawnBoss()
{
    m_bossActive = true;
    m_bossSpawnedThisLevel = true;

    // Spawn warning sound
    m_audio.play(SoundID::BOSS_SPAWN);

    // Boss enters from center top
    m_enemies.push_back(new Enemy(SCREEN_WIDTH / 2.0f, -80.0f, EnemyType::BOSS));
}

void Game::spawnPowerUp(float x, float y)
{
    // 15% chance to spawn a powerup when an enemy is destroyed
    if (rand() % 100 < 15)
    {
        PowerUp p;
        p.x = x;
        p.y = y;
        p.vy = 90.0f; // Falls slowly
        p.radius = 10.0f;
        p.dead = false;
        p.type = (rand() % 100 < 40) ? PowerUpType::WEAPON : PowerUpType::SHIELD; // 40% weapon, 60% shield
        m_powerups.push_back(p);
    }
}

void Game::nextLevel()
{
    m_level++;
    m_enemiesKilledThisLevel = 0;
    m_bossActive = false;
    m_bossSpawnedThisLevel = false;

    // Scale spawn speed
    m_spawnInterval = std::max(SPAWN_INTERVAL_MIN, SPAWN_INTERVAL_INITIAL - (m_level - 1) * 0.3f);
    m_spawnTimer = 0.5f;

    // Clear all old enemies and bullets on stage transition
    for (auto *enemy : m_enemies)
    {
        delete enemy;
    }
    m_enemies.clear();
    m_bullets.clear();

    // Heal player partially on stage clear
    m_player->rechargeShield(30);

    // Save score
    saveHighScore();

    m_stateTimer = 2.0f;
    m_state = GameState::GET_READY;
}

void Game::update(float dt)
{
    // 1. Handle Screen Shake updates
    if (m_shakeDuration > 0.0f)
    {
        m_shakeDuration -= dt;
        m_shakeOffsetX = (-m_shakeIntensity + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (2.0f * m_shakeIntensity));
        m_shakeOffsetY = (-m_shakeIntensity + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (2.0f * m_shakeIntensity));
        if (m_shakeDuration <= 0.0f)
        {
            m_shakeOffsetX = 0.0f;
            m_shakeOffsetY = 0.0f;
        }
    }

    // 2. Stars drift in background regardless of pause/gameover (adds life to screens)
    m_particles.update(dt);

    if (m_state == GameState::START_SCREEN || m_state == GameState::GAME_OVER)
    {
        return;
    }

    if (m_state == GameState::PAUSED)
    {
        return;
    }

    if (m_state == GameState::GET_READY || m_state == GameState::BOSS_WARNING)
    {
        m_stateTimer -= dt;

        // Let player move and regenerate shield during warning/ready phases
        const Uint8 *kb = SDL_GetKeyboardState(nullptr);
        m_player->handleInput(kb, dt);
        m_player->update(dt);
        m_player->regenerateShield(dt);

        // Update thruster particles
        float noseX = m_player->getX();
        float noseY = m_player->getY() + m_player->getRadius() * 0.4f;
        m_particles.spawnThrusterTrail(noseX, noseY + 10.0f, 0.0f, 150.0f + (rand() % 100), COLOR_PARTICLE_FIRE);

        // Update existing bullets/particles
        for (auto &bullet : m_bullets)
        {
            bullet.update(dt);
        }
        for (auto &p : m_powerups)
        {
            p.y += p.vy * dt;
        }

        if (m_stateTimer <= 0.0f)
        {
            if (m_state == GameState::BOSS_WARNING)
            {
                m_state = GameState::PLAYING;
                spawnBoss();
            }
            else
            {
                m_state = GameState::PLAYING;
            }
        }
        return;
    }

    // ==================================================
    // PLAYING STATE
    // ==================================================
    const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);

    // Player inputs & physics
    m_player->handleInput(keyboardState, dt);
    m_player->update(dt);
    m_player->regenerateShield(dt);

    // Spawn thruster particles
    float trailX = m_player->getX();
    float trailY = m_player->getY() + m_player->getRadius() * 0.5f;
    m_particles.spawnThrusterTrail(trailX, trailY + 8.0f, 0.0f, 180.0f, COLOR_PARTICLE_FIRE);

    // Player weapon firing
    if (m_spacePressed && m_player->canShoot())
    {
        auto newBullets = m_player->fire();
        for (const auto &b : newBullets)
        {
            m_bullets.push_back(b);
        }
        m_audio.play(SoundID::PLAYER_SHOOT);
        m_player->resetShootCooldown();
    }

    // Spawn regular enemies if boss is not active
    int enemiesToSpawnBeforeBoss = 10 + m_level * 5;
    if (!m_bossActive && !m_bossSpawnedThisLevel)
    {
        m_spawnTimer -= dt;
        if (m_spawnTimer <= 0.0f)
        {
            spawnEnemy();
            m_spawnTimer = m_spawnInterval;
        }

        // Check if level threshold met to trigger Boss fight
        if (m_enemiesKilledThisLevel >= enemiesToSpawnBeforeBoss)
        {
            m_state = GameState::BOSS_WARNING;
            m_stateTimer = 3.0f; // 3 seconds of warning flash
            m_enemies.clear();   // Clear small fry enemies
            return;
        }
    }

    // Update Enemies
    for (auto it = m_enemies.begin(); it != m_enemies.end();)
    {
        Enemy *enemy = *it;
        
        // Remove dead enemies immediately
        if (enemy->isDead())
        {
            delete enemy;
            it = m_enemies.erase(it);
            continue;
        }

        enemy->update(dt);

        // Enemy shooting logic
        if (enemy->canShoot())
        {
            auto enemyBullets = enemy->fire();
            if (!enemyBullets.empty())
            {
                for (const auto &b : enemyBullets)
                {
                    m_bullets.push_back(b);
                }
                m_audio.play(SoundID::ENEMY_SHOOT);
                enemy->resetShootCooldown();
            }
        }

        // Delete enemy if it drifts past bottom screen (except Boss which hovers)
        if (enemy->getY() > SCREEN_HEIGHT + 50.0f && enemy->getType() != EnemyType::BOSS)
        {
            delete enemy;
            it = m_enemies.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Update Bullets
    for (auto it = m_bullets.begin(); it != m_bullets.end();)
    {
        it->update(dt);
        if (it->isDead())
        {
            it = m_bullets.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Update Powerups
    for (auto it = m_powerups.begin(); it != m_powerups.end();)
    {
        it->y += it->vy * dt;
        if (it->dead || it->y > SCREEN_HEIGHT + 30.0f)
        {
            it = m_powerups.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Handle Collisions
    updateCollisions();
}

void Game::updateCollisions()
{
    float playerRadius = m_player->getRadius();
    float px = m_player->getX();
    float py = m_player->getY();

    // 1. Bullets Collisions
    for (auto &bullet : m_bullets)
    {
        if (bullet.isDead())
            continue;

        if (bullet.isPlayerBullet())
        {
            // Player bullets vs Enemies
            for (auto *enemy : m_enemies)
            {
                if (enemy->isDead())
                    continue;

                float ex = enemy->getX();
                float ey = enemy->getY();
                float er = enemy->getRadius();
                float distSq = (bullet.getX() - ex) * (bullet.getX() - ex) + (bullet.getY() - ey) * (bullet.getY() - ey);
                float minDist = bullet.getRadius() + er;

                if (distSq < minDist * minDist)
                {
                    bullet.kill();
                    enemy->takeDamage(bullet.getDamage());

                    // Hit sparks
                    m_particles.spawnSparks(bullet.getX(), bullet.getY(), enemy->getColor(), 5);

                    if (enemy->isDead())
                    {
                        m_audio.play(SoundID::EXPLOSION);
                        m_particles.spawnExplosion(ex, ey, enemy->getColor(), enemy->getType() == EnemyType::BOSS ? 50 : 20);

                        if (enemy->getType() == EnemyType::BOSS)
                        {
                            m_score += 1500;
                            triggerScreenShake(1.2f, 15.0f);
                            // Win stage trigger
                            nextLevel();
                            return; // Stop update loop immediately
                        }
                        else
                        {
                            m_score += 100 + static_cast<int>(enemy->getType()) * 50;
                            m_enemiesKilledThisLevel++;
                            triggerScreenShake(0.2f, 4.0f);
                            spawnPowerUp(ex, ey);
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            // Enemy bullets vs Player
            float distSq = (bullet.getX() - px) * (bullet.getX() - px) + (bullet.getY() - py) * (bullet.getY() - py);
            float minDist = bullet.getRadius() + playerRadius;

            if (distSq < minDist * minDist)
            {
                bullet.kill();
                m_player->hitShield(bullet.getDamage());
                m_audio.play(SoundID::HIT);
                triggerScreenShake(0.35f, 9.0f);
                m_particles.spawnSparks(bullet.getX(), bullet.getY(), COLOR_SHIELD, 6);

                if (m_player->isDead())
                {
                    m_audio.play(SoundID::EXPLOSION);
                    m_particles.spawnExplosion(px, py, COLOR_PLAYER, 60);
                    m_state = GameState::GAME_OVER;
                    saveHighScore();
                }
            }
        }
    }

    // 2. Enemy ramming Player
    for (auto *enemy : m_enemies)
    {
        if (enemy->isDead())
            continue;

        float ex = enemy->getX();
        float ey = enemy->getY();
        float er = enemy->getRadius();
        float distSq = (ex - px) * (ex - px) + (ey - py) * (ey - py);
        float minDist = er + playerRadius;

        if (distSq < minDist * minDist)
        {
            // Player takes heavy crash damage
            int crashDmg = 40;
            if (enemy->getType() == EnemyType::BOSS)
                crashDmg = 80;

            m_player->hitShield(crashDmg);
            enemy->takeDamage(enemy->getHealth()); // Kill enemy instantly

            m_audio.play(SoundID::EXPLOSION);
            m_particles.spawnExplosion(ex, ey, enemy->getColor(), 25);
            triggerScreenShake(0.5f, 14.0f);

            if (enemy->getType() == EnemyType::BOSS)
            {
                nextLevel();
                return;
            }
            else
            {
                m_enemiesKilledThisLevel++;
            }

            if (m_player->isDead())
            {
                m_particles.spawnExplosion(px, py, COLOR_PLAYER, 60);
                m_state = GameState::GAME_OVER;
                saveHighScore();
            }
        }
    }

    // 3. Powerups Collection
    for (auto &p : m_powerups)
    {
        if (p.dead)
            continue;

        float distSq = (p.x - px) * (p.x - px) + (p.y - py) * (p.y - py);
        float minDist = p.radius + playerRadius;

        if (distSq < minDist * minDist)
        {
            p.dead = true;
            m_audio.play(SoundID::POWERUP);

            if (p.type == PowerUpType::SHIELD)
            {
                m_player->rechargeShield(40);
                m_particles.spawnSparks(p.x, p.y, COLOR_SHIELD, 15);
            }
            else
            {
                m_player->upgradeWeapon();
                m_particles.spawnSparks(p.x, p.y, COLOR_BOSS, 15);
            }
        }
    }
}

void Game::drawPowerUp(const PowerUp &p)
{
    float r = p.radius;
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    if (p.type == PowerUpType::SHIELD)
    {
        // Draw blue shield powerup (glowing octagon + cross inside)
        SDL_Color color = COLOR_SHIELD;

        // Glow pass
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, 60);
        for (int i = -1; i <= 1; ++i)
        {
            SDL_RenderDrawLine(m_renderer, p.x - r + i, p.y, p.x + r + i, p.y);
            SDL_RenderDrawLine(m_renderer, p.x, p.y - r + i, p.x, p.y + r + i);
        }

        // Core pass
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(m_renderer, p.x - r + 3, p.y, p.x + r - 3, p.y);
        SDL_RenderDrawLine(m_renderer, p.x, p.y - r + 3, p.x, p.y + r - 3);

        // Border octagon
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, 255);
        float x_pts[8];
        float y_pts[8];
        for (int i = 0; i < 8; ++i)
        {
            float angle = i * M_PI / 4.0f;
            x_pts[i] = p.x + cos(angle) * r;
            y_pts[i] = p.y + sin(angle) * r;
        }
        for (int i = 0; i < 8; ++i)
        {
            SDL_RenderDrawLine(m_renderer, x_pts[i], y_pts[i], x_pts[(i + 1) % 8], y_pts[(i + 1) % 8]);
        }
    }
    else
    {
        // Draw magenta weapon powerup (glowing diamond + inner triangle)
        SDL_Color color = COLOR_BOSS;

        // Glow pass
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, 60);
        SDL_Rect glow = {static_cast<int>(p.x - r - 1), static_cast<int>(p.y - r - 1), static_cast<int>(2 * r + 2), static_cast<int>(2 * r + 2)};
        SDL_RenderDrawRect(m_renderer, &glow);

        // Core diamond
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, 255);
        SDL_RenderDrawLine(m_renderer, p.x, p.y - r, p.x + r, p.y);
        SDL_RenderDrawLine(m_renderer, p.x + r, p.y, p.x, p.y + r);
        SDL_RenderDrawLine(m_renderer, p.x, p.y + r, p.x - r, p.y);
        SDL_RenderDrawLine(m_renderer, p.x - r, p.y, p.x, p.y - r);

        // Inner white arrow
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(m_renderer, p.x, p.y - r * 0.4f, p.x - r * 0.4f, p.y + r * 0.2f);
        SDL_RenderDrawLine(m_renderer, p.x - r * 0.4f, p.y + r * 0.2f, p.x + r * 0.4f, p.y + r * 0.2f);
        SDL_RenderDrawLine(m_renderer, p.x + r * 0.4f, p.y + r * 0.2f, p.x, p.y - r * 0.4f);
    }
}

void Game::render()
{
    // Apply background color to clear renderer
    SDL_SetRenderDrawColor(m_renderer, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, 255);
    SDL_RenderClear(m_renderer);

    // ==========================================
    // SHAKING VIEWPORT PASS
    // ==========================================
    // Set viewport offset by screenshake variables
    SDL_Rect gameViewport = {
        static_cast<int>(m_shakeOffsetX),
        static_cast<int>(m_shakeOffsetY),
        SCREEN_WIDTH,
        SCREEN_HEIGHT};
    SDL_RenderSetViewport(m_renderer, &gameViewport);

    // Render Stars (underneath everything)
    m_particles.renderStars(m_renderer);

    // Render Powerups
    for (const auto &p : m_powerups)
    {
        drawPowerUp(p);
    }

    // Render Bullets
    for (auto &bullet : m_bullets)
    {
        bullet.render(m_renderer);
    }

    // Render Enemies
    for (auto *enemy : m_enemies)
    {
        enemy->render(m_renderer);
    }

    // Render Player
    m_player->render(m_renderer);

    // Render Explosion and Spark Particles
    m_particles.renderParticles(m_renderer);

    // ==========================================
    // STEADY HUD PASS
    // ==========================================
    // Reset viewport so HUD doesn't shake or offset
    SDL_Rect defaultViewport = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderSetViewport(m_renderer, &defaultViewport);

    // Determine weapon label
    std::string weaponLabel = "SINGLE";
    if (m_player)
    {
        switch (m_player->getWeaponType())
        {
        case WeaponType::SINGLE:
            weaponLabel = "SINGLE CANNON";
            break;
        case WeaponType::DOUBLE:
            weaponLabel = "DUAL BLASTER";
            break;
        case WeaponType::TRIPLE:
            weaponLabel = "SPREAD SHOT";
            break;
        case WeaponType::PLASMA:
            weaponLabel = "PLASMA WAVE";
            break;
        }
    }

    // Render general HUD
    m_hud.render(m_renderer,
                 m_player->getHealth(), PLAYER_MAX_HEALTH,
                 m_player->getShield(), PLAYER_MAX_SHIELD,
                 m_score, m_highScore, m_level, weaponLabel,
                 static_cast<int>(m_player->getWeaponType()),
                 m_player->getMaxUnlockedWeapon());

    // Render Boss Warning overlays or warning text
    if (m_state == GameState::BOSS_WARNING)
    {
        Uint32 ticks = SDL_GetTicks();
        // Red flashing background glow
        if ((ticks / 200) % 2 == 0)
        {
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 30);
            SDL_RenderFillRect(m_renderer, &defaultViewport);

            m_hud.renderText(m_renderer, "WARNING: INTRUDER APPROACHING", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20, COLOR_BOSS, COLOR_BOSS_GLOW, true, true);
        }
    }

    // Screen State overlays
    if (m_state == GameState::START_SCREEN)
    {
        m_hud.renderMessage(m_renderer, "NEON VOID", "PRESS ENTER TO PLAY");
    }
    else if (m_state == GameState::GET_READY)
    {
        m_hud.renderMessage(m_renderer, "STAGE " + std::to_string(m_level), "GET READY...");
    }
    else if (m_state == GameState::PAUSED)
    {
        m_hud.renderMessage(m_renderer, "PAUSED", "PRESS ENTER TO CONTINUE");
    }
    else if (m_state == GameState::GAME_OVER)
    {
        m_hud.renderMessage(m_renderer, "GAME OVER", "FINAL SCORE: " + std::to_string(m_score) + " | ENTER TO RETRY");
    }

    SDL_RenderPresent(m_renderer);
}
