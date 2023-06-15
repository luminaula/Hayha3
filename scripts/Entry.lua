--[[
    --Uncomment to draw example black boxes before detection
clearIgnore()
ignore(0.35,0.60,0.15,0.5)
ignore(0.365,0.50,0.12,0.5)
ignore(0.4,0.40,0.05,0.15)
--]]




clearTeams()

newTeam()
targetID(0)
targetID(2)

newTeam()
targetID(1)
targetID(3)

clearRoutes()
readRouteFile("ak47.route")
readRouteFile("sg553.route")


strict(false)
clear()
listen(0xffe3)
setKeyboardRate(125)


log('reload')
setMouseRate(1000)
center(true)
local weapon = {}
local weapon2 = {}

package.path = package.path .. ";" .. getWeaponPath()

function loadWeapon(name)
    --setWeapon(name)
    --setRecoil(name)
    wpn = require(name)
    resetRecoil(name)
    return wpn
end

function loadWeapon2(name)
    wpn = require(name)
    return wpn
end


weapon2 = loadWeapon("ak47")
weapon = loadWeapon("tap")
setWeapon("tap")
function target ()


    closest = 0
    distance = weapon.targetDistance()

    recoilPos = weapon.recoil()

    for count=1,#detections do
        d = detections[count]
        middleX = d.x + d.w * 0.5 - cursor.x + recoilPos.x
        middleY = d.y + d.h * 0.5 - cursor.y + recoilPos.y
        tmpdist = math.sqrt(middleX * middleX + middleY * middleY)
        for count2=1,#tid do
            if d.obj_id == tid[count2] then
                if tmpdist < distance then
                    distance = tmpdist
                    closest = count
                end
            end
        end
    end
    return closest
end

function aim (x)
    --if checkKey(0xA0) or checkKey(0xA2) then

    weapon.triggerbot(x)
    weapon.aimbot(x)

        
    --end

   
end

function keyMain()
    if checkKey(0x50) then
        weapon = loadWeapon("awp")
        setWeapon("awp")
        --setDetectionNetRatio(1.5)
        --setNetworkResolution(10,10)
    end
    if(checkKey(0x4C)) then
        weapon = loadWeapon("ak47")
        setWeapon("ak47")
        --setDetectionNetRatio(1.0)
        --setNetworkResolution(6,8)
    end
    if(checkKey(0x4F)) then
        weapon = loadWeapon("tap")
        setWeapon("tap")
    end
end
