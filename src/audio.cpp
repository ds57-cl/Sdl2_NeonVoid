#include "audio.hpp"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper to convert raw PCM 16-bit Mono data at 44100Hz into a valid WAV file buffer
std::vector<uint8_t> convertToWav(const std::vector<int16_t> &pcm)
{
    uint32_t dataSize = pcm.size() * sizeof(int16_t);
    uint32_t fileSize = 36 + dataSize;

    std::vector<uint8_t> wav(44 + dataSize);

    // RIFF header
    wav[0] = 'R';
    wav[1] = 'I';
    wav[2] = 'F';
    wav[3] = 'F';
    *reinterpret_cast<uint32_t *>(&wav[4]) = fileSize;
    wav[8] = 'W';
    wav[9] = 'A';
    wav[10] = 'V';
    wav[11] = 'E';

    // fmt subchunk
    wav[12] = 'f';
    wav[13] = 'm';
    wav[14] = 't';
    wav[15] = ' ';
    *reinterpret_cast<uint32_t *>(&wav[16]) = 16;        // Subchunk1Size
    *reinterpret_cast<uint16_t *>(&wav[20]) = 1;         // AudioFormat (1 = PCM)
    *reinterpret_cast<uint16_t *>(&wav[22]) = 1;         // NumChannels (1 = Mono)
    *reinterpret_cast<uint32_t *>(&wav[24]) = 44100;     // SampleRate
    *reinterpret_cast<uint32_t *>(&wav[28]) = 44100 * 2; // ByteRate (44100 * 1 channel * 2 bytes/sample)
    *reinterpret_cast<uint16_t *>(&wav[32]) = 2;         // BlockAlign (1 channel * 2 bytes/sample)
    *reinterpret_cast<uint16_t *>(&wav[34]) = 16;        // BitsPerSample

    // data subchunk
    wav[36] = 'd';
    wav[37] = 'a';
    wav[38] = 't';
    wav[39] = 'a';
    *reinterpret_cast<uint32_t *>(&wav[40]) = dataSize;

    // Copy raw PCM data into WAV body
    std::memcpy(&wav[44], pcm.data(), dataSize);

    return wav;
}

AudioSystem::AudioSystem()
{
    for (int i = 0; i < static_cast<int>(SoundID::COUNT); ++i)
    {
        m_chunks[i] = nullptr;
    }
}

AudioSystem::~AudioSystem()
{
    freeSounds();
    if (m_initialized)
    {
        Mix_CloseAudio();
    }
}

bool AudioSystem::init()
{
    // Seed the random number generator for explosions
    srand(static_cast<unsigned int>(time(nullptr)));

    // Initialize SDL_mixer
    // We request 44.1kHz stereo/mono, but SDL_mixer may open in whatever the OS hardware demands.
    // By loading WAV headers instead of raw bytes, SDL_mixer will automatically resample the audio to match the device format!
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    m_initialized = true;

    // Allocate channels
    Mix_AllocateChannels(16);

    // Generate synth buffers
    generatePlayerShoot();
    generateEnemyShoot();
    generateExplosion();
    generatePowerup();
    generateHit();
    generateBossSpawn();

    // Create Mix_Chunks from buffers using Mix_QuickLoad_WAV
    for (int i = 0; i < static_cast<int>(SoundID::COUNT); ++i)
    {
        Uint8 *rawBytes = m_wavBuffers[i].data();

        m_chunks[i] = Mix_QuickLoad_WAV(rawBytes);
        if (!m_chunks[i])
        {
            std::cerr << "Failed to load WAV audio for SoundID " << i << "! Error: " << Mix_GetError() << std::endl;
            return false;
        }
    }

    return true;
}

void AudioSystem::play(SoundID sound)
{
    if (!m_initialized)
        return;
    int idx = static_cast<int>(sound);
    if (m_chunks[idx])
    {
        Mix_PlayChannel(-1, m_chunks[idx], 0);
    }
}

void AudioSystem::freeSounds()
{
    for (int i = 0; i < static_cast<int>(SoundID::COUNT); ++i)
    {
        if (m_chunks[i])
        {
            Mix_FreeChunk(m_chunks[i]);
            m_chunks[i] = nullptr;
        }
        m_wavBuffers[i].clear();
    }
}

void AudioSystem::generatePlayerShoot()
{
    int idx = static_cast<int>(SoundID::PLAYER_SHOOT);
    float duration = 0.12f;
    int sampleCount = static_cast<int>(44100 * duration);
    std::vector<int16_t> pcm(sampleCount);

    double phase = 0.0;
    for (int i = 0; i < sampleCount; ++i)
    {
        float t = static_cast<float>(i) / sampleCount;
        float freq = 1200.0f - (1050.0f * t); // Sweeps 1200Hz -> 150Hz
        phase += 2.0 * M_PI * freq / 44100.0;

        float val = static_cast<float>(sin(phase));
        float env = 1.0f - t; // Linear decay

        pcm[i] = static_cast<int16_t>(val * env * 10000.0f);
    }
    m_wavBuffers[idx] = convertToWav(pcm);
}

