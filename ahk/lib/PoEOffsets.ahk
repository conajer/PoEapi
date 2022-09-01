;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.19", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        , "all_game_states"             : 0x48 }

      , "IngameState" : { "in_game_ui"                  : 0x450
                        , "in_game_data"                : 0x18
                        ,     "server_data"             : 0x778
                        , "ui_root"                     : 0x1a0
                        , "hovered"                     : 0x1d8
                        , "hovered_item"                : 0x1e8
                        , "camera"                      : 0x78
                        ,     "width"                   : 0xb0
                        ,     "height"                  : 0xb4
                        ,     "matrix"                  : 0x128 }

      , "IngameData"  : { "world_area"                  : 0x88
                        , "area_data"                   : 0x90
                        ,     "area_index"              : 0x38
                        , "area_level"                  : 0xa8
                        , "area_hash"                   : 0x10c
                        , "local_player"                : 0x780
                        , "entity_list"                 : 0x830
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x838 }

      , "IngameUI"    : { "lefe_panel"                  : 0x540
                        , "right_panel"                 : 0x548
                        , "panel_flags"                 : 0x550
                        , "entity_list"                 : 0x678
                        ,     "root"                    : 0x2a8
                        ,     "count"                   : 0x2b0 }

      , "ServerData"  : { "player_data"                 : 0x8a20
                        ,     "passive_skills"          : 0x180
                        ,     "player_class"            : 0x238
                        ,     "level"                   : 0x23c
                        ,     "refund_points_left"      : 0x240
                        ,     "skill_points_from_quest" : 0x244
                        ,     "ascendancy_skill_points" : 0x24c
                        , "league"                      : 0x8ee0
                        , "latency"                     : 0x8f70
                        , "party_status"                : 0x9148
                        , "stash_tabs"                  : 0x8f78
                        , "inventory_slots"             : 0x9500 }

      , "Entity"      : { "internal"                    : 0x8
                        ,     "path"                    : 0x8
                        , "component_list"              : 0x10
                        , "id"                          : 0x60 }

      , "Element"     : { "self"                        : 0x28
                        , "childs"                      : 0x68
                        , "root"                        : 0xd8
                        , "parent"                      : 0xe0
                        , "position"                    : 0xe8
                        , "scale"                       : 0x100
                        , "is_visible"                  : 0x161
                        , "is_enabled"                  : 0x165
                        , "size"                        : 0x180
                        , "highlighted"                 : 0x1c0
                        , "text"                        : 0x378
                        , "item"                        : 0x440
                        , "item_pos"                    : 0x448 }

      , "StashTab"    : { "name"                        : 0x8
                        , "inventory_id"                : 0x28
                        , "type"                        : 0x34
                        , "index"                       : 0x38
                        , "folder_id"                   : 0x3a
                        , "flags"                       : 0x3d
                        , "is_affinity"                 : 0x3e
                        , "affinities"                  : 0x3f }

      , "Inventory"   : { "type"                        : 0x138
                        , "sub_type"                    : 0x13c
                        , "cols"                        : 0x144
                        , "rows"                        : 0x148
                        , "cells"                       : 0x168
                        , "count"                       : 0x188 } } }
