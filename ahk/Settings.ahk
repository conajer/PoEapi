; Hotkeys (^ for Ctrl, # for window key)
;
; `,    logout
; a,	auto pickup
; F1,   auto aruas
; F2,	auto portal
; F5,   hideout
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
global AutoQuitThreshold := 20

; Flasks
global LifeThreshold := 80
global ManaThreshold := 30
global ChargesPerUseLimit := 30
global MonsterThreshold := 10
global AlwaysRunning := true

; Stash and inventory
global InventoryKey := "f"

; Attack and defense
global AttackSkillKey := "w"
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
;
global AutoPickupKey := "a"
global genericItemFilter := "Contract|Incubator|Scarab$|Quicksilver|Diamond|Basalt|Quartz"
global rareItemFilter := "Jewels|Amulet|Rings|Belts"