void AudioSystem::generateEnemyShoot()
{
    int idx = static_cast<int>(SoundID::ENEMY_SHOOT);
    float duration = 0.18f;
    int sampleCount = static_cast<int>(44100 * duration);
    std::vector<int16_t> pcm(sampleCount);

    double phase = 0.0;
    for (int i = 0; i < sampleCount; ++i)
    {
        float t = static_cast<float>(i) / sampleCount;
        float freq = 550.0f - (400.0f * t); // Sweeps 550Hz -> 150Hz
        phase += 2.0 * M_PI * freq / 44100.0;

        // Triangle wave for buzzy retro alien feel
        double normPhase = phase / (2.0 * M_PI);
        float val = 2.0f * static_cast<float>(fabs(2.0f * (normPhase - floor(normPhase + 0.5f)))) - 1.0f;

        float env = 1.0f - t; // Linear decay

        pcm[i] = static_cast<int16_t>(val * env * 5000.0f);
    }
    m_wavBuffers[idx] = convertToWav(pcm);
}

void AudioSystem::generateExplosion()
{
    int idx = static_cast<int>(SoundID::EXPLOSION);
    float duration = 0.45f;
    int sampleCount = static_cast<int>(44100 * duration);
    std::vector<int16_t> pcm(sampleCount);

    float lastVal = 0.0f;
    for (int i = 0; i < sampleCount; ++i)
    {
        float t = static_cast<float>(i) / sampleCount;
        // White noise
        float noise = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;

        // Simple low-pass filter (moving average) to soften the noise and add bass rumble
        float val = 0.82f * lastVal + 0.18f * noise;
        lastVal = val;

        float env = static_cast<float>(exp(-5.5f * t)); // Exponential decay

        pcm[i] = static_cast<int16_t>(val * env * 14000.0f);
    }
    m_wavBuffers[idx] = convertToWav(pcm);
}

void AudioSystem::generatePowerup()
{
    int idx = static_cast<int>(SoundID::POWERUP);
    float duration = 0.35f;
    int sampleCount = static_cast<int>(44100 * duration);
    std::vector<int16_t> pcm(sampleCount);

    double phase = 0.0;
    for (int i = 0; i < sampleCount; ++i)
    {
        float t = static_cast<float>(i) / sampleCount;

        // Ascending steps (arpeggio)
        float freq = 400.0f;
        if (t < 0.2f)
            freq = 350.0f;
        else if (t < 0.4f)
            freq = 523.25f; // C5
        else if (t < 0.6f)
            freq = 659.25f; // E5
        else if (t < 0.8f)
            freq = 783.99f; // G5
        else
            freq = 1046.50f; // C6

        phase += 2.0 * M_PI * freq / 44100.0;

        // Square wave
        float val = (sin(phase) >= 0.0) ? 1.0f : -1.0f;
        float env = 1.0f - t;

        pcm[i] = static_cast<int16_t>(val * env * 5000.0f);
    }
    m_wavBuffers[idx] = convertToWav(pcm);
}

void AudioSystem::generateHit()
{
    int idx = static_cast<int>(SoundID::HIT);
    float duration = 0.15f;
    int sampleCount = static_cast<int>(44100 * duration);
    std::vector<int16_t> pcm(sampleCount);

    double phase = 0.0;
    for (int i = 0; i < sampleCount; ++i)
    {
        float t = static_cast<float>(i) / sampleCount;
        float freq = 180.0f - (140.0f * t); // sweeps 180Hz -> 40Hz
        phase += 2.0 * M_PI * freq / 44100.0;

        // Sawtooth wave
        double normPhase = phase / (2.0 * M_PI);
        float val = 2.0f * static_cast<float>(normPhase - floor(normPhase + 0.5f));
        float env = static_cast<float>(exp(-4.0f * t));

        pcm[i] = static_cast<int16_t>(val * env * 11000.0f);
    }
    m_wavBuffers[idx] = convertToWav(pcm);
}

void AudioSystem::generateBossSpawn()
{
    int idx = static_cast<int>(SoundID::BOSS_SPAWN);
    float duration = 1.2f;
    int sampleCount = static_cast<int>(44100 * duration);
    std::vector<int16_t> pcm(sampleCount);

    double phase = 0.0;
    for (int i = 0; i < sampleCount; ++i)
    {
        float t = static_cast<float>(i) / sampleCount;

        // Alternating alarm between 180Hz and 130Hz
        float freq = (fmod(t, 0.4f) < 0.2f) ? 180.0f : 130.0f;
        phase += 2.0 * M_PI * freq / 44100.0;

        // Triangle wave
        double normPhase = phase / (2.0 * M_PI);
        float val = 2.0f * static_cast<float>(fabs(2.0f * (normPhase - floor(normPhase + 0.5f)))) - 1.0f;

        // Pulsing volume envelope
        float env = 0.7f + 0.3f * static_cast<float>(sin(2.0 * M_PI * 8.0 * t));

        pcm[i] = static_cast<int16_t>(val * env * 9000.0f);
    }
    m_wavBuffers[idx] = convertToWav(pcm);
}
