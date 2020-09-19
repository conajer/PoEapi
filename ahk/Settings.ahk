; Hotkeys (^ for Ctrl, # for window key)
;
; `,    logout
; a,	auto pickup
; F1,   cast aruas
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

; Attack and defense
global AttackSkillKey := "w"
global QuickDefenseKey := "q"
global QuickDefenseAction := "qe2345"
global AruasKey := "!q!w!e!r!t"

; Delve Chests
global IgnoredChests := "Armour|Weapon|Generic|NoDrops|Encounter"

; Auto pickup
; Some items are picked up by default, includes:
;     all currency, divination card and map items
;     unique items
;     6 sockets, 6 linked or 3 linked R-G-B items
;     gems whose quality > 5 or level > 12
;
global AutoPickupKey := "a"
global genericItemFilter := "Incubator|Scarab$|Quicksilver|Diamond|Basalt|Quartz"
global rareItemFilter := "Jewels|Amulet|Rings|Belts"
