; Hotkeys (^ for Ctrl, # for window key)
;
; `,    logout
; a,	auto pickup/levelup gems
; F1,   auto aruas
; F2,	auto portal
; F3,   auto identify/sell
; !F3,  identify all and sell
; F4,   auto stash
; F5,   hideout
; ^F5,  azurite mine
; #d,   minimize window
; ^m,   toggle maphack
; ^r,   reload script
; ^q,   quit

; Canvas, doesn't support full screen mode
global EnableCanvas := true
global ShowCanvasBorder := false

; Banner, doesn't support full screen mode
global EnableBanner := true
global ShowEnergyShield := false
global ShowDebugMenu := false

; Log
global LogLevel := 0
global ShowLogMessage := false

; Auto quit
global AutoQuitMinLevel := 90
global AutoQuitThreshold := 20

; Flasks
global LifeThreshold := 80
global ManaThreshold := 30
global ChargesPerUseLimit := 30
global MonsterThreshold := 10
global AlwaysRunning := true

; Stash and inventory
global CloseAllUIKey := "{Space}"
global InventoryKey := "f"

; Attack and defense
global AttackSkillKey := "w"
global DefenseBuffSkillKey := "r"
global QuickDefenseKey := "q"
global QuickDefenseAction := "qe2345"
global AruasKey := "!q!w!e!r!t"

; Delve Chests
global IgnoredChests := "Armour|Weapon|Generic|NoDrops|Encounter"

; Heist Chests
global HeistChestNameRegex := "HeistChest(Secondary|RewardRoom)(.*)(Military|Robot|Science|Thug)"

; Auto pickup
; Some items are picked up by default, includes:
;     1. all currency, divination card and map items
;     2. unique items
;     3. 6 sockets, 6 linked or 3 linked R-G-B items
;     4. gems whose quality > 5 or level > 12
;     5. All weapon/armour items whose item level is between 60 to 75
;
global AutoPickupKey := "a"
global AutoPickupRange := 50
global genericItemFilter := "Blueprint|Contract|Incubator|Scarab$|Quicksilver|Basalt|Quartz|(Divine|Eternal) Life"
global rareItemFilter := "Jewel|Amulet|Ring|Belt"

; Auto identify/sell/stash rules
; Rules Syntax:
;       [ { "baseType" : <RegEx>
;         , "baseName" : <RegEx>
;         , "Constraints" : { <property>" : <value>|[minValue, maxValue]
;                           , ...}}
;       , ... ]
; 
; Supported base types:
;       Currency|Weapon|Armour|Belt|Amulet|Ring|Jewel|Flask|DivinationCard
;
; Supported constraints:
;       index, name, baseName, isIdentified, isMirrored, isCorrupted, isRGB
;       rarity, itemLevel, quality, sockets, links, tier, level
;       and is<BaseType> is<SubType>
;
global IdentifyExceptions :=[ {"baseType" : "Map"},
                            , {"baseName" : "Opal Ring|Two-Toned Boots"}
                            , {"baseType" : "Weapon|Armour|Belt|Amulet|Ring", "Constraints" : {"rarity" : 2, "isIdentified" : false, "itemLevel" : [60, 75]}} ]

global VendorRules := [ {"baseType" : "Gem", "Constraints" : {"baseName" : "^(?!Awakened)", "level" : [1, 18], "quality" : [0, 4]}}
                      , {"baseType" : "Weapon|Armour|Belt|Amulet|Ring|Quiver|Flask|Jewel"} ]
global VendorExceptions := [ {"baseType" : ".*", "Constraints" : {"rarity" : 3}}
                           , {"baseType" : "Currency|Map|MapFragment"}
                           , {"baseType" : "Gem", "Constraints" : {"baseName" : "Awakened"}}
                           , {"baseName" : "Blueprint|Contract|Cluster Jewel|Opal Ring|Two-Toned Boots"}
                           , {"baseType" : "Flask", "Constraints" : {"name" : "Bubbling|Seething|Catalysed|Staunching|Heat|Warding"}}
                           , {"baseType" : "Weapon|Armour|Belt|Amulet|Ring", "Constraints" : {"rarity" : 2, "isIdentified" : false, "itemLevel" : [60, 75]}} ]

global StashRules := [ {"tabName" : "Es",      "baseName" : "Essence of|Remnant of"}
                     , {"tabName" : "Fossils", "baseName" : "Fossil$|Resonator$"}
                     , {"tabName" : "$$$",     "baseType" : "Currency", "Constraints" : {"index" : [3, 60] }}
                     , {"tabName" : "Maps",    "baseType" : "Map(?!Fragment)"}
                     , {"tabName" : "Fr",      "baseType" : "MapFragment", "baseName" : "Splinter|Scarab$"} ]
