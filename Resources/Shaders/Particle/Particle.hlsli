static const uint32_t kMaxParticles = 1024;

struct Particle {
    float32_t3 scale;
    float32_t3 translate;
    float lifeTime;
    float32_t3 velocity;
    float currentTime;
    float32_t4 color;
};
