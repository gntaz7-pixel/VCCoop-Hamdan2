#include "guest_mission_death.h"
#include "guest_mission_death_policy.h"
#include "vccoop.h"
#include "net.h"
#include "mirror.h"
#include "util.h"

static guest_mission_death::Tracker g_death;
static uint32_t g_eventAt;
static bool g_checkSeen, g_warned;

static void ReadGuests(guest_mission_death::GuestState (&out)[MAX_PLAYERS - 1])
{
    const uint32_t now = GetTickCount();
    for (int p = 1; p < MAX_PLAYERS; ++p) {
        const NetPlayer &np = g_players[p];
        const MsgState &s = np.state;
        guest_mission_death::GuestState &g = out[p - 1];
        g.connected = np.connected;
        g.inGame = s.inGame != 0;
        // Never treat a stale/frozen state as a newly detected death.
        g.fresh = np.lastStateAt != 0 && (uint32_t)(now - np.lastStateAt) <= 2000;
        g.cutscene = s.cutscene != 0;
        // 'down' alone also means ARRESTED. Only health/death ped states count.
        g.dead = s.health <= 0.0f || s.pedState == 54 || s.pedState == 55;
    }
}

void GuestMissionDeathEnd()
{
    g_death.End();
    g_eventAt = 0;
    g_checkSeen = g_warned = false;
}

void GuestMissionDeathBegin(int mission)
{
    GuestMissionDeathEnd();
    // Do not affect INITIAL, local vehicle challenges, property purchases or solo.
    if (!g_cfg.host || !g_cfg.failMissionOnGuestDeath || mission == 0 || MirrorHostQuiet()) return;
    guest_mission_death::GuestState guests[MAX_PLAYERS - 1] = {};
    ReadGuests(guests);
    g_death.Begin(guests, MAX_PLAYERS - 1);
}

void GuestMissionDeathFrame(bool inGame)
{
    if (!inGame || !g_cfg.host || !g_cfg.failMissionOnGuestDeath ||
        !MissionUnderway() || MirrorHostQuiet()) {
        GuestMissionDeathEnd();
        return;
    }
    if (!g_death.Active()) return; // only MirrorMissionStart arms detection
    guest_mission_death::GuestState guests[MAX_PLAYERS - 1] = {};
    ReadGuests(guests);
    const int deadSlot = g_death.Poll(guests, MAX_PLAYERS - 1);
    if (deadSlot >= 0) {
        g_eventAt = GetTickCount();
        Log("coop mission : guest %d died during story mission; awaiting scripted host death check", deadSlot + 1);
    }
    if (g_eventAt && !g_checkSeen && !g_warned &&
        (uint32_t)(GetTickCount() - g_eventAt) > 5000) {
        g_warned = true;
        Log("coop mission : WARNING no player-death condition was seen; this mission may require a separate failure hook");
    }
}

bool GuestMissionDeathAffectsCondition(uint16_t op, bool missionScript, bool hostSubject)
{
    if (!g_cfg.host || !g_cfg.failMissionOnGuestDeath || !g_death.Pending() ||
        !guest_mission_death::MatchesHostDeathCheck(op, missionScript, hostSubject)) return false;
    if (!g_checkSeen) Log("coop mission : guest death applied to host mission's native death condition (%04X)", op);
    g_checkSeen = true;
    return true;
}
