// Pure, platform-independent state machine for cooperative story-mission deaths.
// No Win32/game dependencies: exercised by tests/guest_mission_death_test.cpp.
#pragma once
#include <stdint.h>

namespace guest_mission_death {
struct GuestState {
    bool connected, inGame, fresh, cutscene, dead;
};

class Tracker {
public:
    Tracker() { End(); }
    void End()
    {
        active_ = pending_ = false;
        for (int i = 0; i < 3; ++i) { observed_[i] = false; wasDead_[i] = false; }
    }
    void Begin(const GuestState *guests, int count)
    {
        End();
        active_ = true;
        for (int i = 0; i < count && i < 3; ++i) {
            if (!Eligible(guests[i])) continue;
            observed_[i] = true;
            wasDead_[i] = guests[i].dead; // already dead on mission start is not a new death
        }
    }
    // Return guest slot (0..2) for a fresh alive -> dead transition, -1 otherwise.
    int Poll(const GuestState *guests, int count)
    {
        if (!active_) return -1;
        int triggered = -1;
        for (int i = 0; i < count && i < 3; ++i) {
            if (!Eligible(guests[i])) {
                observed_[i] = false; // reconnect / loading: establish a new baseline
                continue;
            }
            if (observed_[i] && !wasDead_[i] && guests[i].dead && !pending_) {
                pending_ = true;
                triggered = i;
            }
            observed_[i] = true;
            wasDead_[i] = guests[i].dead;
        }
        return triggered;
    }
    bool Pending() const { return active_ && pending_; }
    bool Active() const { return active_; }
private:
    static bool Eligible(const GuestState &s)
    {
        return s.connected && s.inGame && s.fresh && !s.cutscene;
    }
    bool active_, pending_;
    bool observed_[3], wasDead_[3];
};

// These Vice City opcodes ask whether the LOCAL player/actor is dead.
// Script's NOT flag is processed afterwards by UpdateCompareFlag.
inline bool MatchesHostDeathCheck(uint16_t opcode, bool missionScript, bool isHostSubject)
{
    return missionScript && isHostSubject && (opcode == 0x0117 || opcode == 0x0118);
}
} // namespace guest_mission_death
