/*
* Vendor.c, 9/28/2020 12:07 PM
*/

static std::map<string, int> vendor_offsets {
    {"service",  0x268},
        {"list", 0x408},
    {"name",     0x278},
};

class Vendor : public Element {
public:

    wstring vendor_name;
    std::map<wstring, shared_ptr<Element>> services;

    Vendor(addrtype address) : Element(address, &vendor_offsets) {
        add_method(L"name", this, (MethodType)&Vendor::name, AhkWStringPtr);
        add_method(L"isSelected", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"getServices", this, (MethodType)&Vendor::get_services);
    }

    wstring& name() {
        if (vendor_name.empty()) {
            Element element(read<addrtype>("name"));
            vendor_name = element.get_text();
        }

        return vendor_name;
    }

    std::map<wstring, shared_ptr<Element>>& get_services() {
	    Element service_list(read<addrtype>("service", "list"));
        services.clear();

        for (auto& i : service_list.get_childs()) {
            printf("%d\n", i->child_count());
            if (i->child_count() == 0)
                continue;

            auto service = i->get_child(0);
            if (service->child_count() > 0)
                continue;
            
            wstring& service_name = service->get_text();
            services[service_name] = service;
        }

        if (obj_ref && services.size() > 0) {
            AhkObjRef* ahkobj_ref;

            __get(L"Services", &ahkobj_ref, AhkObject);
            if (!ahkobj_ref) {
                __set(L"Services", nullptr, AhkObject, nullptr);
                __get(L"Services", &ahkobj_ref, AhkObject);
            }

            AhkObj svcs(ahkobj_ref);
            for (auto& i : services)
                svcs.__set(i.first.c_str(), (AhkObjRef*)*i.second, AhkObject, nullptr);
        }

        return services;
    }
};
