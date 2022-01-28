;
; PoEOffsets.ahk 1/6/2021 10:45 AM
;

global PoEOffsets = { "version" : "3.16", "offsets"
    : { "GameStates"  : { "active_game_states"          : 0x20
                        , "all_game_states"             : 0x50
                        , "size"                        : 0x58 }

      , "IngameState" : { "in_game_ui"                  : 0x78
                        , "in_game_data"                : 0x18
                        ,     "server_data"             : 0x580
                        , "ui_root"                     : 0x4f0
                        , "hovered"                     : 0x528
                        , "hovered_item"                : 0x538
                        , "camera"                      : 0x868
                        , "width"                       : 0x870
                        , "height"                      : 0x874
                        , "matrix"                      : 0x928 }

      , "IngameData"  : { "world_area"                  : 0x80
                        , "area_data"                   : 0x88
                        ,     "area_index"              : 0x38
                        , "area_level"                  : 0xa0
                        , "area_hash"                   : 0x114
                        , "local_player"                : 0x588
                        , "entity_list"                 : 0x638
                        ,     "root"                    : 0x8
                        , "entity_list_count"           : 0x640 }

      , "IngameUI"    : { "inventory"                   : 0x5a8
                        ,     "grid"                    : 0x3d8
                        , "stash"                       : 0x5b0
                        ,     "tabs"                    : 0x2f0
                        , "overlay_map"                 : 0x6b0
                        ,     "large"                   : 0x278
                        ,     "small"                   : 0x280
                        , "chat"                        : 0x478
                        , "lefe_panel"                  : 0x580
                        , "right_panel"                 : 0x588
                        , "panel_flags"                 : 0x590
                        , "atlas"                       : 0x658
                        , "entity_list"                 : 0x6b8
                        ,     "root"                    : 0x2a8
                        ,     "count"                   : 0x2b0
                        , "gem_level_up"                : 0xa80
                        , "notifications"               : 0xa98 }

      , "ServerData"  : { "player_data"                 : 0x8660
                        ,     "passive_skills"          : 0x160
                        ,     "player_class"            : 0x200
                        ,     "level"                   : 0x204
                        ,     "skill_points_from_quest" : 0x20c
                        ,     "skill_points_left"       : 0x210
                        ,     "ascendancy_skill_points" : 0x214
                        , "league"                      : 0x87a8
                        , "party_status"                : 0x8a08
                        , "stash_tabs"                  : 0x8838
                        , "inventory_slots"             : 0x8c68 }

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
                        , "text"                        : 0x3a0 }

      , "StashTab"    : { "name"                        : 0x8
                        , "inventory_id"                : 0x28
                        , "type"                        : 0x34
                        , "index"                       : 0x38
                        , "folder_id"                   : 0x3a
                        , "flags"                       : 0x3d
                        , "is_affinity"                 : 0x3e
                        , "affinities"                  : 0x3f }

      , "Inventory"   : { "type"                    : 0x8
                        , "sub_type"                : 0xc
                        , "cols"                    : 0x14
                        , "rows"                    : 0x18
                        , "cells"                   : 0x38
                        , "count"                   : 0x58 } } }
