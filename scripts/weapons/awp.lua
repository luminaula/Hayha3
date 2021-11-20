log('awp')

local clickingTime = 0
local noShoot = 0
local recoilPos = nil
local recoilScript = "ak47"

local weapon = {}

package.path = package.path .. ";" .. getUtilPath()

local Util = require("Util")

function weapon.triggerbot(x)
    if noShoot > time() then
        return
    end
    targetX = x.x + x.w * 0.5
    targetY = x.y + x.h * 0.35

    recoilPos = recoil(recoilScript)
    deltaX = targetX - cursor.x + recoilPos.x
    deltaY = targetY - cursor.y + recoilPos.y

    distance = math.sqrt(deltaX * deltaX + deltaY * deltaY)

    if distance < 30 then
        usleep(20000)
        click(50000)
        noShoot = time() + 500000
        return
    end

end

function weapon.aimbot(x)

    targetX = x.x + x.w * 0.5
    targetY = x.y + x.h * 0.33
    recoilPos = recoil(recoilScript)
    deltaX = targetX - cursor.x + recoilPos.x
    deltaY = targetY - cursor.y + recoilPos.y

    Util.move(deltaX,deltaY)
end

function weapon.recoil()
    return recoilCurrent()
end

function weapon.targetDistance()
    return 100
end


return weapon