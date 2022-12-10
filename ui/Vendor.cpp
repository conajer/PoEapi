/*
* Vendor.c, 9/28/2020 12:07 PM
*/


static std::map<string, int> npc_menu_offsets {
    {"service",    0x2a8},
        {"list",   0x448},
    {"name",       0x2b8},
};

static std::map<string, int> npc_menu2_offsets {
    {"service",    0x2a8},
       {"list",    0x448},
    {"name",       0x2c0},
    {"sp_service", 0x288},
};

class NpcMenu : public Element {
public:

    NpcMenu(addrtype address, FieldOffsets* offsets = &npc_menu_offsets) : Element(address, offsets) {
    }

    wstring get_name() {
        Element element(read<addrtype>("name"));
        return element.get_text();
    }

    void get_services(std::map<wstring, shared_ptr<Element>>& services) {
        if (read<addrtype>("sp_service")) {
            Element special_service(read<addrtype>("sp_service"));
            for (auto& e : special_service.get_childs()) {
                Element service_list(PoEMemory::read<addrtype>(e->address + (*offsets)["list"]));
                for (auto& i : service_list.get_childs()) {
                    if (i->child_count() == 0)
                        continue;

                    auto service = i->get_child(0);
                    if (service->child_count() > 0)
                        continue;
                    
                    wstring& service_name = service->get_text();
                    services[service_name] = service;
                }
            }
        }

	    Element service_list(read<addrtype>("service", "list"));
        for (auto& i : service_list.get_childs()) {
            if (i->child_count() == 0)
                continue;

            auto service = i->get_child(0);
            if (service->child_count() > 0)
                continue;
            
            wstring& service_name = service->get_text();
            services[service_name] = service;
        }
    }
};

class NpcMenu2 : public NpcMenu {
public:

    NpcMenu2(addrtype address) : NpcMenu(address, &npc_menu2_offsets) {
    }
};

class Vendor : public Element {
private:

    AhkObjRef* __get_services() {
        AhkObj temp_services;
        for (auto& i : get_services())
            temp_services.__set(i.first.c_str(), (AhkObjRef*)*i.second, AhkObject, nullptr);
        __set(L"services", (AhkObjRef*)temp_services, AhkObject, nullptr);

        return temp_services;
    }

public:

    wstring vendor_name;
    std::map<wstring, shared_ptr<Element>> services;
    shared_ptr<NpcMenu> npc_menus[2];

    Vendor(addrtype address) : Element(address) {
        if (shared_ptr<Element> e = get_child(7))
            npc_menus[0] = shared_ptr<NpcMenu>(new NpcMenu(e->address));
        if (shared_ptr<Element> e = get_child(8))
            npc_menus[1] = shared_ptr<NpcMenu>(new NpcMenu2(e->address));

        add_method(L"name", this, (MethodType)&Vendor::name, AhkWStringPtr);
        add_method(L"isSelected", (Element*)this, (MethodType)&Vendor::is_selected, AhkBool);
        add_method(L"getServices", this, (MethodType)&Vendor::__get_services, AhkObject);
    }

    bool is_selected() {
        for (auto& menu : npc_menus) {
            if (menu && menu->is_visible())
                return true;
        }
        return false;
    }

    wstring& name() {
        vendor_name = L"";
        for (auto& menu : npc_menus) {
            if (menu && menu->is_visible())
                vendor_name = menu->get_name();
        }

        return vendor_name;
    }

    std::map<wstring, shared_ptr<Element>>& get_services() {
        services.clear();
        for (auto& menu : npc_menus) {
            if (menu && menu->is_visible())
                menu->get_services(services);
        }

        return services;
    }
};
