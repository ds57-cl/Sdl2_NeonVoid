#include "bullet.hpp"
#include "config.hpp"
#include <cmath>

Bullet::Bullet(float x, float y, float vx, float vy, bool isPlayerBullet, float radius, int damage, SDL_Color color)
    : m_x(x), m_y(y), m_vx(vx), m_vy(vy), m_isPlayerBullet(isPlayerBullet), m_radius(radius), m_damage(damage), m_color(color), m_dead(false) {}

void Bullet::update(float dt)
{
    m_x += m_vx * dt;
    m_y += m_vy * dt;

    if (m_x < -50.0f || m_x > SCREEN_WIDTH + 50.0f || m_y < -50.0f || m_y > SCREEN_HEIGHT + 50.0f)
    {
        m_dead = true;
    }
}

void Bullet::render(SDL_Renderer *renderer)
{
    if (m_dead)
        return;

    // Calculate motion blur tail length based on velocity
    float vx2 = m_vx * m_vx;
    float vy2 = m_vy * m_vy;
    float speed = sqrt(vx2 + vy2);

    if (speed < 1.0f)
        return;

    float tailLength = 14.0f;
    float dirX = m_vx / speed;
    float dirY = m_vy / speed;

    float x1 = m_x;
    float y1 = m_y;
    float x2 = m_x - dirX * tailLength;
    float y2 = m_y - dirY * tailLength;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // 1. Draw Outer Glow (thick and transparent)
    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, 60);
    // Draw offset lines to simulate thickness
    for (int offset = -2; offset <= 2; ++offset)
    {
        // Draw slightly offset parallel lines
        float ox = -dirY * offset;
        float oy = dirX * offset;
        SDL_RenderDrawLine(renderer,
                           static_cast<int>(x1 + ox), static_cast<int>(y1 + oy),
                           static_cast<int>(x2 + ox), static_cast<int>(y2 + oy));
    }

    // 2. Draw Inner Glow (thin and bright)
    SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, 160);
    for (int offset = -1; offset <= 1; ++offset)
    {
        float ox = -dirY * offset;
        float oy = dirX * offset;
        SDL_RenderDrawLine(renderer,
                           static_cast<int>(x1 + ox), static_cast<int>(y1 + oy),
                           static_cast<int>(x2 + ox), static_cast<int>(y2 + oy));
    }

    // 3. Draw Core Line (solid color)
    SDL_Color coreColor = m_color;
    if (m_isPlayerBullet)
    {
        // Bright white/cyan core
        coreColor = {230, 255, 255, 255};
    }
    else
    {
        // Bright white/red core
        coreColor = {255, 230, 230, 255};
    }
    SDL_SetRenderDrawColor(renderer, coreColor.r, coreColor.g, coreColor.b, 255);
    SDL_RenderDrawLine(renderer,
                       static_cast<int>(x1), static_cast<int>(y1),
                       static_cast<int>(x2), static_cast<int>(y2));
}
