#ifndef BULLET_HPP
#define BULLET_HPP

#include <SDL.h>

class Bullet {
public:
    Bullet(float x, float y, float vx, float vy, bool isPlayerBullet, float radius, int damage, SDL_Color color);
    ~Bullet() = default;

    void update(float dt);
    void render(SDL_Renderer* renderer);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getRadius() const { return m_radius; }
    bool isPlayerBullet() const { return m_isPlayerBullet; }
    int getDamage() const { return m_damage; }
    bool isDead() const { return m_dead; }
    void kill() { m_dead = true; }

private:
    float m_x, m_y;
    float m_vx, m_vy;
    bool m_isPlayerBullet;
    float m_radius;
    int m_damage;
    SDL_Color m_color;
    bool m_dead;
};

#endif // BULLET_HPP
