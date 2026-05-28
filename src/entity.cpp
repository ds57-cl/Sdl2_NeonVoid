#include "entity.hpp"
#include "config.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Entity::Entity(float x, float y, float radius, int health, SDL_Color color, SDL_Color glowColor)
    : m_x(x), m_y(y), m_vx(0.0f), m_vy(0.0f), m_radius(radius), m_health(health), m_maxHealth(health), m_dead(false), m_color(color), m_glowColor(glowColor) {}

void Entity::takeDamage(int amount)
{
    if (m_dead)
        return;
    m_health -= amount;
    if (m_health <= 0)
    {
        m_health = 0;
        m_dead = true;
    }
}

void Entity::drawGlowLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2)
{
    // 1. Draw Outer Glow (semi-transparent, thick offsets)
    SDL_SetRenderDrawColor(renderer, m_glowColor.r, m_glowColor.g, m_glowColor.b, m_glowColor.a);
    SDL_RenderDrawLine(renderer, static_cast<int>(x1 - 1), static_cast<int>(y1), static_cast<int>(x2 - 1), static_cast<int>(y2));
    SDL_RenderDrawLine(renderer, static_cast<int>(x1 + 1), static_cast<int>(y1), static_cast<int>(x2 + 1), static_cast<int>(y2));
    SDL_RenderDrawLine(renderer, static_cast<int>(x1), static_cast<int>(y1 - 1), static_cast<int>(x2), static_cast<int>(y2 - 1));
    SDL_RenderDrawLine(renderer, static_cast<int>(x1), static_cast<int>(y1 + 1), static_cast<int>(x2), static_cast<int>(y2 + 1));

    // 2. Draw Bright Core
    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
    SDL_RenderDrawLine(renderer, static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2));
}

// ==========================================
// ENEMY
// ==========================================

Enemy::Enemy(float x, float y, EnemyType type)
    : Entity(x, y, 16.0f, 20, COLOR_ENEMY_DRONE, COLOR_ENEMY_DRONE_GLOW), m_type(type), m_shootCooldown(0.0f), m_angle(0.0f), m_oscillationTime(0.0f)
{

    m_oscillationTime = static_cast<float>(rand() % 100); // Random starting phase

    switch (type)
    {
    case EnemyType::DRONE:
        m_radius = 15.0f;
        m_health = 15;
        m_maxHealth = 15;
        m_speed = 120.0f;
        m_shootDelay = -1.0f; // Doesn't shoot
        m_color = COLOR_ENEMY_DRONE;
        m_glowColor = COLOR_ENEMY_DRONE_GLOW;
        break;
    case EnemyType::SCOUT:
        m_radius = 14.0f;
        m_health = 25;
        m_maxHealth = 25;
        m_speed = 180.0f;
        m_shootDelay = 1.6f + (rand() % 10) * 0.1f;
        m_shootCooldown = m_shootDelay * 0.5f; // Ready sooner
        m_color = COLOR_ENEMY_SCOUT;
        m_glowColor = COLOR_ENEMY_SCOUT_GLOW;
        break;
    case EnemyType::BOMBER:
        m_radius = 24.0f;
        m_health = 80;
        m_maxHealth = 80;
        m_speed = 50.0f;
        m_shootDelay = 2.4f + (rand() % 10) * 0.1f;
        m_shootCooldown = m_shootDelay * 0.8f;
        m_color = COLOR_ENEMY_BOMBER;
        m_glowColor = COLOR_ENEMY_BOMBER_GLOW;
        break;
    case EnemyType::BOSS:
        m_radius = 65.0f;
        m_health = 800;
        m_maxHealth = 800;
        m_speed = 60.0f;
        m_shootDelay = 1.0f;
        m_shootCooldown = 1.5f;
        m_color = COLOR_BOSS;
        m_glowColor = COLOR_BOSS_GLOW;
        break;
    }
}

void Enemy::update(float dt)
{
    m_oscillationTime += dt;
    m_angle += 1.5f * dt; // Rotate shape slowly

    if (m_shootCooldown > 0.0f)
    {
        m_shootCooldown -= dt;
    }

    switch (m_type)
    {
    case EnemyType::DRONE:
        m_vy = m_speed;
        m_vx = sin(m_oscillationTime * 3.0f) * 100.0f;
        break;
    case EnemyType::SCOUT:
        m_vy = m_speed * 0.7f;
        m_vx = cos(m_oscillationTime * 2.2f) * 160.0f;
        break;
    case EnemyType::BOMBER:
        m_vy = m_speed;
        m_vx = sin(m_oscillationTime * 0.8f) * 35.0f;
        break;
    case EnemyType::BOSS:
        // Move into screen from top then hover/oscillate
        if (m_y < 140.0f)
        {
            m_vy = m_speed;
            m_vx = 0.0f;
        }
        else
        {
            m_vy = 0.0f;
            // Wobble horizontally
            m_vx = sin(m_oscillationTime * 0.9f) * 120.0f;
        }
        break;
    }

    m_x += m_vx * dt;
    m_y += m_vy * dt;
}

