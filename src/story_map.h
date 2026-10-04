// Host-authoritative *story-contact* icons; does not touch mission objectives,
// player blips, property/shop icons, checkpoints or independent guest challenges.
#pragma once
#include <stdint.h>
#include "story_map_policy.h"
struct MainMapCmd {
    bool tracked;
    uint16_t op;
    story_map::Key key;
    uint8_t outputType;
    uint16_t outputWhere;
    uint32_t removeHandle;
};
MainMapCmd StoryMapBefore(void *script, int ip, uint16_t op);
void StoryMapAfter(void *script, const MainMapCmd &cmd);
void StoryMapFrame(bool inGame);
void StoryMapSendSnapshot(int peer);
void StoryMapReceiveSnapshot(const uint8_t *data, int len);
