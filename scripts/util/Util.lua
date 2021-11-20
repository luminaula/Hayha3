local Util = {}


function Util.deadReckoning(current, last, timeLast, timeFuture)
    ret = current
    return ret
end

function Util.move(deltaX,deltaY)
    rdeltaX = deltaX / 2
    rdeltaY = deltaY / 2
    if(math.abs(deltaX) < 2) then
        rdeltaX = 0
    end
    if(math.abs(deltaY) < 2) then
        rdeltaY = 0
    end
    directionY = -1
    directionX = -1
    accumTime = 0

    step = 1

    dist = math.sqrt(deltaX * deltaX + deltaY * deltaY)

    if(dist > 250) then
        step = 2.5
    elseif(dist > 100) then
        step = 1.2
    end

    if(deltaX > 0) then
        rdeltaX = math.ceil(rdeltaX)
        directionX = 1
    else
        rdeltaX = math.floor(rdeltaX)
    end

    if(deltaY > 0) then
        rdeltaY = math.ceil(rdeltaY)
        directionY = 1
    else
        rdeltaY = math.floor(rdeltaY)
    end


    directionX = directionX * step
    directionY = directionY * step

    rdeltaX = math.abs(rdeltaX)
    rdeltaY = math.abs(rdeltaY)


    for i=1,rdeltaY do
        moveMouse(0,directionY,accumTime)
        accumTime = accumTime + lastTime/rdeltaY
    end
    accumTime = 0
    for i=1,rdeltaX do
        moveMouse(directionX,0,accumTime)
        accumTime = accumTime + lastTime/rdeltaX
    end
end

return Util