void Enemy::render(SDL_Renderer *renderer)
{
    if (m_dead)
        return;

    float r = m_radius;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (m_type == EnemyType::DRONE)
    {
        // Draw green diamond rotating
        float cosA = cos(m_angle);
        float sinA = sin(m_angle);

        float x_pts[4] = {0, r, 0, -r};
        float y_pts[4] = {-r, 0, r, 0};

        for (int i = 0; i < 4; ++i)
        {
            float rx = x_pts[i] * cosA - y_pts[i] * sinA;
            float ry = x_pts[i] * sinA + y_pts[i] * cosA;
            x_pts[i] = m_x + rx;
            y_pts[i] = m_y + ry;
        }

        for (int i = 0; i < 4; ++i)
        {
            drawGlowLine(renderer, x_pts[i], y_pts[i], x_pts[(i + 1) % 4], y_pts[(i + 1) % 4]);
        }
    }
    else if (m_type == EnemyType::SCOUT)
    {
        // Draw V-shaped yellow scout pointing down
        drawGlowLine(renderer, m_x, m_y + r, m_x - r, m_y - r);
        drawGlowLine(renderer, m_x - r, m_y - r, m_x, m_y - r * 0.2f);
        drawGlowLine(renderer, m_x, m_y - r * 0.2f, m_x + r, m_y - r);
        drawGlowLine(renderer, m_x + r, m_y - r, m_x, m_y + r);

        // Cockpit lines
        drawGlowLine(renderer, m_x, m_y + r * 0.3f, m_x - r * 0.3f, m_y - r * 0.2f);
        drawGlowLine(renderer, m_x, m_y + r * 0.3f, m_x + r * 0.3f, m_y - r * 0.2f);
    }
    else if (m_type == EnemyType::BOMBER)
    {
        // Draw heavy orange octagon
        float x_pts[8];
        float y_pts[8];
        for (int i = 0; i < 8; ++i)
        {
            float angle = m_angle + i * M_PI / 4.0;
            x_pts[i] = m_x + cos(angle) * r;
            y_pts[i] = m_y + sin(angle) * r;
        }
        for (int i = 0; i < 8; ++i)
        {
            drawGlowLine(renderer, x_pts[i], y_pts[i], x_pts[(i + 1) % 8], y_pts[(i + 1) % 8]);
        }
        // Inner glowing core
        drawGlowLine(renderer, m_x - r * 0.5f, m_y, m_x + r * 0.5f, m_y);
        drawGlowLine(renderer, m_x, m_y - r * 0.5f, m_x, m_y + r * 0.5f);
    }
    else if (m_type == EnemyType::BOSS)
    {
        // Draw massive magenta boss ship
        // Central core
        float core_r = r * 0.4f;
        float x_pts[6];
        float y_pts[6];
        for (int i = 0; i < 6; ++i)
        {
            float angle = m_angle * 0.5f + i * M_PI / 3.0;
            x_pts[i] = m_x + cos(angle) * core_r;
            y_pts[i] = m_y + sin(angle) * core_r;
        }
        for (int i = 0; i < 6; ++i)
        {
            drawGlowLine(renderer, x_pts[i], y_pts[i], x_pts[(i + 1) % 6], y_pts[(i + 1) % 6]);
        }

        // Left wing pod
        drawGlowLine(renderer, m_x, m_y, m_x - r * 0.7f, m_y + r * 0.2f);
        drawGlowLine(renderer, m_x - r * 0.7f, m_y + r * 0.2f, m_x - r, m_y - r * 0.1f);
        drawGlowLine(renderer, m_x - r, m_y - r * 0.1f, m_x - r * 0.5f, m_y - r * 0.3f);
        drawGlowLine(renderer, m_x - r * 0.5f, m_y - r * 0.3f, m_x, m_y);

        // Right wing pod
        drawGlowLine(renderer, m_x, m_y, m_x + r * 0.7f, m_y + r * 0.2f);
        drawGlowLine(renderer, m_x + r * 0.7f, m_y + r * 0.2f, m_x + r, m_y - r * 0.1f);
        drawGlowLine(renderer, m_x + r, m_y - r * 0.1f, m_x + r * 0.5f, m_y - r * 0.3f);
        drawGlowLine(renderer, m_x + r * 0.5f, m_y - r * 0.3f, m_x, m_y);

        // Front cannon blades
        drawGlowLine(renderer, m_x - r * 0.2f, m_y + r * 0.2f, m_x - r * 0.15f, m_y + r * 0.5f);
        drawGlowLine(renderer, m_x + r * 0.2f, m_y + r * 0.2f, m_x + r * 0.15f, m_y + r * 0.5f);
        drawGlowLine(renderer, m_x - r * 0.15f, m_y + r * 0.5f, m_x, m_y + r * 0.3f);
        drawGlowLine(renderer, m_x + r * 0.15f, m_y + r * 0.5f, m_x, m_y + r * 0.3f);
    }
}

