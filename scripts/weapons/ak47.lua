log('Ak47')

local clickingTime = 0
local noShoot = 0
local lastShot = 0
local recoilScript = "ak47"
local shot = false

local weapon = {}

package.path = package.path .. ";" .. getUtilPath()

local Util = require("Util")

function weapon.triggerbot(x)
    if curTime < noShoot then
        return
    end

    targetX = x.x + x.w * 0.5
    targetY = x.y + x.h * 0.5

    recoilPos = recoil(recoilScript)
    deltaX = targetX - cursor.x + recoilPos.x
    deltaY = targetY - cursor.y + recoilPos.y

    if math.abs(deltaX) < x.w * 0.4 and math.abs(deltaY) < x.h * 0.4 then
        lastShot = time()
        shot = true
        click(250000)
        return
    end

    --Keep Shooting
    if timeClicked() ~= 0 then
        click(50000)
    end

end

function weapon.aimbot(x)

    if x.obj_id < 2 then
        targetX = x.x + x.w * 0.5
        targetY = x.y + x.h * 0.25
    else
        targetX = x.x + x.w * 0.5
        targetY = x.y + x.h * 0.5
    end

    recoilPos = recoil(recoilScript)
    deltaX = targetX - cursor.x + recoilPos.x
    deltaY = targetY - cursor.y + recoilPos.y

    Util.move(deltaX,deltaY)



end

function weapon.recoil()
    clickTime = timeClicked()
    curTime = time()
    timeDelta = clickTime - clickingTime
    if clickTime == 0 and curTime - lastShot > 250000 and shot then
        timeDelta = 0
        shot = false
        resetRecoil(recoilScript)
        noShoot = curTime + 350000
        log("reset")
    end
        
    travel(recoilScript, timeDelta)
    clickingTime = clickTime
    return recoil(recoilScript)
end

function weapon.targetDistance()
    return 100
end

return weapon