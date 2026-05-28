#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <SDL.h>
#include <vector>

struct Star {
    float x, y;
    float speed;
    float size;
    Uint8 brightness;
};

struct Particle {
    float x, y;
    float vx, vy;
    float life;      // Current lifetime in seconds
    float maxLife;   // Initial lifetime in seconds
    float size;
    SDL_Color color;
    bool shrink;     // Whether it shrinks over time
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem() = default;

    void initStars(int count);
    void update(float dt);
    void renderStars(SDL_Renderer* renderer);
    void renderParticles(SDL_Renderer* renderer);

    void addParticle(float x, float y, float vx, float vy, float maxLife, float size, SDL_Color color, bool shrink = true);
    void spawnExplosion(float x, float y, SDL_Color color, int count = 20);
    void spawnThrusterTrail(float x, float y, float vx, float vy, SDL_Color color);
    void spawnSparks(float x, float y, SDL_Color color, int count = 6);
    void clear();

private:
    std::vector<Star> m_stars;
    std::vector<Particle> m_particles;
};

#endif // PARTICLE_HPP
