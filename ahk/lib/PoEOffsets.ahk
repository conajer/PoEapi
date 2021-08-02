;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.15", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        ,     "current"                 : 0x0
                        , "game_states"                 : 0x48
                        ,     "root"                    : 0x8 }
                      
      , "IngameState" : { "name"                        : 0x10
                        , "load_stage1"                 : 0x40
                        , "in_game_ui"                  : 0x98
                        , "in_game_data"                : 0x30
                        , "server_data"                 : 0x498
                        , "ui_root"                     : 0x5c0
                        , "hovered"                     : 0x5f8
                        , "hovered_item"                : 0x608
                        , "time_in_game"                : 0x674
                        , "load_stage2"                 : 0x688
                        , "camera"                      : 0x788
                        , "width"                       : 0x790
                        , "height"                      : 0x798
                        , "matrix"                      : 0x848 }
                      
      , "IngameData"  : { "world_area"                  : 0x80
                        , "area_level"                  : 0xa0
                        , "area_hash"                   : 0x104
                        , "local_player"                : 0x500
                        , "entity_list"                 : 0x5b0
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x5b8 }
                      
      , "IngameUI"    : { "inventory"                   : 0x540
                        ,     "grid"                    : 0x3b0
                        , "stash"                       : 0x548
                        ,     "tabs"                    : 0x2c8
                        , "overlay_map"                 : 0x648
                        ,     "large"                   : 0x238
                        ,     "small"                   : 0x240
                        , "chat"                        : 0x420
                        , "lefe_panel"                  : 0x518
                        , "right_panel"                 : 0x520
                        , "panel_flags"                 : 0x528
                        , "atlas"                       : 0x5f8
                        , "entity_list"                 : 0x650
                        ,     "root"                    : 0x2a8
                        , "trade"                       : 0x710
                        , "gem_level_up"                : 0xa28
                        , "notifications"               : 0xa20 }
                      
      , "ServerData"  : { "player_data"                 : 0x8218
                        ,     "passive_skills"          : 0x160
                        ,     "player_class"            : 0x200
                        ,     "level"                   : 0x204
                        ,     "skill_points_from_quest" : 0x20c
                        ,     "skill_points_left"       : 0x210
                        ,     "ascendancy_skill_points" : 0x214
                        , "league"                      : 0x82e0
                        , "latency"                     : 0x8358
                        , "party_status"                : 0x8500
                        , "stash_tabs"                  : 0x8370
                        , "local_player"                : 0x8638
                        , "inventory_slots"             : 0x8758 }
                      
      , "Entity"      : { "internal"                    : 0x8
                        ,     "path"                    : 0x8
                        , "component_list"              : 0x10
                        , "id"                          : 0x60 }
                      
      , "Element"     : { "self"                        : 0x20
                        , "childs"                      : 0x40
                        , "root"                        : 0x90
                        , "parent"                      : 0x98
                        , "position"                    : 0xa0
                        , "scale"                       : 0x110
                        , "is_visible"                  : 0x119
                        , "size"                        : 0x138
                        , "highlighted"                 : 0x180
                        , "text"                        : 0x2f0 }
                      
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
