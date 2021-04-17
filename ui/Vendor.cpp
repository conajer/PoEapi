/*
* Vendor.c, 9/28/2020 12:07 PM
*/

static std::map<string, int> vendor_offsets {
    {"service",  0x268},
        {"list", 0x418},
    {"name",     0x278},
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

    Vendor(addrtype address) : Element(address, &vendor_offsets) {
        add_method(L"name", this, (MethodType)&Vendor::name, AhkWStringPtr);
        add_method(L"isSelected", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"getServices", this, (MethodType)&Vendor::__get_services, AhkObject);
    }

    wstring& name() {
        Element element(read<addrtype>("name"));
        vendor_name = element.get_text();

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

        return services;
    }
};
