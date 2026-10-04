#pragma once
#include <stdint.h>

// Called on host only. We never kill the host to force mission failure.
// Instead, an observed guest death makes the game's existing scripted
// IS_PLAYER_DEAD / IS_CHAR_DEAD check see a death, so the mission follows
// its OWN failure branch, including native cleanup and text mirroring.
void GuestMissionDeathBegin(int mission);
void GuestMissionDeathEnd();
void GuestMissionDeathFrame(bool inGame);
bool GuestMissionDeathAffectsCondition(uint16_t opcode, bool missionScript, bool hostSubject);
