;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.12.5c", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        ,     "current"                 : 0x0
                        , "game_states"                 : 0x48
                        ,     "root"                    : 0x8 }
                      
      , "IngameState" : { "name"                        : 0x10
                        , "in_game_ui"                  : 0x80
                        , "in_game_data"                : 0x500
                        , "server_data"                 : 0x508
                        , "ui_root"                     : 0x630
                        , "time_in_game"                : 0x6dc
                        , "unknown"                     : 0x6f0
                        , "camera"                      : 0x1178
                        , "width"                       : 0x1180
                        , "height"                      : 0x1184
                        , "matrix"                      : 0x1238 }
                      
      , "IngameData"  : { "world_area"                  : 0x68
                        , "area_level"                  : 0x80
                        , "area_hash"                   : 0xe4
                        , "local_player"                : 0x408
                        , "entity_list"                 : 0x490
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x498 }
                      
      , "IngameUI"    : { "inventory"                   : 0x520
                        ,     "grid"                    : 0x3a8
                        , "stash"                       : 0x528
                        ,     "tabs"                    : 0x2c8
                        , "overlay_map"                 : 0x5a8
                        ,     "large"                   : 0x230
                        ,     "small"                   : 0x238
                        , "chat"                        : 0x400
                        , "entity_list"                 : 0x5b0
                        ,     "root"                    : 0x2a0
                        , "vendor"                      : 0x638
                        , "purchase"                    : 0x658
                        , "sell"                        : 0x660
                        , "trade"                       : 0x668
                        , "gem_level_up"                : 0x8c8
                        , "notifications"               : 0x910 }
                      
      , "ServerData"  : { "player_data"                 : 0x7898
                        ,     "passive_skills"          : 0x160
                        ,     "player_class"            : 0x200
                        ,     "level"                   : 0x204
                        ,     "skill_points_from_quest" : 0x20c
                        ,     "skill_points_left"       : 0x210
                        ,     "ascendancy_skill_points" : 0x214
                        , "league"                      : 0x78d0
                        , "latency"                     : 0x7948
                        , "party_status"                : 0x7a80
                        , "stash_tabs"                  : 0x7958
                        , "inventory_slots"             : 0x7ce8 }
                      
      , "Entity"      : { "internal"                    : 0x8
                        ,     "path"                    : 0x8
                        , "component_list"              : 0x10
                        , "id"                          : 0x58 }
                      
      , "Element"     : { "self"                        : 0x18
                        , "childs"                      : 0x38
                        , "root"                        : 0x88
                        , "parent"                      : 0x90
                        , "position"                    : 0x98
                        , "scale"                       : 0x108
                        , "is_visible"                  : 0x111
                        , "size"                        : 0x130
                        , "highlighted"                 : 0x178
                        , "text"                        : 0x2e8 }
                      
      , "StashTab"    : { "name"                        : 0x8
                        , "inventory_id"                : 0x28
                        , "type"                        : 0x34
                        , "index"                       : 0x38
                        , "folder_id"                   : 0x3a
                        , "flags"                       : 0x3d
                        , "is_affinity"                 : 0x3e
                        , "affinities"                  : 0x3f }
                      
      , "Inventory"   : { "id"                          : 0x0
                        , "internal"                    : 0x8
                        ,     "type"                    : 0x0
                        ,     "sub_type"                : 0x4
                        ,     "is_requested"            : 0x4
                        ,     "cols"                    : 0xc
                        ,     "rows"                    : 0x10
                        ,     "cells"                   : 0x30
                        ,     "count"                   : 0x50 } } }
