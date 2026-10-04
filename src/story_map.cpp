// Story mission contact icons are produced by main.scm on BOTH machines.
// The guest's local storyline can momentarily diverge (save/reconnect/flags)
// and create contacts that the host has not unlocked. Rather than wiping
// the radar, track only CREATE_CONTACT_POINT (02A7/0570) from non-mission
// scripts; compare them against the host's authoritative, ordered snapshot.
// Change display (0x018B) instead of deleting a blip: its script still owns
// the handle and may change / remove it later. Re-enable when host unlocks.
#include "util.h"
#include "vccoop.h"
#include "game.h"
#include "net.h"
#include "mirror.h"
#include "story_map.h"
#include <string.h>

using namespace game;
namespace {
    enum { LOCAL_CAP = 96, HOST_CAP = 64 };
    struct Contact { uint32_t handle; story_map::Key key; bool hidden; };
    Contact host[HOST_CAP] = {}, guest[LOCAL_CAP] = {};
    int hostCount, guestCount;
    story_map::Key authoritative[story_map::MAX_CONTACTS] = {};
    int authoritativeCount;
    bool haveAuthority, hostSeenContact, dirty;
    bool previousInGame;

    // Decode SCM integer/float value *without moving the script IP*.
    bool Param(void *script, int &at, uint32_t &result) {
        const uint8_t *ss = ScriptSpace();
        uint8_t kind = ss[at];
        if (kind == 1 || kind == 6) { memcpy(&result, ss + at + 1, 4); at += 5; return true; }
        if (kind == 2 || kind == 3) {
            uint16_t off; memcpy(&off, ss + at + 1, 2);
            result = kind == 2 ? *(uint32_t *)(ss + off) : Field<uint32_t>(script, 0x30 + off * 4);
            at += 3; return true;
        }
        if (kind == 4) { result = (uint32_t)(int32_t)(int8_t)ss[at + 1]; at += 2; return true; }
        if (kind == 5) { int16_t v; memcpy(&v, ss + at + 1, 2); result = (uint32_t)(int32_t)v; at += 3; return true; }
        return false;
    }
    uint32_t ReadOut(void *script, const MainMapCmd &c) {
        return c.outputType == 2 ? *(uint32_t *)(ScriptSpace() + c.outputWhere) :
               Field<uint32_t>(script, 0x30 + c.outputWhere * 4);
    }
    void Display(uint32_t handle, int setting) {
        int32_t args[2] = { (int32_t)handle, setting };
        MirrorLocal(0x018B, 2, args); // CHANGE_BLIP_DISPLAY: 0=off, 2=radar-only
    }
    void Reconcile() {
        if (g_cfg.host || !g_cfg.syncStoryMap) return;
        for (int i = 0; i < guestCount; i++) {
            // Do not hide anything until the host has supplied a valid baseline.
            const bool shouldHide = haveAuthority && !story_map::Included(guest[i].key, authoritative, authoritativeCount);
            if (shouldHide == guest[i].hidden) continue;
            Display(guest[i].handle, shouldHide ? 0 : 2);
            guest[i].hidden = shouldHide;
            if (g_cfg.logScripts) Log("carte : contact invite %08X %s (liste hote: %d)", guest[i].handle,
                                       shouldHide ? "masque" : "affiche", authoritativeCount);
        }
    }
    void Erase(Contact *list, int &count, uint32_t handle) {
        for (int i = 0; i < count; i++) if (list[i].handle == handle) {
            list[i] = list[--count]; return;
        }
    }
    void Store(Contact *list, int &count, int cap, uint32_t handle, const story_map::Key &key) {
        for (int i = 0; i < count; i++) if (list[i].handle == handle) {
            list[i].key = key; list[i].hidden = false; return;
        }
        if (count < cap) list[count++] = {handle, key, false};
    }
    void Reset() {
        hostCount = guestCount = authoritativeCount = 0;
        haveAuthority = hostSeenContact = dirty = false;
    }
}

