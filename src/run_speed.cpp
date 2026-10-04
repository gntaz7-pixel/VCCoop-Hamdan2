// Adjustable on-foot run/sprint animation rate for the local player and remote puppets.
// Normal walk, vehicle physics, combat animations and network positions are untouched.
#include "run_speed.h"
#include "run_speed_policy.h"
#include "game.h"
#include "entities.h"
#include "net.h"
#include "vccoop.h"

using namespace game;

static runspeed::RateController g_rates;

static void ApplyToPlayer(void *ped, bool cutscene)
{
    if (!ped) return;
    void *clump = Field<void *>(ped, 0x4C);
    if (!clump) return;
    const bool onFoot = !InVehicle(ped) && !EnteringState(PedState(ped)) &&
                        !ExitingState(PedState(ped));
    const bool alive = Health(ped) > 0.f;
    const int state = MoveState(ped);
    // No change to velocity/position packets. Native engine calculates the local movement;
    // receivers already interpolate the transmitted coordinates as usual.
    for (void *a = FirstAssoc(clump); a; a = NextAssoc(a)) {
        const int id = Field<int16_t>(a, 0x2C);
        if (id != 1 && id != 2) continue; // RUN and SPRINT, not walk or action animations
        const bool slow = runspeed::ShouldScale(onFoot, alive, cutscene, state, id,
                        Field<float>(a, 0x18), Field<float>(a, 0x1C));
        float &speed = Field<float>(a, 0x24); // CAnimBlendAssociation::m_fSpeed
        speed = g_rates.Update(ped, a, Field<void *>(a, 0x14), id,
                               speed, slow, g_cfg.runSpeedPercent);
    }
}

void RunSpeedFrame(bool inGame)
{
    g_rates.BeginFrame();
    if (inGame && g_cfg.runSpeedPercent <= 100) {
        const bool cutscene = *(bool *)0xA10AB2;
        ApplyToPlayer(FindPlayerPed(), cutscene);
        // Puppet positions are sourced from the sender: only match their visible stride rate.
        for (int i = 0; i < MAX_PLAYERS; ++i)
            if (g_players[i].connected) ApplyToPlayer(PuppetPed(i), cutscene);
    }
    g_rates.EndFrame();
}
