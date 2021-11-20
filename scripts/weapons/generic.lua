log('Ak47')

local clickingTime = 0
local noShoot = 0
local noMove = 0

local weapon = {}

package.path = package.path .. ";" .. getUtilPath()

local Util = require("Util")


function weapon.triggerbot(x)

    targetX = x.x + x.w * 0.5
    targetY = x.y + x.h * 0.5

    deltaX = targetX - cursor.x
    deltaY = targetY - cursor.y

    if math.abs(deltaX) < x.w * 0.5 and math.abs(deltaY) < x.h * 0.5 then
        click(1)
    end

end

function weapon.aimbot(x)

    targetX = x.x + x.w * 0.5
    targetY = x.y + x.h * 0.5
    deltaX = targetX - cursor.x
    deltaY = targetY - cursor.y

    Util.move(deltaX,deltaY)

    
end

function weapon.recoil()
    pos.x = 0
    pos.y = 0
    return recoil(pos)
end

function weapon.targetDistance()
    return 9000
end

return weapon