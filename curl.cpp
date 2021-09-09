/*
* curl.cpp, 8/24/2021 06:10 AM
*/

#include <string>
#include <memory>
#include <vector>
#include <curl/curl.h>

static const std::pair<string, int> curl_features[] = {
    {"AsynchDNS",      CURL_VERSION_ASYNCHDNS},
    {"Debug",          CURL_VERSION_DEBUG},
    {"TrackMemory",    CURL_VERSION_CURLDEBUG},
    {"IDN",            CURL_VERSION_IDN},
    {"IPv6",           CURL_VERSION_IPV6},
    {"Largefile",      CURL_VERSION_LARGEFILE},
    {"Unicode",        CURL_VERSION_UNICODE},
    {"SSPI",           CURL_VERSION_SSPI},
    {"GSS-API",        CURL_VERSION_GSSAPI},
    {"Kerberos",       CURL_VERSION_KERBEROS5},
    {"SPNEGO",         CURL_VERSION_SPNEGO},
    {"NTLM",           CURL_VERSION_NTLM},
    {"NTLM_WB",        CURL_VERSION_NTLM_WB},
    {"SSL",            CURL_VERSION_SSL},
    {"libz",           CURL_VERSION_LIBZ},
    {"brotli",         CURL_VERSION_BROTLI},
    {"zstd",           CURL_VERSION_ZSTD},
    {"CharConv",       CURL_VERSION_CONV},
    {"TLS-SRP",        CURL_VERSION_TLSAUTH_SRP},
    {"HTTP2",          CURL_VERSION_HTTP2},
    {"HTTP3",          CURL_VERSION_HTTP3},
    {"UnixSockets",    CURL_VERSION_UNIX_SOCKETS},
    {"HTTPS-proxy",    CURL_VERSION_HTTPS_PROXY},
    {"MultiSSL",       CURL_VERSION_MULTI_SSL},
    {"PSL",            CURL_VERSION_PSL},
    {"alt-svc",        CURL_VERSION_ALTSVC},
    {"HSTS",           CURL_VERSION_HSTS},
    {"gsasl",          CURL_VERSION_GSASL},
};

class curl : public AhkObj {
public:

    CURL* c;
    std::vector<std::string> request_headers;
    std::string status_text;
    std::vector<std::string> headers;
    std::string response_text;
    long status = 0;
    long verbose = 0;

    curl() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        c = curl_easy_init();

        curl_easy_setopt(c, CURLOPT_SSL_OPTIONS, CURLSSLOPT_AUTO_CLIENT_CERT);
        curl_easy_setopt(c, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(c, CURLOPT_HEADERDATA, this);
        curl_easy_setopt(c, CURLOPT_HEADERFUNCTION, __header_cb);
        curl_easy_setopt(c, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, __write_data_cb);
        curl_easy_setopt(c, CURLOPT_DEBUGDATA, this);
        curl_easy_setopt(c, CURLOPT_DEBUGFUNCTION, __debug_cb);

        add_property(L"responseText", &response_text, AhkString);
        add_property(L"status", &status, AhkInt);
        add_property(L"statusText", &status_text, AhkString);
        add_property(L"verbose", &verbose, AhkInt);

        add_method(L"escape", (curl*)this, (MethodType)&curl::escape, AhkString, ParamList{AhkString});
        add_method(L"unescape", (curl*)this, (MethodType)&curl::unescape, AhkString, ParamList{AhkString});
        add_method(L"get", (curl*)this, (MethodType)&curl::get, AhkInt, ParamList{AhkString});
        add_method(L"post", (curl*)this, (MethodType)&curl::post, AhkInt, ParamList{AhkString, AhkString, AhkString});
        add_method(L"getResponseHeader", (curl*)this, (MethodType)&curl::get_response_header, AhkString, ParamList{AhkString});
        add_method(L"getAllResponseHeaders", (curl*)this, (MethodType)&curl::get_all_response_headers, AhkObject);
        add_method(L"getAllRequestHeaders", (curl*)this, (MethodType)&curl::get_all_request_headers, AhkObject);
        add_method(L"getinfo", (curl*)this, (MethodType)&curl::getinfo, AhkInt, ParamList{AhkInt, AhkPointer});
        add_method(L"getAllOptions", (curl*)this, (MethodType)&curl::get_all_opts, AhkObject);
        add_method(L"getFeatures", (curl*)this, (MethodType)&curl::get_features, AhkObject);
        add_method(L"getProtocols", (curl*)this, (MethodType)&curl::get_protocols, AhkObject);
        add_method(L"setRequestHeader", (curl*)this, (MethodType)&curl::set_request_header, AhkVoid, ParamList{AhkString, AhkString});
        add_method(L"setopt", (curl*)this, (MethodType)&curl::setopt, AhkInt, ParamList{AhkInt, AhkPointer});
        add_method(L"strerror", (curl*)this, (MethodType)&curl::strerror, AhkString, ParamList{AhkInt});
        add_method(L"sslVersion", (curl*)this, (MethodType)&curl::ssl_version, AhkString);
        add_method(L"version", (curl*)this, (MethodType)&curl::version, AhkString);
    }

