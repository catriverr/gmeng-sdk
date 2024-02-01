/// This file is not a networking utility.
/// It is a utility to conversate and create strings that will be sent through the tsgmeng:0/network conversator

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
#include <map>

using string = std::string;

/// permissions as bits
/// example:
/// number 255 = (binary) 1 1 1 1 1 1 1 1
///                       max permissions, can do everything
/// number 1   = (binary) 0 0 0 0 0 0 0 1
///                       only permission is for the last one
/// number 0   = (binary) 0 0 0 0 0 0 0 0
///                       no permissions at all
template <typename T>
bool* get_perm_int(T number) {
    bool* perms[sizeof(T) * 8];
    for (int i = sizeof(T) * 8 - 1; i >= 0; --i) {
        *perms[i] = (number & (1ULL << i)) != 0;
    }
    return *perms;
};
/// permissions as bits
/// example:
/// number 255 = (binary) 1 1 1 1 1 1 1 1
///                       max permissions, can do everything
/// number 1   = (binary) 0 0 0 0 0 0 0 1
///                       only permission is for the last one
/// number 0   = (binary) 0 0 0 0 0 0 0 0
///                       no permissions at all
/// create([ true, true, true, true, true, true, true, true ]) = 255
/// create<uint64_t>([ true, true, true, ...(x64) ]) = 2^64 ( can be used for values higher than 8-bit limit 255 )
template <typename T = uint8_t>
T create_perm_int(const bool boolArray[sizeof(T) * 8]) {
    T result = 0;
    for (int i = 0; i < sizeof(T) * 8; ++i) {
        result |= static_cast<T>(boolArray[i]) << i;
    }
    return result;
};

namespace Gmeng {
    namespace EventPackage {
        template <typename promise_t>
        class promise {
            private:
                struct refuse { string reason; };
                struct accept { string reason; };
            public:
                bool resolved = false; promise_t resolution; string reason;
                using pcallback_t = std::function<void( std::function<accept(promise_t)>, std::function<refuse(string)> )>;
                promise(pcallback_t p) {
                    auto __func_accept__ = [&](promise_t obj) { this->resolved = true; this->resolution = obj;    };
                    auto __func_refuse__ = [&](string reason) { this->resolved = true; this->reason     = reason; };
                    p( __func_accept__, __func_refuse__ );
                };
                /// artificially alter the promise. this spoofs the result until an actual resolution is generated.

                void pbreak() {
                    this->resolved = true; this->resolution = false;
                };
                void pcomplete() {
                    this->resolved = true; this->resolution = true;
                };
        };
        namespace structure {
            struct pheader {
                std::vector<string> names;
                std::vector<string> values;
            };
            struct pack_t {
                std::uint64_t  event_type; std::uint8_t perms;   string key;
                std::string event_cast_by; structure::pheader event_headers;
            };
            // cache value
            struct cached_t {
                string identifier;
                std::vector<string> values;
            };
            #define cache_types unsigned int
            struct permission_object {
              public:
                bool SEND_PACKAGES       = false; // perms[0] //
                bool MODIFY_CLIENT_CACHE = false; // perms[1] //
                bool CREATE_CACHE_MAP    = false; // perms[2] //
                bool BECOME_PARTIER      = false; // perms[3] //
                bool UNDEFINED_4         = false; // perms[4] //
                bool UNDEFINED_5         = false; // perms[5] //
                bool UNDEFINED_6         = false; // perms[6] //
                bool UNDEFINED_7         = false; // perms[7] //
                permission_object(bool perms[8]) {
                    for (int i = 0; i < 8; i++) {
                        bool *p;
                        switch (i) {
                            case 0: p = &this->SEND_PACKAGES;       break;
                            case 1: p = &this->MODIFY_CLIENT_CACHE; break;
                            case 2: p = &this->CREATE_CACHE_MAP;    break;
                            case 3: p = &this->BECOME_PARTIER;      break;
                        };
                        *p = perms[i];
                    };
                };
            };
            struct cache_request {
                string key; string category;      cache_types type : 2; /// 00 = modify, 01 = create, 10 = delete, 11 = reset
                cached_t data; std::uint64_t partier_id; uint8_t perms;
            };
            // client cache, holds all cache for the values in a server
            // only modified if the server sends a __CONTROLLER_CACHE_RESET__( __CACHE_IDENTIFIER__ )
            class ccache_t {
                // [cache_key] is for checking packet relativity
                // its value is shared by all delegates to the server
                // it is used to check if the request is directed to
                // the correct client, and removes the possibility
                // of a security issue when the client is connected to
                // 2 different servers instead of a singular server.
                //
                // [server_key] is for checking packet authenticity
                // its value is only shared with client and the server
                // it is used for checking if a packet is sent by the server
                // if a packet is being spoofed or a fake packet being sent,
                // this will detect that since no other client has access to it.
                // while this can still be impersonated it is still a good measure.
                std::uint64_t    cache_key; std::uint64_t    server_key;
                std::vector<std::uint64_t>          registered_partiers;
                std::map<string, std::map<string, cached_t>> collection;
                promise<bool> add(cache_request r) {
                    return promise<bool>([&](auto accept, auto refuse) -> void {
                        if (r.key != this->cache_key) return refuse("cache packet key is invalid");
                        permission_object perms(get_perm_int(r.perms));
                        if (std::find(registered_partiers.begin(), registered_partiers.end(), r.partier_id) == registered_partiers.end()) {
                            if (!perms.BECOME_PARTIER) return refuse("cache holder has insufficient permissions to become a partier");
                            this->registered_partiers.push_back(r.partier_id);
                        };
                        if (!perms.SEND_PACKAGES) return refuse("cache holder cannot send packages");
                        if (!perms.MODIFY_CLIENT_CACHE) return refuse("cache holder does not have permissions to partake in client cache");
                        if (!collection.contains(r.category)) {
                            if (!perms.CREATE_CACHE_MAP) return refuse("cache holder cannot add to that category");
                            this->collection[r.category] = {};
                        };
                        this->collection[r.category][r.data.identifier] = r.data;
                    });
                };
            };
        };
        namespace s = structure;
        namespace {
            ///                  Sending Packages

            static s::pack_t create_package(int id, s::pheader header) {};
            static void      send_to_parent(s::pack_t p) {};
            ///                 Recieving Packages

            // recognizes a package type
            // for example a PLAYER_JOIN package will have the id of 7540
            // the event_type is also the first header of the package object
            // so, recognize() does:
            // { COMPARE( package->id, package->headers[0] ).is_equal() && package->id && validate(package); } | return;
            static s::pack_t recognize(string pack) {
                /// package string structure:
                ///           GMENG_NETWORKING #gm/version %(type) !(perms) *(caster-uuid) [ HEADERS ] @(cache_key)
                ///                 ^^            ^^         ^^       ^^         ^^            ^^           ^^
                ///            log identifier  version      type     perms   event owner    any value,   cache key, asures package authenticity
                ///                                                                         depends on
                ///                                                                         the type
                /// examples: GMENG_NETWORKING #gm/7.0.1-d %(7540) !(11101) *(server-id) [ PLAYER_JOIN,  pos=(0,0), name=catriverr ] @(nv196lv32)
                ///           GMENG_NETWORKING #gm/7.0.1-d %(7541) !(11101) *(server-id) [ PLAYER_LEAVE, name=catriverr ] @(nv196lv32)
            };
            ///                 Validating Packages
            static bool validate() {};
        };
    };
    namespace Networking {};
    namespace net = Networking;
};
