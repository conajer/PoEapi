;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.17", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        , "all_game_states"             : 0x48 }

      , "IngameState" : { "in_game_ui"                  : 0x438
                        , "in_game_data"                : 0x18
                        ,     "server_data"             : 0x670
                        , "ui_root"                     : 0x1a0
                        , "hovered"                     : 0x1d8
                        , "hovered_item"                : 0x1e8
                        , "camera"                      : 0x78
                        ,     "width"                   : 0xb0
                        ,     "height"                  : 0xb4
                        ,     "matrix"                  : 0x128 }

      , "IngameData"  : { "world_area"                  : 0x80
                        , "area_data"                   : 0x88
                        ,     "area_index"              : 0x38
                        , "area_level"                  : 0xa0
                        , "area_hash"                   : 0x104
                        , "local_player"                : 0x678
                        , "entity_list"                 : 0x728
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x730 }

      , "IngameUI"    : { "lefe_panel"                  : 0x550
                        , "right_panel"                 : 0x558
                        , "panel_flags"                 : 0x560
                        , "entity_list"                 : 0x690
                        ,     "root"                    : 0x2a8
                        ,     "count"                   : 0x2b0 }

      , "ServerData"  : { "player_data"                 : 0x89a0
                        ,     "passive_skills"          : 0x180
                        ,     "player_class"            : 0x238
                        ,     "level"                   : 0x23c
                        ,     "refund_points_left"      : 0x240
                        ,     "skill_points_from_quest" : 0x244
                        ,     "ascendancy_skill_points" : 0x24c
                        , "league"                      : 0x89e0
                        , "latency"                     : 0x8a70
                        , "party_status"                : 0x8c48
                        , "stash_tabs"                  : 0x8a78
                        , "inventory_slots"             : 0x8eb8 }

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
                        , "text"                        : 0x3a0
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
