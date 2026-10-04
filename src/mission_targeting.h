// Pure target-selection logic for host-owned mission NPCs.
// No Windows or GTA dependencies: tests/mission_targeting_test.cpp runs natively.
#pragma once

namespace mission_targeting {

struct Guest {
    bool connected;
    bool inGame;
    bool alive;
    bool onFoot;
    bool notInCutscene;
    bool sameArea;
    bool fresh;
    float distance2;   // squared XY distance in game metres
    float heightDiff;  // absolute Z difference in metres
};

inline bool Eligible(const Guest &c, float maxDistance)
{
    return c.connected && c.inGame && c.alive && c.onFoot && c.notInCutscene &&
           c.sameArea && c.fresh && c.heightDiff <= 7.0f &&
           c.distance2 >= 0.0f && c.distance2 <= maxDistance * maxDistance;
}

// Returns 0-based guest index or -1 for host. A guest currently targeted is
// retained for up to 58 m, and only replaced when another guest is at least
// ~30 % closer. This prevents 2 nearby guests from causing objective thrashing.
inline int ChooseGuest(const Guest *guests, int count, int currentIndex)
{
    const float acquire = 48.0f;
    const float retain = 58.0f;
    int best = -1;
    float bestD2 = acquire * acquire;
    for (int i = 0; i < count; ++i) {
        if (!Eligible(guests[i], acquire)) continue;
        if (best < 0 || guests[i].distance2 < bestD2) {
            best = i;
            bestD2 = guests[i].distance2;
        }
    }
    if (currentIndex >= 0 && currentIndex < count && Eligible(guests[currentIndex], retain)) {
        if (best < 0 || best == currentIndex || guests[currentIndex].distance2 * 0.49f <= bestD2)
            return currentIndex;
    }
    return best;
}

} // namespace mission_targeting
