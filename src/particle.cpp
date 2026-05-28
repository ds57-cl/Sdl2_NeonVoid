#include "particle.hpp"
#include "config.hpp"
#include <cstdlib>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ParticleSystem::ParticleSystem() {
    m_stars.reserve(150);
    m_particles.reserve(500);
}

void ParticleSystem::initStars(int count) {
    m_stars.clear();
    for (int i = 0; i < count; ++i) {
        Star star;
        star.x = static_cast<float>(rand() % SCREEN_WIDTH);
        star.y = static_cast<float>(rand() % SCREEN_HEIGHT);
        star.speed = static_cast<float>(30 + rand() % 80); // Speed 30 to 110
        star.size = 1.0f + (star.speed - 30.0f) / 80.0f * 1.5f; // Size 1.0 to 2.5
        // Faster stars are brighter (parallax effect)
        star.brightness = static_cast<Uint8>(100 + (star.speed - 30.0f) / 80.0f * 155.0f);
        m_stars.push_back(star);
    }
}

void ParticleSystem::update(float dt) {
    // 1. Update Stars
    for (auto& star : m_stars) {
        star.y += star.speed * dt;
        if (star.y > SCREEN_HEIGHT) {
            star.y = 0;
            star.x = static_cast<float>(rand() % SCREEN_WIDTH);
            star.speed = static_cast<float>(30 + rand() % 80);
            star.size = 1.0f + (star.speed - 30.0f) / 80.0f * 1.5f;
            star.brightness = static_cast<Uint8>(100 + (star.speed - 30.0f) / 80.0f * 155.0f);
        }
    }

    // 2. Update Particles
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        
        // Apply air resistance/drag to make explosions drift to a stop
        it->vx *= static_cast<float>(pow(0.92, dt * 60.0));
        it->vy *= static_cast<float>(pow(0.92, dt * 60.0));
        
        it->life -= dt;
        if (it->life <= 0) {
            it = m_particles.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::renderStars(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (const auto& star : m_stars) {
        SDL_SetRenderDrawColor(renderer, star.brightness, star.brightness, star.brightness, star.brightness);
        
        if (star.size <= 1.5f) {
            SDL_RenderDrawPoint(renderer, static_cast<int>(star.x), static_cast<int>(star.y));
        } else {
            SDL_Rect r = {
                static_cast<int>(star.x), 
                static_cast<int>(star.y), 
                static_cast<int>(star.size), 
                static_cast<int>(star.size)
            };
            SDL_RenderFillRect(renderer, &r);
        }
    }
}

void ParticleSystem::renderParticles(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const auto& p : m_particles) {
        float progress = p.life / p.maxLife; // 1.0 down to 0.0
        float currentSize = p.shrink ? (p.size * progress) : p.size;
        
        if (currentSize < 0.5f) continue;
        
        Uint8 alpha = static_cast<Uint8>(p.color.a * progress);
        
        // 1. Render Neon Glow Pass (draw larger translucent square)
        float glowSize = currentSize * 2.5f;
        SDL_Rect glowRect = {
            static_cast<int>(p.x - glowSize / 2.0f),
            static_cast<int>(p.y - glowSize / 2.0f),
            static_cast<int>(glowSize),
            static_cast<int>(glowSize)
        };
        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, static_cast<Uint8>(alpha * 0.25f));
        SDL_RenderFillRect(renderer, &glowRect);

        // 2. Render Core Pass
        SDL_Rect coreRect = {
            static_cast<int>(p.x - currentSize / 2.0f),
            static_cast<int>(p.y - currentSize / 2.0f),
            static_cast<int>(currentSize),
            static_cast<int>(currentSize)
        };
        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, alpha);
        SDL_RenderFillRect(renderer, &coreRect);
    }
}

void ParticleSystem::addParticle(float x, float y, float vx, float vy, float maxLife, float size, SDL_Color color, bool shrink) {
    Particle p;
    p.x = x;
    p.y = y;
    p.vx = vx;
    p.vy = vy;
    p.life = maxLife;
    p.maxLife = maxLife;
    p.size = size;
    p.color = color;
    p.shrink = shrink;
    m_particles.push_back(p);
}

void ParticleSystem::spawnExplosion(float x, float y, SDL_Color color, int count) {
    for (int i = 0; i < count; ++i) {
        float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * M_PI;
        float speed = 80.0f + static_cast<float>(rand() % 180); // Speed 80 to 260
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;
        
        float maxLife = 0.4f + (static_cast<float>(rand() % 40) / 100.0f); // 0.4s to 0.8s
        float size = 3.0f + static_cast<float>(rand() % 5); // 3 to 7 pixels
        
        // Mix slightly with white to make hot core particles
        SDL_Color pColor = color;
        if (rand() % 3 == 0) {
            pColor.r = static_cast<Uint8>((color.r + 255) / 2);
            pColor.g = static_cast<Uint8>((color.g + 255) / 2);
            pColor.b = static_cast<Uint8>((color.b + 255) / 2);
        }

        addParticle(x, y, vx, vy, maxLife, size, pColor, true);
    }
}

void ParticleSystem::spawnThrusterTrail(float x, float y, float vx, float vy, SDL_Color color) {
    // Spawn small particles moving backwards relative to ship direction
    float spreadX = -10.0f + static_cast<float>(rand() % 20);
    float spreadY = -10.0f + static_cast<float>(rand() % 20);
    
    // Add jitter
    float speedVx = vx + spreadX;
    float speedVy = vy + spreadY;
    
    float maxLife = 0.15f + (static_cast<float>(rand() % 15) / 100.0f); // 0.15s to 0.3s
    float size = 2.0f + static_cast<float>(rand() % 3);
    
    addParticle(x, y, speedVx, speedVy, maxLife, size, color, true);
}

void ParticleSystem::spawnSparks(float x, float y, SDL_Color color, int count) {
    for (int i = 0; i < count; ++i) {
        float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * M_PI;
        float speed = 120.0f + static_cast<float>(rand() % 100);
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;
        
        float maxLife = 0.15f + (static_cast<float>(rand() % 15) / 100.0f);
        float size = 1.5f + static_cast<float>(rand() % 3);
        
        addParticle(x, y, vx, vy, maxLife, size, color, true);
    }
}

void ParticleSystem::clear() {
    m_particles.clear();
}