    ~curl() {
        curl_easy_cleanup(c);
    }

    char* escape(const char* url) {
        static char* escaped_url;

        curl_free(escaped_url);
        escaped_url = curl_easy_escape(c, url, 0);
        return escaped_url;
    }

    char* unescape(const char* url) {
        static char* unescaped_url;

        curl_free(unescaped_url);
        unescaped_url = curl_easy_unescape(c, url, 0, 0);
        return unescaped_url;
    }

    int get(const char* url) {
        curl_easy_setopt(c, CURLOPT_HTTPGET, 1L);
        return perform(url);
    }

    int post(const char* url, char* data, char* content_type) {
        curl_easy_setopt(c, CURLOPT_POST, 1L);
        if (data && data[0])
            curl_easy_setopt(c, CURLOPT_POSTFIELDS, data);
        if (content_type && content_type[0])
            set_request_header("Content-Type", content_type);

        return perform(url);
    }

    int perform(const char* url) {
        status_text.clear();
        headers.clear();
        response_text.clear();

        curl_easy_setopt(c, CURLOPT_VERBOSE, verbose);
        if (!request_headers.empty()) {
            curl_slist* list = nullptr;
            for (auto& i : request_headers)
                list = curl_slist_append(list, i.c_str());
            curl_easy_setopt(c, CURLOPT_HTTPHEADER, list);
        }

        curl_easy_setopt(c, CURLOPT_URL, url);
        CURLcode res = curl_easy_perform(c);

        if (res != CURLE_OK)
            return -res;
        curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &status);

        return status;
    }

    const char* get_response_header(const char* header) {
        for (auto& i : headers) {
            if (i.find(header) == 0) {
                int pos = i.find(':') + 2;
                return i.c_str() + pos;
            }
        }

        return nullptr;
    }

    AhkObjRef* get_all_response_headers() {
        AhkTempObj all_headers;
        for (auto& i : headers)
            all_headers.__set(L"", i.c_str(), AhkString, nullptr);
        return all_headers;
    }

    AhkObjRef* get_all_request_headers() {
        AhkTempObj all_headers;
        for (auto& i : request_headers)
            all_headers.__set(L"", i.c_str(), AhkString, nullptr);
        return all_headers;
    }

    int getinfo(CURLINFO info, void* ptr) {
        return curl_easy_getinfo(c, info, ptr);
    }

    AhkObjRef* get_all_opts() {
        AhkTempObj all_opts;
        for(auto i = curl_easy_option_next(0); i; i = curl_easy_option_next(i)) {
            AhkObj opt;
            opt.__set(L"name", i->name, AhkString,
                      L"id", i->id, AhkInt,
                      L"type", i->type, AhkInt,
                      L"flags", i->flags, AhkInt,
                      nullptr);
            all_opts.__set(i->name, (AhkObjRef*)opt, AhkObject, nullptr);
        }

        return all_opts;
    }

    AhkObjRef* get_features() {
        curl_version_info_data* info;
        AhkTempObj features;

        info = curl_version_info(CURLVERSION_NOW);
        if (info->features) {
            for (auto& i : curl_features) {
                if (info->features & i.second)
                    features.__set(L"", (char *)i.first.c_str(), AhkString, nullptr);
            }
        }

        return features;
    }

    AhkObjRef* get_protocols() {
        curl_version_info_data* info;
        AhkTempObj protocols;

        info = curl_version_info(CURLVERSION_NOW);
        for (const char* const* proto = info->protocols; *proto; ++proto)
            protocols.__set(L"", *proto, AhkString, nullptr);

        return protocols;
    }

    void set_request_header(const char*header, const char* value) {
        std::string header_field = std::string(header) + ": " + value;
        for (auto i = request_headers.begin(); i < request_headers.end(); ++i) {
            if (i->find(header) == 0) {
                return value ? (void)i->assign(header_field)
                             : (void)request_headers.erase(i);
            }
        }

        if (value)
            request_headers.push_back(header_field);
    }

    int setopt(CURLoption option, void* ptr) {
        return curl_easy_setopt(c, option, ptr);
    }

    const char* strerror(int err) {
        return curl_easy_strerror((CURLcode)-err);
    }

    const char* ssl_version() {
        curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
        return info->ssl_version;
    }

    const char* version() {
        curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
        return info->version;
    }

    static size_t __header_cb(char *ptr, size_t size, size_t nmemb, curl* c) {
        size *= nmemb;
        if (memchr(ptr, ':', size)) {
            c->headers.push_back(std::string(ptr, size - 2));
        } else if (!memcmp(ptr, "HTTP/", 5)) {
            c->headers.clear();
            c->status_text.assign(ptr, size - 2);
        }

        return size;
    }

    static size_t __write_data_cb(char *ptr, size_t size, size_t nmemb, curl* c) {
        size *= nmemb;
        c->response_text.append(ptr, size);

        return size;
    }

    static int __debug_cb(CURL* handle, curl_infotype type, char *data, size_t size, curl* c) {
        c->__call(L"__debug", AhkInt, type, AhkPointer, data, AhkInt, size, nullptr);
        return 0;
    }
};
