;
; Settings.ahk, 5/17/2022 5:31 PM
;

; Plugins
global PluginOptions := { "AutoFlask"     : { "enabled" : true }

                        , "AutoOpen"      : { "enabled" : true
                                            , "range"   : 5
                                            , "ignoredChests" : "Amphora"
                                            , "chest"   : true
                                            , "delveChestOnly" : true
                                            , "door"    : true }

                        , "AutoPickup"    : { "enabled" : true
                                            , "range"   : 75
                                            , "ignoreChests"      : false
                                            ; 0: unique items, RGB items and full rare set recipe items.
                                            ; 1: 6 sockets items and quality gems
                                            ; 2: influenced items with item level >= 82 and synthesised items
                                            ; 3 and above: currency items, 6 links items
                                            , "strictLevel"       : 0
                                            , "genericItemFilter" : "Sentinel$|Incubator|Quicksilver|Basalt|Quartz|Eternal (Life|Mana)"
                                            , "rareItemFilter"    : "Jewel|Amulet|Ring" }

                        , "KillCounter"   : { "enabled" : true
                                            , "radius"  : 50 }

                        , "MinimapSymbol" : { "enabled" : true
                                            , "showNPC"            : true
                                            , "showPlayer"         : true
                                            , "showMonsters"       : true
                                            , "showMinions"        : true
                                            , "showCorpses"        : false
                                            , "rarity"             : 0
                                            , "showDelveChests"    : true
                                            , "showHeistChests"    : true
                                            , "fontSize"           : 12
                                            , "minSize"            : 4
                                            , "showDamage"         : true
                                            , "minDamage"          : 100000
                                            ; 0: move up/down(negative speed);
                                            ; 1: also move left/right, with random speed;
                                            ; 2: all directions with random speed;
                                            ; others: same as 0;
                                            , "style"              : 1
                                            , "speedX"             : 0.5
                                            , "speedY"             : 1.0
                                            , "ignoredDelveChests" : "Armour|Weapon|Generic|NoDrops|Encounter"
                                            , "ignoredHeistChests" : "Armour|Weapons|Corrupted|Gems|Jewellery|Jewels|QualityCurrency|Talisman|Trinkets|Uniques" }

                        , "PlayerStatus"  : { "enabled" : true
                                            , "autoQuitThresholdPercentage" : 20
                                            , "autoQuitMinLevel" : 90 } }

; Flasks
global LifeThreshold := 80
global ManaThreshold := 50
global ChargesPerUseLimit := 50
global MonsterThreshold := 5
global AlwaysRunning := true

; Trader
global TraderUICompact := true
global TraderUITransparent := 225
global TraderMaxSessions := 3
global TraderTimeout := 60
global TraderMessages := { "thanks" : "t4t, gl."
                         , "1sec"   : "1 sec."
                         , "ask"    : "Hi, are you still interested in {} for {}?"
                         , "sold"   : "sold out." }

; Vendor recipes
global VendorTabDivinationCards := "1"
global VendorTabGems := "2"
global VendorTabFullRareSets := "1, 2, 3"

; Attack and defense
global AttackSkillKey := "w"
global SecondaryAttackSkillKey := ""
global DefenseBuffSkillKey := "r"
global QuickDefenseKey := "q"
global QuickDefenseAction := "qe2345"
global AruasKey := "!q!w!e!r!t"

; Delve
global AutoDropFlare := true
global MaxDarknessStacks := 10

; Heist Chests
global HeistChestNameRegex := "HeistChest(Secondary|RewardRoom)(.*)(Military|Robot|Science|Thug)"

; Auto pickup
; Some items are picked up by default, includes:
;     1. all currency items, divination cards and map items
;     2. unique items
;     3. 6 sockets, 6 linked or 3 linked R-G-B items
;     4. gems whose quality > 5 or level > 12
;     5. All weapon/armour items whose item level are between 60 to 75
;     6. Influenced items
;
global AutoPickupKey := "a"

; Auto identify/sell/stash rules
; Rules Syntax:
;       [ { "baseType" : <RegEx>
;         , "baseName" : <RegEx>
;         , "Constraints" : { <property>" : <value>|[minValue, maxValue]
;                           , ...}}
;       , ... ]
; 
; Supported base types:
;       Currency, DivinationCard, Flask, Gem, Map, MapFragment, Prophecy,
;       Weapon, Quiver, Armour, Belt, Amulet, Ring, Jewel, Contract, Blueprint
;
; Supported constraints:
;       index, name, baseName, isIdentified, isMirrored, isCorrupted, isRGB
;       rarity, itemLevel, quality, sockets, links, tier, level, price, job
;       and is<BaseType> is<SubType>
;
global IdentifyExceptions :=[ {"baseType" : "Map"},
                            , {"baseName" : "Opal Ring|Two-Toned Boots"}
                            , {"baseType" : "Weapon|Armour|Belt|Amulet|Ring", "Constraints" : {"rarity" : 2, "isIdentified" : false, "itemLevel" : [60, 75]}} ]

global VendorRules := [ {"baseType" : "Gem", "Constraints" : {"baseName" : "^(?!Awakened)", "level" : [1, 18], "quality" : [0, 4]}}
                      , {"baseType" : "Weapon|Armour|Belt|Amulet|Ring|Quiver|Flask|Jewel"} ]
global VendorExceptions := [ {"baseType" : ".*", "Constraints" : {"rarity" : 3, "price" : [0.5, 99999]}}
                           , {"baseType" : "Currency|Map|MapFragment"}
                           , {"baseType" : "Gem", "Constraints" : {"baseName" : "Awakened"}}
                           , {"baseName" : "Blueprint|Contract|Cluster Jewel|Opal Ring|Two-Toned Boots"}
                           , {"baseType" : "Flask", "Constraints" : {"name" : "Bubbling|Seething|Catalysed|Staunching|Heat|Warding"}}
                           , {"baseType" : "Weapon|Armour|Belt|Amulet|Ring", "Constraints" : {"rarity" : 2, "isIdentified" : false, "itemLevel" : [60, 75]}} ]

global StashRules := [ {"tabName" : "Es",      "baseName" : "Essence of|Remnant of"}
                     , {"tabName" : "Fossils", "baseName" : "Fossil$|Resonator$"}
                     , {"tabName" : "$$$",     "baseType" : "Currency", "Constraints" : {"index" : [3, 60] }}
                     , {"tabName" : "1",       "baseType" : "Weapon", "Constraints" : {"rarity" : 2, "isIdentified" : false, "itemLevel" : [60, 85]}}
                     , {"tabName" : "2",       "baseType" : "Armour", "Constraints" : {"rarity" : 2, "isIdentified" : false, "itemLevel" : [60, 85]}}
                     , {"tabName" : "3",       "baseType" : "Belt|Amulet|Ring", "Constraints" : {"rarity" : 2, "isIdentified" : false, "itemLevel" : [60, 85]}}
                     , {"tabName" : "Maps",    "baseType" : "Map(?!Fragment)"}
                     , {"tabName" : "Fr",      "baseType" : "MapFragment", "baseName" : "Splinter|Scarab$"} ]
