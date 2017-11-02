function vfn(name) return "game_assets/vox/" .. name .. ".vox" end

voxel_models = {
    cryobed = { model=vfn("cryobed32"), id=1 },
    floorlight = { model=vfn("floorlight"), id=2 },
    rtg = { model=vfn("rtg"), id=3 },
    cabinet = { model=vfn("cabinet"), id=4 },
    battery = { model=vfn("battery"), id=5 },
    cordex = { model=vfn("cordex"), id=6 },
    replicator_small = { model=vfn("replicator-small"), id=7 },
    solar_panel = { model=vfn("solarpanel"), id=8 },
    wood_logs = { model=vfn("woodlogs"), id=9 },
    turret = { model=vfn("turret"), id=10 },
    axe = { model=vfn("simpleaxe"), id=11 },
    campfire_kit = { model=vfn("crate-fakefire"), id=12 },
    tent_kit = { model=vfn("crate-tent"), id=13 },
    fakefire = { model=vfn("campfire-fake"), id=14 },
    tent = { model=vfn("tent"), id=15 }
}

function voxelId(name)
    if voxel_models[name] ~= nil then
        voxid = voxel_models[name].id
        print(name .. " : " .. voxid)
        return voxid
    else
        print("Unknown model: "..name)
    end
    return 0
end
