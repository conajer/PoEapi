/*
* InGameUI.cpp, 8/18/2020 6:46 PM
*/

#include <unordered_map>

#include "ui/Inventory.cpp"
#include "ui/Stash.cpp"
#include "ui/Vendor.cpp"
#include "ui/Purchase.cpp"
#include "ui/Sell.cpp"
#include "ui/Trade.cpp"
#include "ui/OverlayMap.cpp"
#include "ui/Chat.cpp"
#include "ui/Notifications.cpp"
#include "ui/Atlas.cpp"
#include "ui/Skills.cpp"

std::map<string, int> in_game_ui_offsets {
    {"overlay_map",         3},
    {"gem_level_up",        5},
    {"vendor",             20},
    {"skills",             22},
    {"atlas",              24},
    {"atlas_skills",       25},
    {"inventory",          33},
        {"grid",        0x3d8},
    {"stash",              34},
        {"tabs",        0x2f8},
    {"kirac_mission",      61},
    {"temple",             66},
    {"delve_chart",        70},
    {"syndicate",          72},
    {"horticrafting",      83},
    {"heist_locker",       86},
    {"favours",            87},
    {"expedition_locker",  89},
    {"purchase",           91},
    {"purchase2",          92},
    {"sell",               93},
    {"sell2",              94},
    {"trade",              95},
    {"chat",            0x480},
    {"notifications",   0x9f0},
    {"lefe_panel",      0x540},
    {"right_panel",     0x548},
    {"panel_flags",     0x550},
    {"entity_list",     0x6a8},
        {"root",        0x2a8},
        {"count",       0x2b0},
};

class InGameUI : public Element {
public:

    unique_ptr<Inventory> inventory;
    unique_ptr<Stash> stash;
    unique_ptr<Vendor> vendor;
    unique_ptr<Purchase> purchase;
    unique_ptr<Sell> sell;
    unique_ptr<Trade> trade;
    unique_ptr<OverlayMap> large_map, corner_map;
    unique_ptr<Chat> chat;
    unique_ptr<Notifications> notifications;
    unique_ptr<Atlas> atlas;
    unique_ptr<Skills> skills;
    std::map<string, shared_ptr<Element>> active_panels;
    shared_ptr<Entity> nearest_entity;

    InGameUI(addrtype address) : Element(address, &in_game_ui_offsets) {
        if (address && is_valid()) {
            get_inventory();
            get_stash();
            get_vendor();
            get_trade();
            get_overlay_map();
            get_chat();
            get_notifications();
            get_atlas();
            get_skills();

            std::vector<string> active_panel_names = {
                "skills", "atlas", "atlas_skills", "kirac_mission", "temple", "delve_chart", "syndicate", "horticrafting"
            };

            for (auto& i : active_panel_names)  
                active_panels[i] = get_child(i);
        }
    }

    void __new() {
        Element::__new();
        __set(L"inventory", (AhkObjRef*)*inventory, AhkObject,
              L"stash", (AhkObjRef*)*stash, AhkObject,
              L"largeMap", (AhkObjRef*)*large_map, AhkObject,
              L"cornerMap", (AhkObjRef*)*corner_map, AhkObject,
              L"chat", (AhkObjRef*)*chat, AhkObject,
              L"notifications", (AhkObjRef*)*notifications, AhkObject,
              L"atlas", (AhkObjRef*)*atlas, AhkObject,
              L"skills", (AhkObjRef*)*skills, AhkObject,
              nullptr);
    }

    bool has_active_panel() {
        if (address) {
            if (read<short>("panel_flags") || chat->is_opened() || vendor->is_selected())
                return true;

            for (auto& i : active_panels) {
                if (i.second && i.second->is_visible())
                    return true;
            }
        }

        return false;
    }

    Inventory* get_inventory() {
        if (shared_ptr<Element> e = get_child("inventory")) {
            addrtype addr = PoEMemory::read<addrtype>(e->address + (*offsets)["grid"]);
            if (!inventory || inventory->address != addr)
                inventory = unique_ptr<Inventory>(new Inventory(addr));
        }
        return inventory.get();
    }

    Stash* get_stash() {
        if (shared_ptr<Element> e = get_child("stash")) {
            addrtype addr = PoEMemory::read<addrtype>(e->address + (*offsets)["tabs"]);
            if (!stash || stash->address != addr)
                stash = unique_ptr<Stash>(new Stash(addr));
        }
        return stash.get();
    }