void Enemy::resetShootCooldown()
{
    m_shootCooldown = m_shootDelay;
}

std::vector<Bullet> Enemy::fire()
{
    std::vector<Bullet> bullets;
    if (m_type == EnemyType::SCOUT)
    {
        bullets.push_back(Bullet(m_x, m_y + m_radius, 0.0f, BULLET_SPEED_ENEMY * 1.1f, false, 3.0f, 10, COLOR_BULLET_ENEMY));
    }
    else if (m_type == EnemyType::BOMBER)
    {
        bullets.push_back(Bullet(m_x, m_y + m_radius, 0.0f, BULLET_SPEED_ENEMY * 0.75f, false, 5.0f, 25, COLOR_ENEMY_BOMBER));
    }
    else if (m_type == EnemyType::BOSS)
    {
        // 3-way fan bullet pattern
        bullets.push_back(Bullet(m_x, m_y + 40.0f, 0.0f, BULLET_SPEED_ENEMY, false, 4.0f, 15, COLOR_BULLET_ENEMY));
        bullets.push_back(Bullet(m_x - 30.0f, m_y + 30.0f, -70.0f, BULLET_SPEED_ENEMY * 0.9f, false, 4.0f, 15, COLOR_BULLET_ENEMY));
        bullets.push_back(Bullet(m_x + 30.0f, m_y + 30.0f, 70.0f, BULLET_SPEED_ENEMY * 0.9f, false, 4.0f, 15, COLOR_BULLET_ENEMY));

        // Add rare diagonal bullets depending on time
        if (fmod(m_oscillationTime, 4.0f) < 1.0f)
        {
            bullets.push_back(Bullet(m_x - 50.0f, m_y + 10.0f, -140.0f, BULLET_SPEED_ENEMY * 0.8f, false, 3.5f, 12, COLOR_BULLET_ENEMY));
            bullets.push_back(Bullet(m_x + 50.0f, m_y + 10.0f, 140.0f, BULLET_SPEED_ENEMY * 0.8f, false, 3.5f, 12, COLOR_BULLET_ENEMY));
        }
    }
    return bullets;
}

// ==========================================
// PLAYER
// ==========================================

Player::Player(float x, float y)
    : Entity(x, y, 16.0f, PLAYER_MAX_HEALTH, COLOR_PLAYER, COLOR_PLAYER_GLOW),
      m_shield(PLAYER_MAX_SHIELD), m_maxShield(PLAYER_MAX_SHIELD), m_shieldHitEffect(0.0f), m_shieldRegenTimer(0.0f), m_shootCooldown(0.0f), m_shootDelay(0.18f), m_weaponType(WeaponType::SINGLE), m_maxUnlockedWeapon(0) {}

void Player::update(float dt)
{
    // Decay shield hit flash effect
    if (m_shieldHitEffect > 0.0f)
    {
        m_shieldHitEffect -= 4.0f * dt;
        if (m_shieldHitEffect < 0.0f)
            m_shieldHitEffect = 0.0f;
    }

    // Cooldown management
    if (m_shootCooldown > 0.0f)
    {
        m_shootCooldown -= dt;
    }

    // Apply friction to slow player down smoothly
    m_vx *= PLAYER_FRICTION;
    m_vy *= PLAYER_FRICTION;

    m_x += m_vx * dt;
    m_y += m_vy * dt;

    // Clamp coordinates to screen space
    float r = m_radius;
    if (m_x < r)
    {
        m_x = r;
        m_vx = 0.0f;
    }
    if (m_x > SCREEN_WIDTH - r)
    {
        m_x = SCREEN_WIDTH - r;
        m_vx = 0.0f;
    }
    if (m_y < r)
    {
        m_y = r;
        m_vy = 0.0f;
    }
    if (m_y > SCREEN_HEIGHT - r)
    {
        m_y = SCREEN_HEIGHT - r;
        m_vy = 0.0f;
    }
}

