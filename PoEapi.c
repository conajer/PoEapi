/*
* PoEapi.c, 8/27/2020 2:22 PM
*/

#define DLLEXPORT extern "C" __declspec(dllexport)

enum EventTypes {
    WM_POEAPI_LOG = 0x9000,
    WM_PLAYER_CHANGED,
    WM_LEAGUE_CHANGED,
    WM_PLAYER_LIFE,
    WM_PLAYER_MANA,
    WM_PLAYER_ENERGY_SHIELD,
    WM_PLAYER_DIED,
    WM_PLAYER_USE_SKILL,
    WM_PLAYER_MOVE,
    WM_BUFF_ADDED,
    WM_BUFF_REMOVED,
    WM_AREA_CHANGED,
    WM_MONSTER_CHANGED,
    WM_MINION_CHANGED,
    WM_KILLED,
    WM_DELVE_CHEST,
};

int major_version = 0;
int minor_version = 6;
int patch_level = 2;

const char* supported_PoE_version = "3.11.1f";

DLLEXPORT void poeapi_get_version(int& major, int& minor, int &patch) {
    major = major_version;
    minor = minor_version;
    patch = patch_level;
}
