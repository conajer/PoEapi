;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.12.5c", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        ,     "current"                 : 0x0
                        , "game_states"                 : 0x48
                        ,     "root"                    : 0x8 }
                      
      , "IngameState" : { "name"                        : 0x10
                        , "in_game_ui"                  : 0x98
                        , "in_game_data"                : 0x30
                        , "server_data"                 : 0x418
                        , "ui_root"                     : 0x540
                        , "time_in_game"                : 0x5ec
                        , "unknown"                     : 0x600
                        , "camera"                      : 0x1080
                        , "width"                       : 0x1088
                        , "height"                      : 0x108c
                        , "matrix"                      : 0x1140 }
                      
      , "IngameData"  : { "world_area"                  : 0x78
                        , "area_level"                  : 0x90
                        , "area_hash"                   : 0xf4
                        , "local_player"                : 0x438
                        , "entity_list"                 : 0x4c0
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x4c8 }
                      
      , "IngameUI"    : { "inventory"                   : 0x530
                        ,     "grid"                    : 0x3a8
                        , "stash"                       : 0x538
                        ,     "tabs"                    : 0x2c0
                        , "overlay_map"                 : 0x5b8
                        ,     "large"                   : 0x230
                        ,     "small"                   : 0x238
                        , "chat"                        : 0x410
                        , "entity_list"                 : 0x5c0
                        ,     "root"                    : 0x2a0
                        , "vendor"                      : 0x648
                        , "purchase"                    : 0x668
                        , "sell"                        : 0x670
                        , "trade"                       : 0x678
                        , "gem_level_up"                : 0x990
                        , "notifications"               : 0x948 }
                      
      , "ServerData"  : { "player_data"                 : 0x7b18
                        ,     "passive_skills"          : 0x160
                        ,     "player_class"            : 0x200
                        ,     "level"                   : 0x204
                        ,     "skill_points_from_quest" : 0x20c
                        ,     "skill_points_left"       : 0x210
                        ,     "ascendancy_skill_points" : 0x214
                        , "league"                      : 0x7be0
                        , "latency"                     : 0x7c58
                        , "party_status"                : 0x7dd0
                        , "stash_tabs"                  : 0x7c68
                        , "local_player"                : 0x7f08
                        , "inventory_slots"             : 0x7ff8 }
                      
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
