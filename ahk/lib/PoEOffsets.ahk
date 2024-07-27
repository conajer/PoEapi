;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.25", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        , "all_game_states"             : 0x48 }

      , "IngameState" : { "in_game_ui"                  : 0x298
                        , "in_game_data"                : 0x218
                        ,     "server_data"             : 0x8c8
                        , "ui_root"                     : 0x520
                        , "hovered"                     : 0x558
                        , "hovered_item"                : 0x568
                        , "camera"                      : 0x278
                        ,     "width"                   : 0x318
                        ,     "height"                  : 0x31c
                        ,     "matrix"                  : 0x1a8 }

      , "IngameData"  : { "world_area"                  : 0xa0
                        , "area_data"                   : 0xa8
                        ,     "area_index"              : 0x40
                        , "area_level"                  : 0xcc
                        , "area_hash"                   : 0x104
                        , "local_player"                : 0x8d0
                        , "entity_list"                 : 0x980
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x988 }

      , "IngameUI"    : { "lefe_panel"                  : 0x560
                        , "right_panel"                 : 0x568
                        , "panel_flags"                 : 0x570
                        , "entity_list"                 : 0x648
                        ,     "root"                    : 0x470
                        ,     "count"                   : 0x478 }

      , "ServerData"  : { "player_data"                 : 0x9858
                        ,     "passive_skills"          : 0x180
                        ,     "player_class"            : 0x238
                        ,     "level"                   : 0x23c
                        ,     "refund_points_left"      : 0x240
                        ,     "skill_points_from_quest" : 0x244
                        ,     "ascendancy_skill_points" : 0x24c
                        , "league"                      : 0xb130
                        , "party_status"                : 0xb418
                        , "stash_tabs"                  : 0xb1d0
                        , "inventory_slots"             : 0xb848 }

      , "Entity"      : { "internal"                    : 0x8
                        ,     "path"                    : 0x8
                        , "component_list"              : 0x10
                        , "id"                          : 0x80 }

      , "Element"     : { "self"                        : 0x28
                        , "childs"                      : 0x68
                        , "root"                        : 0xb8
                        , "parent"                      : 0x148
                        , "position"                    : 0xc0
                        , "scale"                       : 0xd8
                        , "is_visible"                  : 0x151
                        , "is_enabled"                  : 0x155
                        , "size"                        : 0x198
                        , "highlighted"                 : 0x1d8
                        , "text"                        : 0x310
                        , "item"                        : 0x390 }

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
