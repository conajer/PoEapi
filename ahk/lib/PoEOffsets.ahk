;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.30", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x28
                        , "all_game_states"             : 0x50 }

      , "IngameState" : { "in_game_ui"                  : 0x290
                        , "in_game_data"                : 0x218
                        ,     "server_data"             : 0x978
                        , "ui_root"                     : 0x800
                        , "hovered"                     : 0x838
                        , "camera"                      : 0x270
                        ,     "width"                   : 0x308
                        ,     "height"                  : 0x30c
                        ,     "matrix"                  : 0x198 }

      , "IngameData"  : { "world_area"                  : 0xc0
                        , "area_data"                   : 0xc8
                        ,     "area_index"              : 0x40
                        , "area_level"                  : 0xe4
                        , "area_hash"                   : 0x124
                        , "local_player"                : 0x980
                        , "entity_list"                 : 0xa58
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0xa60 }

      , "IngameUI"    : { "lefe_panel"                  : 0x630
                        , "right_panel"                 : 0x638
                        , "panel_flags"                 : 0x640
                        , "entity_list"                 : 0x720
                        ,     "root"                    : 0x510
                        ,     "count"                   : 0x518 }

      , "ServerData"  : { "player_data"                 : 0x9858
                        ,     "passive_skills"          : 0x180
                        ,     "player_class"            : 0x238
                        ,     "level"                   : 0x23c
                        ,     "refund_points_left"      : 0x240
                        ,     "skill_points_from_quest" : 0x244
                        ,     "ascendancy_skill_points" : 0x24c
                        , "league"                      : 0xc840
                        , "party_status"                : 0xcb88
                        , "stash_tabs"                  : 0xc910
                        , "inventory_slots"             : 0xce70 }

      , "Entity"      : { "internal"                    : 0x8
                        ,     "path"                    : 0x8
                        , "component_list"              : 0x10
                        , "id"                          : 0x88 }

      , "Element"     : { "self"                        : 0xb8
                        , "childs"                      : 0xf8
                        , "root"                        : 0x128
                        , "position"                    : 0x144
                        , "scale"                       : 0x15c
                        , "parent"                      : 0x1d8
                        , "is_visible"                  : 0x1e1
                        , "is_enabled"                  : 0x1e5
                        , "size"                        : 0x260
                        , "highlighted"                 : 0x268
                        , "text"                        : 0x358
                        , "item"                        : 0x448 }

      , "StashTab"    : { "name"                        : 0x8
                        , "inventory_id"                : 0x28
                        , "type"                        : 0x34
                        , "index"                       : 0x38
                        , "folder_id"                   : 0x3a
                        , "flags"                       : 0x3d
                        , "is_affinity"                 : 0x3e
                        , "affinities"                  : 0x3f }

      , "Inventory"   : { "type"                        : 0x140
                        , "sub_type"                    : 0x144
                        , "cols"                        : 0x150
                        , "rows"                        : 0x154
                        , "cells"                       : 0x170
                        , "count"                       : 0x190 } } }