    Vendor* get_vendor() {
        if (!vendor) {
            shared_ptr<Element> e = get_child("vendor");
            vendor = unique_ptr<Vendor>(new Vendor(e->address));
        }
        return vendor.get();
    }

    Purchase* get_purchase() {
        map<string, vector<int>> v = {{"favours", {11}}, {"purchase", {6, 1, 0, 0}}, {"purchase2", {8, 1, 0, 0}}};

        purchase.reset();
        for (auto& i : v) {
            shared_ptr<Element> e = get_child(i.first);
            if (e->is_visible()) {
                if (e = e->get_child(i.second))
                    purchase = unique_ptr<Purchase>(new Purchase(e->address));
                break;
            }
        }

        return purchase.get();
    }

    Sell* get_sell() {
        map<string, vector<int>> v = {{"sell", {3}}, {"sell2", {4}}};

        sell.reset();
        for (auto& i : v) {
            shared_ptr<Element> e = get_child(i.first);
            if (e->is_visible()) {
                bool reverse = e->get_child(1)->is_visible();
                if (e = e->get_child(i.second)) {
                    sell = unique_ptr<Sell>(new Sell(e->address, reverse));
                }
                break;
            }
        }

        return sell.get();
    }

    Trade* get_trade() {
        shared_ptr<Element> e = get_child("trade");
        if (e = e->get_child(std::vector<int>{3, 1, 0, 0})) {
            if (!trade || trade->address != e->address)
                trade = unique_ptr<Trade>(new Trade(e->address));
            return trade.get();
        }

        return nullptr;
    }

    OverlayMap* get_overlay_map() {
        if (!large_map) {
            shared_ptr<Element> e = get_child("overlay_map");
            large_map.reset(new OverlayMap(e->get_child(0)->address));
            large_map->shift_modifier = -20.0;
            corner_map.reset(new OverlayMap(e->get_child(1)->address));
            corner_map->shift_modifier = 0;
        }
        
        auto& overlay_map = large_map->is_visible() ? large_map : corner_map;
        return overlay_map.get();
    }

    Chat* get_chat() {
        if (!chat) {
            //shared_ptr<Element> e = get_child("chat");
            chat = unique_ptr<Chat>(new Chat(read<addrtype>("chat")));
        }
        return chat.get();
    }

    Notifications* get_notifications() {
        if (!notifications) {
            //shared_ptr<Element> e = get_child("notifications");
            notifications = unique_ptr<Notifications>(new Notifications(read<addrtype>("notifications")));
        }
        return notifications.get();
    }

    Atlas* get_atlas() {
        if (!atlas) {
            shared_ptr<Element> e = get_child("atlas");
            atlas = unique_ptr<Atlas>(new Atlas(e->address));
        }
        return atlas.get();
    }

    Skills* get_skills() {
        if (!skills) {
            shared_ptr<Element> e = get_child("skills");
            skills = unique_ptr<Skills>(new Skills(e->address));
        }
        return skills.get();
    }

    int get_all_entities(EntityList& entities, EntityList& removed) {
        entities.swap(removed);
        entities.clear();
        addrtype root = read<addrtype>("entity_list", "root");
        int count = std::min(read<int>("entity_list", "count"), 1024);

        addrtype next = root;
        while (1) {
            next = PoEMemory::read<addrtype>(next);
            if (!next || next == root || entities.size() > count)
                break;

            addrtype label = PoEMemory::read<addrtype>(next + 0x10);
            if (!(PoEMemory::read<byte>(label + 0x161) & 0x8))
                continue;

            addrtype entity_address = PoEMemory::read<addrtype>(next + 0x18);
            int entity_id = PoEMemory::read<int>(entity_address + 0x60);
            auto i = removed.find(entity_id);
            if (i != removed.end()) {
                entities.insert(*i);
                removed.erase(i);
                continue;
            }

            std::shared_ptr<Entity> entity(new Entity(entity_address));
            entity->label = shared_ptr<Element>(new Element(label));
            entities[entity_id] = entity;
        }

        return entities.size();
    }

    shared_ptr<Entity>& get_nearest_entity(LocalPlayer& player, wstring text) {
        unsigned int dist, max_dist = -1;
        EntityList entities, removed;

        get_all_entities(entities, removed);
        nearest_entity = nullptr;
        for (auto i : entities) {
            if ((dist = player.dist(*i.second)) < max_dist) {
                if (i.second->name().find(text) != -1 || i.second->path.find(text) != -1) {
                    nearest_entity = i.second;
                    max_dist = dist;
                }
            }
        }

        return nearest_entity;
    }
};
