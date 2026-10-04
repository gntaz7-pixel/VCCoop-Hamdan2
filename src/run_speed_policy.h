// Pure C++11 movement-animation rate controller: no Windows or GTA dependencies.
#pragma once
#include <cmath>
#include <cstdint>

namespace runspeed {

inline int ClampPercent(int p) { return p < 70 ? 70 : p > 100 ? 100 : p; }

// Vice City pedestrian movement states: 1 still, 2 walk, 3 run, 4 sprint.
// Walking-group animation IDs: 0 walk, 1 run, 2 sprint, 3 idle, 4 start.
inline bool ShouldScale(bool onFoot, bool alive, bool cutscene, int moveState,
                        int animId, float blend, float blendDelta)
{
    return onFoot && alive && !cutscene && (moveState == 3 || moveState == 4) &&
           (animId == 1 || animId == 2) && blend >= 0.05f && blendDelta >= -0.01f;
}

// Avoid repeatedly multiplying the same association every frame (90% * 90% * ...).
// An association may be rewritten by the engine; in that case learn its new native speed.
class RateController {
public:
    void BeginFrame() { ++epoch_; if (epoch_ == 0) ++epoch_; }

    float Update(void *ped, void *association, void *hierarchy, int animId,
                 float incomingSpeed, bool slowing, int percent)
    {
        if (!std::isfinite(incomingSpeed) || incomingSpeed <= 0.f) return incomingSpeed;
        const float factor = ClampPercent(percent) / 100.f;
        for (int i = 0; i < CAPACITY; ++i) {
            Entry &e = cache_[i];
            if (e.ped != ped || e.association != association ||
                e.hierarchy != hierarchy || e.animId != animId) continue;
            const float tolerance = 0.0001f * (std::fabs(e.written) > 1.f ? std::fabs(e.written) : 1.f);
            if (std::fabs(incomingSpeed - e.written) > tolerance) e.native = incomingSpeed;
            e.seen = epoch_;
            e.written = slowing ? e.native * factor : e.native;
            return e.written;
        }
        if (!slowing || factor == 1.f) return incomingSpeed;
        for (int i = 0; i < CAPACITY; ++i) if (!cache_[i].ped || cache_[i].seen != epoch_) {
            Entry &e = cache_[i];
            e.ped = ped; e.association = association; e.hierarchy = hierarchy;
            e.animId = animId; e.native = incomingSpeed;
            e.written = incomingSpeed * factor; e.seen = epoch_;
            return e.written;
        }
        // Full cache: fail open rather than slow a random unrelated animation.
        return incomingSpeed;
    }

    void EndFrame() {
        for (int i = 0; i < CAPACITY; ++i) if (cache_[i].seen != epoch_) cache_[i].ped = nullptr;
    }

private:
    enum { CAPACITY = 64 };
    struct Entry {
        void *ped = nullptr;
        void *association = nullptr;
        void *hierarchy = nullptr;
        int animId = -1;
        float native = 0.f;
        float written = 0.f;
        uint32_t seen = 0;
    };
    Entry cache_[CAPACITY];
    uint32_t epoch_ = 0;
};
} // namespace runspeed