void Player::handleInput(const Uint8 *keyboardState, float dt)
{
    float accel = PLAYER_SPEED * 8.0f; // Rapid acceleration

    if (keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A])
    {
        m_vx -= accel * dt;
    }
    if (keyboardState[SDL_SCANCODE_RIGHT] || keyboardState[SDL_SCANCODE_D])
    {
        m_vx += accel * dt;
    }
    if (keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W])
    {
        m_vy -= accel * dt;
    }
    if (keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S])
    {
        m_vy += accel * dt;
    }

    // Cap player max velocity to avoid flying too fast
    float currentSpeed = sqrt(m_vx * m_vx + m_vy * m_vy);
    if (currentSpeed > PLAYER_SPEED)
    {
        m_vx = (m_vx / currentSpeed) * PLAYER_SPEED;
        m_vy = (m_vy / currentSpeed) * PLAYER_SPEED;
    }
}

void Player::render(SDL_Renderer *renderer)
{
    if (m_dead)
        return;

    float r = m_radius;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // 1. Draw Player ship lines
    // Nose to left wing tip
    drawGlowLine(renderer, m_x, m_y - r, m_x - r, m_y + r);
    // Left wing tip to indent
    drawGlowLine(renderer, m_x - r, m_y + r, m_x, m_y + r * 0.4f);
    // Indent to right wing tip
    drawGlowLine(renderer, m_x, m_y + r * 0.4f, m_x + r, m_y + r);
    // Right wing tip to nose
    drawGlowLine(renderer, m_x + r, m_y + r, m_x, m_y - r);

    // Draw detailed inner cockpit diamond
    drawGlowLine(renderer, m_x, m_y - r * 0.3f, m_x - r * 0.25f, m_y + r * 0.2f);
    drawGlowLine(renderer, m_x - r * 0.25f, m_y + r * 0.2f, m_x, m_y + r * 0.4f);
    drawGlowLine(renderer, m_x, m_y + r * 0.4f, m_x + r * 0.25f, m_y + r * 0.2f);
    drawGlowLine(renderer, m_x + r * 0.25f, m_y + r * 0.2f, m_x, m_y - r * 0.3f);

    // 2. Draw Shield Octagon (if shields active)
    if (m_shield > 0 || m_shieldHitEffect > 0.0f)
    {
        float shield_r = r * 1.5f;
        float shieldIntensity = (static_cast<float>(m_shield) / m_maxShield) * 0.35f + m_shieldHitEffect * 0.65f;
        shieldIntensity = std::clamp(shieldIntensity, 0.0f, 1.0f);

        SDL_Color shieldColor = COLOR_SHIELD;
        SDL_Color shieldGlowColor = COLOR_SHIELD_GLOW;
        shieldGlowColor.a = static_cast<Uint8>(shieldIntensity * 120.0f);

        // Compute 8 points of octagon
        float x_pts[8];
        float y_pts[8];
        for (int i = 0; i < 8; ++i)
        {
            float angle = i * M_PI / 4.0f;
            x_pts[i] = m_x + cos(angle) * shield_r;
            y_pts[i] = m_y + sin(angle) * shield_r;
        }

        // Draw glowing outline
        SDL_SetRenderDrawColor(renderer, shieldColor.r, shieldColor.g, shieldColor.b, static_cast<Uint8>(shieldIntensity * 255.0f));
        for (int i = 0; i < 8; ++i)
        {
            // Draw duplicate lines offset to represent glow
            float x1 = x_pts[i], y1 = y_pts[i];
            float x2 = x_pts[(i + 1) % 8], y2 = y_pts[(i + 1) % 8];

            SDL_SetRenderDrawColor(renderer, shieldColor.r, shieldColor.g, shieldColor.b, static_cast<Uint8>(shieldIntensity * 60.0f));
            SDL_RenderDrawLine(renderer, x1 - 1, y1, x2 - 1, y2);
            SDL_RenderDrawLine(renderer, x1 + 1, y1, x2 + 1, y2);
            SDL_RenderDrawLine(renderer, x1, y1 - 1, x2, y2 - 1);
            SDL_RenderDrawLine(renderer, x1, y1 + 1, x2, y2 + 1);

            SDL_SetRenderDrawColor(renderer, shieldColor.r, shieldColor.g, shieldColor.b, static_cast<Uint8>(shieldIntensity * 255.0f));
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}

void Player::upgradeWeapon()
{
    if (m_maxUnlockedWeapon < 3)
    {
        m_maxUnlockedWeapon++;
        selectWeapon(m_maxUnlockedWeapon);
    }
}

void Player::resetWeapon()
{
    m_maxUnlockedWeapon = 0;
    selectWeapon(0);
}

bool Player::selectWeapon(int index)
{
    if (index < 0 || index > m_maxUnlockedWeapon)
    {
        return false;
    }

    switch (index)
    {
    case 0:
        m_weaponType = WeaponType::SINGLE;
        m_shootDelay = 0.18f;
        break;
    case 1:
        m_weaponType = WeaponType::DOUBLE;
        m_shootDelay = 0.16f;
        break;
    case 2:
        m_weaponType = WeaponType::TRIPLE;
        m_shootDelay = 0.18f;
        break;
    case 3:
        m_weaponType = WeaponType::PLASMA;
        m_shootDelay = 0.08f;
        break;
    }

    // Clamp current cooldown to new shoot delay to avoid exploit or lag
    if (m_shootCooldown > m_shootDelay)
    {
        m_shootCooldown = m_shootDelay;
    }

    return true;
}

void Player::rechargeShield(int amount)
{
    m_shield = std::min(m_maxShield, m_shield + amount);
}

void Player::hitShield(int damage)
{
    m_shieldHitEffect = 1.0f;                   // Max visual flash
    m_shieldRegenTimer = SHIELD_REGEN_COOLDOWN; // Reset regeneration delay

    if (m_shield > 0)
    {
        m_shield -= damage;
        if (m_shield < 0)
        {
            // Leak leftover damage to health
            int leak = -m_shield;
            m_shield = 0;
            takeDamage(leak);
        }
    }
    else
    {
        takeDamage(damage);
    }
}

void Player::regenerateShield(float dt)
{
    if (m_shieldRegenTimer > 0.0f)
    {
        m_shieldRegenTimer -= dt;
    }
    else if (m_shield < m_maxShield)
    {
        m_shield = std::min(static_cast<float>(m_maxShield), m_shield + SHIELD_REGEN_RATE * dt);
    }
}

void Player::resetShootCooldown()
{
    m_shootCooldown = m_shootDelay;
}

std::vector<Bullet> Player::fire()
{
    std::vector<Bullet> bullets;
    float r = m_radius;

    if (m_weaponType == WeaponType::SINGLE)
    {
        bullets.push_back(Bullet(m_x, m_y - r, 0.0f, -BULLET_SPEED_PLAYER, true, 3.0f, 20, COLOR_BULLET_PLAYER));
    }
    else if (m_weaponType == WeaponType::DOUBLE)
    {
        bullets.push_back(Bullet(m_x - r * 0.7f, m_y - r * 0.1f, 0.0f, -BULLET_SPEED_PLAYER, true, 3.0f, 20, COLOR_BULLET_PLAYER));
        bullets.push_back(Bullet(m_x + r * 0.7f, m_y - r * 0.1f, 0.0f, -BULLET_SPEED_PLAYER, true, 3.0f, 20, COLOR_BULLET_PLAYER));
    }
    else if (m_weaponType == WeaponType::TRIPLE)
    {
        bullets.push_back(Bullet(m_x, m_y - r, 0.0f, -BULLET_SPEED_PLAYER, true, 3.0f, 20, COLOR_BULLET_PLAYER));
        bullets.push_back(Bullet(m_x - r * 0.7f, m_y - r * 0.1f, -140.0f, -BULLET_SPEED_PLAYER * 0.95f, true, 3.0f, 20, COLOR_BULLET_PLAYER));
        bullets.push_back(Bullet(m_x + r * 0.7f, m_y - r * 0.1f, 140.0f, -BULLET_SPEED_PLAYER * 0.95f, true, 3.0f, 20, COLOR_BULLET_PLAYER));
    }
    else if (m_weaponType == WeaponType::PLASMA)
    {
        // Bullet spread, fast but lower base damage per shot
        float spread = -50.0f + static_cast<float>(rand() % 100);
        bullets.push_back(Bullet(m_x, m_y - r, spread, -BULLET_SPEED_PLAYER * 1.15f, true, 2.5f, 12, COLOR_BULLET_PLAYER));
    }
    return bullets;
}
