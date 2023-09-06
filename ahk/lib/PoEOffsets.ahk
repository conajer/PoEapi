;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.22", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        , "all_game_states"             : 0x48 }

      , "IngameState" : { "in_game_ui"                  : 0x98
                        , "in_game_data"                : 0x18
                        ,     "server_data"             : 0x850
                        , "ui_root"                     : 0x2a0
                        , "hovered"                     : 0x2d8
                        , "hovered_item"                : 0x2e8
                        , "camera"                      : 0x78
                        ,     "width"                   : 0x340
                        ,     "height"                  : 0x344
                        ,     "matrix"                  : 0x1e8 }

      , "IngameData"  : { "world_area"                  : 0x88
                        , "area_data"                   : 0x90
                        ,     "area_index"              : 0x38
                        , "area_level"                  : 0xa8
                        , "area_hash"                   : 0xec
                        , "local_player"                : 0x858
                        , "entity_list"                 : 0x908
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x910 }

      , "IngameUI"    : { "lefe_panel"                  : 0x550
                        , "right_panel"                 : 0x558
                        , "panel_flags"                 : 0x560
                        , "entity_list"                 : 0x628
                        ,     "root"                    : 0x2a0
                        ,     "count"                   : 0x2a8 }

      , "ServerData"  : { "player_data"                 : 0x9718
                        ,     "passive_skills"          : 0x180
                        ,     "player_class"            : 0x238
                        ,     "level"                   : 0x23c
                        ,     "refund_points_left"      : 0x240
                        ,     "skill_points_from_quest" : 0x244
                        ,     "ascendancy_skill_points" : 0x24c
                        , "league"                      : 0x9b60
                        , "latency"                     : 0x9bf8
                        , "party_status"                : 0x9df0
                        , "stash_tabs"                  : 0x9c00
                        , "inventory_slots"             : 0xa1e8 }

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
                        , "highlighted"                 : 0x1b8
                        , "text"                        : 0x2e8
                        , "item"                        : 0x370 }

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
