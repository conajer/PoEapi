;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.14.0c", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        ,     "current"                 : 0x0
                        , "game_states"                 : 0x48
                        ,     "root"                    : 0x8 }
                      
      , "IngameState" : { "name"                        : 0x10
                        , "load_stage1"                 : 0x40
                        , "in_game_ui"                  : 0xa0
                        , "in_game_data"                : 0x30
                        , "server_data"                 : 0x420
                        , "ui_root"                     : 0x548
                        , "time_in_game"                : 0x5f4
                        , "load_stage2"                 : 0x608
                        , "camera"                      : 0x1080
                        , "width"                       : 0x1088
                        , "height"                      : 0x108c
                        , "matrix"                      : 0x1140 }
                      
      , "IngameData"  : { "world_area"                  : 0x80
                        , "area_level"                  : 0xa0
                        , "area_hash"                   : 0x104
                        , "local_player"                : 0x478
                        , "entity_list"                 : 0x520
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x528 }
                      
      , "IngameUI"    : { "inventory"                   : 0x538
                        ,     "grid"                    : 0x3a8
                        , "stash"                       : 0x540
                        ,     "tabs"                    : 0x2c0
                        , "overlay_map"                 : 0x5c0
                        ,     "large"                   : 0x230
                        ,     "small"                   : 0x238
                        , "chat"                        : 0x418
                        , "lefe_panel"                  : 0x510
                        , "right_panel"                 : 0x518
                        , "panel_flags"                 : 0x520
                        , "skills"                      : 0x568
                        , "atlas"                       : 0x570
                        , "entity_list"                 : 0x5c8
                        ,     "root"                    : 0x2a0
                        , "vendor"                      : 0x650
                        , "purchase"                    : 0x670
                        , "sell"                        : 0x678
                        , "trade"                       : 0x680
                        , "favours"                     : 0x7f0
                        , "gem_level_up"                : 0x998
                        , "notifications"               : 0x958 }
                      
      , "ServerData"  : { "player_data"                 : 0x7d18
                        ,     "passive_skills"          : 0x160
                        ,     "player_class"            : 0x200
                        ,     "level"                   : 0x204
                        ,     "skill_points_from_quest" : 0x20c
                        ,     "skill_points_left"       : 0x210
                        ,     "ascendancy_skill_points" : 0x214
                        , "league"                      : 0x7de0
                        , "latency"                     : 0x7e58
                        , "party_status"                : 0x7fd0
                        , "stash_tabs"                  : 0x7e68
                        , "local_player"                : 0x8108
                        , "inventory_slots"             : 0x8220 }
                      
      , "Entity"      : { "internal"                    : 0x8
                        ,     "path"                    : 0x8
                        , "component_list"              : 0x10
                        , "id"                          : 0x60 }
                      
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