MainMapCmd StoryMapBefore(void *script, int ip, uint16_t op) {
    MainMapCmd c = {};
    if (!g_cfg.syncStoryMap || !script || Field<bool>(script, 0x85)) return c;
    if (op != 0x0164 && !story_map::IsContact(op)) return c;
    int at = ip + 2;
    c.op = op;
    if (op == 0x0164) {
        c.tracked = Param(script, at, c.removeHandle);
        return c;
    }
    uint32_t args[4] = {};
    for (int i = 0; i < 4; i++) if (!Param(script, at, args[i])) return c;
    // Output handle must be a SCM global or local. We read it AFTER the opcode.
    c.outputType = ScriptSpace()[at];
    if (c.outputType != 2 && c.outputType != 3) return c;
    memcpy(&c.outputWhere, ScriptSpace() + at + 1, 2);
    c.key = {args[0], args[1], args[2], args[3]};
    c.tracked = true;
    return c;
}

void StoryMapAfter(void *script, const MainMapCmd &c) {
    if (!c.tracked) return;
    if (c.op == 0x0164) {
        // A mission-created contact may later be removed by the *main*
        // script. The old mirror only watched mission scripts, so its
        // persistent copy survived forever on guests / late joiners.
        if (g_cfg.host) MirrorMainRemovedBlip(c.removeHandle);
        if (g_cfg.host) {
            for (int i = 0; i < hostCount; i++) if (host[i].handle == c.removeHandle) {
                Erase(host, hostCount, c.removeHandle); dirty = true; break;
            }
        } else Erase(guest, guestCount, c.removeHandle);
        return;
    }
    const uint32_t handle = ReadOut(script, c);
    if (handle == 0 || handle == 0xFFFFFFFFu) return;
    if (g_cfg.host) {
        Store(host, hostCount, HOST_CAP, handle, c.key);
        hostSeenContact = true;
        dirty = true;
    } else {
        Store(guest, guestCount, LOCAL_CAP, handle, c.key);
        Reconcile();
    }
}

void StoryMapSendSnapshot(int peer) {
    if (!g_cfg.syncStoryMap || !g_cfg.host || !hostSeenContact) return;
    uint8_t packet[story_map::PACKET_SIZE] = {};
    packet[0] = story_map::PACKET_KIND;
    // 255 suspends filtering if there are more than one packet can carry.
    if (!story_map::ValidCount(hostCount)) {
        packet[1] = 255;
        if (peer < 0) NetSendReliable(packet, 2); else NetSendReliableTo(peer, packet, 2);
        Log("carte : trop de points de contact (%d), filtre temporairement desactive", hostCount);
        return;
    }
    packet[1] = (uint8_t)hostCount;
    for (int i = 0; i < hostCount; i++) memcpy(packet + 2 + i * sizeof(story_map::Key), &host[i].key, sizeof(story_map::Key));
    int len = 2 + hostCount * (int)sizeof(story_map::Key);
    if (peer < 0) NetSendReliable(packet, len); else NetSendReliableTo(peer, packet, len);
    if (g_cfg.logScripts) Log("carte : %d contacts de mission envoyes %s", hostCount, peer < 0 ? "a tous" : "a un invite");
}

void StoryMapReceiveSnapshot(const uint8_t *data, int len) {
    if (g_cfg.host || !g_cfg.syncStoryMap || len < 2 || data[0] != story_map::PACKET_KIND) return;
    const unsigned count = data[1];
    if (count == 255) { haveAuthority = false; Reconcile(); return; }
    if (!story_map::ValidCount(count) || len != 2 + (int)count * (int)sizeof(story_map::Key)) return;
    authoritativeCount = (int)count;
    memcpy(authoritative, data + 2, count * sizeof(story_map::Key));
    haveAuthority = true;
    Reconcile();
}

void StoryMapFrame(bool inGame) {
    if (!g_cfg.syncStoryMap) return;
    if (!inGame) {
        if (previousInGame) Reset();
        previousInGame = false;
        return;
    }
    previousInGame = true;
    if (g_cfg.host && dirty) { dirty = false; StoryMapSendSnapshot(-1); }
}
