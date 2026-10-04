#include "../src/mission_targeting.h"
#include <cassert>
#include <cstdio>
using mission_targeting::Guest;
using mission_targeting::ChooseGuest;

static Guest Good(float m) { return {true, true, true, true, true, true, true, m * m, 0}; }
int main()
{
    int n = 0;
#define EXPECT(expected, guests, current) do { assert(ChooseGuest(guests, 3, current) == expected); ++n; } while (0)
    Guest c[3] = { Good(30), Good(20), Good(15) };
    EXPECT(2, c, -1); // Prefer the closest active guest
    c[2].connected = false; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].alive = false; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].inGame = false; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].onFoot = false; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].notInCutscene = false; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].sameArea = false; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].fresh = false; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].heightDiff = 10.0f; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].distance2 = 80 * 80; EXPECT(1, c, -1); c[2] = Good(15);
    c[2].distance2 = -1; EXPECT(1, c, -1); c[2] = Good(15);
    EXPECT(1, c, 1); // Keep current guest despite a moderately closer player
    EXPECT(2, c, 0); // Retarget if new one is much closer
    c[0] = Good(100); c[1] = Good(100); c[2] = Good(55); EXPECT(2, c, 2); // Retain target outside acquisition range
    c[1] = Good(20); c[2] = Good(60); EXPECT(1, c, 2); // Switch away past retention radius
    c[0] = Good(100); c[1] = Good(100); c[2] = Good(100); EXPECT(-1, c, -1);
    c[0] = Good(20); c[0].connected = false; EXPECT(-1, c, -1);
    c[0] = Good(20); c[0].heightDiff = 7; EXPECT(0, c, -1);
    c[0].heightDiff = 7.1f; EXPECT(-1, c, -1);
    c[0] = Good(48); EXPECT(0, c, -1);
    c[0] = Good(48.01f); EXPECT(-1, c, -1);
    std::printf("PASS %d mission-targeting selection tests\n", n);
}
