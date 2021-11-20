


function target ()

    return 0
end

function aim (x)

end

function keyMain()
    if not training() then
        refreshTrainData()
        train(10)
    end
end