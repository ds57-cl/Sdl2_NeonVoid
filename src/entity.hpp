#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <SDL.h>
#include <vector>
#include "bullet.hpp"

class Entity
{
public:
    Entity(float x, float y, float radius, int health, SDL_Color color, SDL_Color glowColor);
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;
    virtual void render(SDL_Renderer *renderer) = 0;

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getRadius() const { return m_radius; }
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    bool isDead() const { return m_dead; }
    SDL_Color getColor() const { return m_color; }

    void takeDamage(int amount);
    void setPosition(float x, float y)
    {
        m_x = x;
        m_y = y;
    }
    void addVelocity(float vx, float vy)
    {
        m_vx += vx;
        m_vy += vy;
    }
    void setVelocity(float vx, float vy)
    {
        m_vx = vx;
        m_vy = vy;
    }

protected:
    float m_x, m_y;
    float m_vx, m_vy;
    float m_radius;
    int m_health;
    int m_maxHealth;
    bool m_dead;
    SDL_Color m_color;
    SDL_Color m_glowColor;

    // Helper for rendering glowing vector lines
    void drawGlowLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2);
};

enum class EnemyType
{
    DRONE,
    SCOUT,
    BOMBER,
    BOSS
};

class Enemy : public Entity
{
public:
    Enemy(float x, float y, EnemyType type);
    void update(float dt) override;
    void render(SDL_Renderer *renderer) override;

    EnemyType getType() const { return m_type; }
    bool canShoot() const { return m_shootDelay > 0.0f && m_shootCooldown <= 0.0f; }
    void resetShootCooldown();
    std::vector<Bullet> fire();

private:
    EnemyType m_type;
    float m_shootCooldown;
    float m_angle;           // For rotating shapes (e.g. Drones, Bombers)
    float m_oscillationTime; // For weaving patterns
    float m_speed;
    float m_shootDelay;
};

enum class WeaponType
{
    SINGLE,
    DOUBLE,
    TRIPLE,
    PLASMA
};

class Player : public Entity
{
public:
    Player(float x, float y);
    void update(float dt) override;
    void render(SDL_Renderer *renderer) override;

    void handleInput(const Uint8 *keyboardState, float dt);

    int getShield() const { return m_shield; }
    int getMaxShield() const { return m_maxShield; }
    WeaponType getWeaponType() const { return m_weaponType; }
    int getMaxUnlockedWeapon() const { return m_maxUnlockedWeapon; }

    void upgradeWeapon();
    void resetWeapon();
    bool selectWeapon(int index);
    void rechargeShield(int amount);
    void hitShield(int damage);
    void regenerateShield(float dt);
    bool canShoot() const { return m_shootDelay > 0.0f && m_shootCooldown <= 0.0f; }
    void resetShootCooldown();
    std::vector<Bullet> fire();

    float getShootCooldownRatio() const { return m_shootCooldown / m_shootDelay; }
    float getShieldHitEffect() const { return m_shieldHitEffect; }

private:
    int m_shield;
    int m_maxShield;
    float m_shieldHitEffect; // Visual intensity multiplier for shield hits
    float m_shieldRegenTimer;
    float m_shootCooldown;
    float m_shootDelay;
    WeaponType m_weaponType;
    int m_maxUnlockedWeapon;
};

#endif // ENTITY_HPP
