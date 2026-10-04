#include "../src/guest_mission_death_policy.h"
#include <assert.h>
#include <stdio.h>
using namespace guest_mission_death;
int main()
{
    int checks=0;
#define CHECK(x) do { ++checks; assert((x)); } while (0)
    GuestState g[3] = {{true,true,true,false,false},{false,false,false,false,false},{false,false,false,false,false}};
    Tracker t;
    CHECK(!t.Active()); CHECK(!t.Pending()); CHECK(t.Poll(g,3)==-1);
    t.Begin(g,3); CHECK(t.Active()); CHECK(!t.Pending()); CHECK(t.Poll(g,3)==-1);
    g[0].dead=true; CHECK(t.Poll(g,3)==0); CHECK(t.Pending()); CHECK(t.Poll(g,3)==-1);
    g[0].dead=false; CHECK(t.Poll(g,3)==-1); CHECK(t.Pending()); // latch survives respawn
    t.End(); CHECK(!t.Pending()); CHECK(!t.Active());
    t.Begin(g,3); CHECK(!t.Pending()); // new mission resets
    g[0].dead=true; CHECK(t.Poll(g,3)==0);
    t.End(); t.Begin(g,3); CHECK(!t.Pending()); CHECK(t.Poll(g,3)==-1); // already dead at start
    g[0].dead=false; CHECK(t.Poll(g,3)==-1); g[0].dead=true; CHECK(t.Poll(g,3)==0);
    t.End(); g[0].dead=false; t.Begin(g,3);
    g[0].connected=false; CHECK(t.Poll(g,3)==-1); g[0].connected=true; g[0].dead=true;
    CHECK(t.Poll(g,3)==-1); CHECK(!t.Pending()); // reconnect establishes baseline
    t.End(); g[0].dead=false; t.Begin(g,3);
    g[0].fresh=false; g[0].dead=true; CHECK(t.Poll(g,3)==-1);
    g[0].fresh=true; CHECK(t.Poll(g,3)==-1); CHECK(!t.Pending()); // stale packet cannot fail
    t.End(); g[0].dead=false; t.Begin(g,3); g[0].cutscene=true; g[0].dead=true;
    CHECK(t.Poll(g,3)==-1); g[0].cutscene=false; CHECK(t.Poll(g,3)==-1); // cutscene excluded
    t.End(); g[0].dead=false; t.Begin(g,3); g[0].dead=true;
    CHECK(t.Poll(g,3)==0); CHECK(t.Pending());
    CHECK(MatchesHostDeathCheck(0x0117,true,true));
    CHECK(MatchesHostDeathCheck(0x0118,true,true));
    CHECK(!MatchesHostDeathCheck(0x0117,false,true));
    CHECK(!MatchesHostDeathCheck(0x0118,true,false));
    CHECK(!MatchesHostDeathCheck(0x00E0,true,true));
    CHECK(!MatchesHostDeathCheck(0x03EE,true,true));
    CHECK(!MatchesHostDeathCheck(0x0117,true,false));
    t.End(); CHECK(!t.Pending());
    // Arrested guests have a 'down' flag but their health/death states remain alive.
    // The actual game bridge maps arrests to dead=false, so no failure is emitted.
    g[0].dead=false; t.Begin(g,3); CHECK(t.Poll(g,3)==-1);
    CHECK(!t.Pending());
    // Third participant can also fail the shared mission; disconnected ones cannot.
    g[2] = {true,true,true,false,false};
    CHECK(t.Poll(g,3)==-1);
    g[2].dead=true; CHECK(t.Poll(g,3)==2); CHECK(t.Pending());
    t.End(); g[2].dead=false; t.Begin(g,3);
    g[1] = {true,true,true,false,false};
    CHECK(t.Poll(g,3)==-1);
    g[1].dead=true; CHECK(t.Poll(g,3)==1);
    t.End(); CHECK(!t.Pending());
    // A dead-on-arrival player (e.g. during join/loading) does not count.
    g[1] = {true,true,true,false,true}; t.Begin(g,3);
    CHECK(t.Poll(g,3)==-1); CHECK(!t.Pending());
    g[1].dead=false; CHECK(t.Poll(g,3)==-1);
    g[1].dead=true; CHECK(t.Poll(g,3)==1);
    puts("Guest mission death policy: all tests PASS");
    printf("Assertions: %d\n",checks);
    return 0;
}
