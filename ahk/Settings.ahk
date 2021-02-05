; Hotkeys (^ for Ctrl, # for window key)
;
; `,    Logout
; a,	Auto pickup/levelup gems
; F1,   Auto aruas
; F2,	Auto portal
; F3,   Auto identify/sell
; !F3,  Identify all and sell
; F4,   Auto stash
; F5,   Hideout
; ^F5,  Azurite mine
; !F5   Menagerie
; F6    Dump inventory items
; ^F6   Dump highlighted or all items from current stash tab
; F7    Sell full rare sets to vendor
; F12   Toggle log window
; LAlt  Hold down the left Alt key to show price of items
; #d,   Minimize window
; ^m,   Toggle maphack
; ^r,   Reload script
; ^q,   Quit

; Plugins
global PluginOptions := { "AutoFlask"     : { "enabled" : true}
                        , "AutoOpen"      : { "enabled" : true, "range" : 15 }
                        , "AutoPickup"    : { "enabled" : true, "range" : 50
                                            , "genericItemFilter" : "Blueprint|Contract|Incubator|Scarab$|Quicksilver|Basalt|Quartz|(Divine|Eternal) Life"
                                            , "rareItemFilter"    : "Jewel|Amulet|Ring|Belt" }
                        , "HeistChest"    : { "enabled" : true}
                        , "KillCounter"   : { "enabled" : true, "radius" : 50 }
                        , "MinimapSymbol" : { "enabled" : true
                                            , "showMonsters"       : true
                                            , "showMinions"        : true
                                            , "showCorpses"        : false
                                            , "rarity"             : 1
                                            , "showDelveChests"    : true
                                            , "size"               : 8
                                            , "borderColor"        : 0xffffff
                                            , "ignoredDelveChests" : "Armour|Weapon|Generic|NoDrops|Encounter" }
                        , "PlayerStatus"  : { "enabled" : true
                                            , "autoQuitThresholdPercentage" : 20
                                            , "autoQuitMinLevel"   : 90 } }

; Canvas, doesn't support full screen mode
global EnableCanvas := true
global ShowCanvasBorder := false
global EnableHud := true

; Banner, doesn't support full screen mode
global EnableBanner := true
global ShowEnergyShield := false
global ShowDebugMenu := false

; Log
global ShowLogger := false
global LogLevel := 0
global ShowLogMessage := false

; Flasks
global LifeThreshold := 80
global ManaThreshold := 30
global ChargesPerUseLimit := 30
global MonsterThreshold := 10
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
global VendorTabFullRareSets := "3"

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
;     5. All weapon/armour items whose item level is between 60 to 75
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
;       Weapon, Quiver, Armour, Belt, Amulet, Ring, Jewel
;
; Supported constraints:
;       index, name, baseName, isIdentified, isMirrored, isCorrupted, isRGB
;       rarity, itemLevel, quality, sockets, links, tier, level, price
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
                     , {"tabName" : "2xChaos", "baseType" : "Weapon|Armour|Belt|Amulet|Ring", "Constraints" : {"rarity" : 2, "isIdentified" : false, "itemLevel" : [60, 100]}}
                     , {"tabName" : "Maps",    "baseType" : "Map(?!Fragment)"}
                     , {"tabName" : "Fr",      "baseType" : "MapFragment", "baseName" : "Splinter|Scarab$"} ]
