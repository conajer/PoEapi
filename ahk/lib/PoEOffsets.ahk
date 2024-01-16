;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.23", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        , "all_game_states"             : 0x48 }

      , "IngameState" : { "in_game_ui"                  : 0x98
                        , "in_game_data"                : 0x18
                        ,     "server_data"             : 0x870
                        , "ui_root"                     : 0x320
                        , "hovered"                     : 0x358
                        , "hovered_item"                : 0x368
                        , "camera"                      : 0x78
                        ,     "width"                   : 0x340
                        ,     "height"                  : 0x344
                        ,     "matrix"                  : 0x1e8 }

      , "IngameData"  : { "world_area"                  : 0xb0
                        , "area_data"                   : 0xb8
                        ,     "area_index"              : 0x38
                        , "area_level"                  : 0xd4
                        , "area_hash"                   : 0x114
                        , "local_player"                : 0x878
                        , "entity_list"                 : 0x928
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x930 }

      , "IngameUI"    : { "lefe_panel"                  : 0x540
                        , "right_panel"                 : 0x548
                        , "panel_flags"                 : 0x550
                        , "entity_list"                 : 0x620
                        ,     "root"                    : 0x2b8
                        ,     "count"                   : 0x2c0 }

      , "ServerData"  : { "player_data"                 : 0x9858
                        ,     "passive_skills"          : 0x180
                        ,     "player_class"            : 0x238
                        ,     "level"                   : 0x23c
                        ,     "refund_points_left"      : 0x240
                        ,     "skill_points_from_quest" : 0x244
                        ,     "ascendancy_skill_points" : 0x24c
                        , "league"                      : 0x9e28
                        , "latency"                     : 0x9ec0
                        , "party_status"                : 0xa0b8
                        , "stash_tabs"                  : 0x9ec8
                        , "inventory_slots"             : 0xa4d0 }

      , "Entity"      : { "internal"                    : 0x8
                        ,     "path"                    : 0x8
                        , "component_list"              : 0x10
                        , "id"                          : 0x90 }

      , "Element"     : { "self"                        : 0x28
                        , "childs"                      : 0x68
                        , "root"                        : 0xb8
                        , "parent"                      : 0x140
                        , "position"                    : 0xc0
                        , "scale"                       : 0xd8
                        , "is_visible"                  : 0x149
                        , "is_enabled"                  : 0x14d
                        , "size"                        : 0x190
                        , "highlighted"                 : 0x198
                        , "text"                        : 0x2f8
                        , "item"                        : 0x380 }

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
                        , "cols"                        : 0x14c
                        , "rows"                        : 0x150
                        , "cells"                       : 0x170
                        , "count"                       : 0x190 } } }
