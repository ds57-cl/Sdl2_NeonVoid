#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <SDL_mixer.h>
#include <vector>
#include <cstdint>

enum class SoundID
{
    PLAYER_SHOOT = 0,
    ENEMY_SHOOT,
    EXPLOSION,
    POWERUP,
    HIT,
    BOSS_SPAWN,
    COUNT
};

class AudioSystem
{
public:
    AudioSystem();
    ~AudioSystem();

    bool init();
    void play(SoundID sound);
    void freeSounds();

private:
    bool m_initialized = false;

    std::vector<uint8_t> m_wavBuffers[static_cast<int>(SoundID::COUNT)];

    Mix_Chunk *m_chunks[static_cast<int>(SoundID::COUNT)];

    void generatePlayerShoot();
    void generateEnemyShoot();
    void generateExplosion();
    void generatePowerup();
    void generateHit();
    void generateBossSpawn();
};

#endif // AUDIO_HPP
