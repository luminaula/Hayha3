log('Ak47')

local clickingTime = 0
local noShoot = 0
local noMove = 0
local recoilScript = "ak47"

local weapon = {}

package.path = package.path .. ";" .. getUtilPath()

local Util = require("Util")


function weapon.triggerbot(x)

    targetX = x.x + x.w * 0.5
    targetY = x.y + x.h * 0.5

    recoilPos = recoil(recoilScript)
    deltaX = targetX - cursor.x + recoilPos.x
    deltaY = targetY - cursor.y + recoilPos.y

    if math.abs(deltaX) < x.w * 0.2 and math.abs(deltaY) < x.h * 0.2 then
        click(50000)

        return
    end

end

function weapon.aimbot(x)



    targetX = x.x + x.w * 0.5
    targetY = x.y + x.h * 0.5
    recoilPos = recoil(recoilScript)
    deltaX = targetX - cursor.x + recoilPos.x
    deltaY = targetY - cursor.y + recoilPos.y

    Util.move(deltaX,deltaY)

    
end

function weapon.recoil()
    clickTime = timeClicked()
    curTime = time()
    timeDelta = clickTime - clickingTime
    if timeDelta < 0 then
        timeDelta = 0
        noShoot = curTime + 250000
        resetRecoil(recoilScript)
    end
        
    travel(recoilScript, timeDelta)
    clickingTime = clickTime
    return recoil(recoilScript)
end

function weapon.targetDistance()
    return 100
end

return weapon