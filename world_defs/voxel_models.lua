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
    tent = { model=vfn("tent"), id=15 },
    sawmill = { model=vfn("sawmill"), id=16 },
    block = { model=vfn("block"), id=17 },
    woodwaste = { model=vfn("wastewood"), id=18 },
    bonecarver = { model=vfn("bonecarver"), id=19 },
    butcher = { model=vfn("butcher"), id=20 },
    carpenter = { model=vfn("carpenter"), id=21 },
    chemist = { model=vfn("chemist"), id=22 },
    stairs_up = { model=vfn("stairs-up"), id=23 },
    stairs_down = { model=vfn("stairs-down"), id=24 },
    stairs_updown = { model=vfn("stairs-updown"), id=25 },
    tailor = { model=vfn("tailor"), id=26 },
    forge = { model=vfn("forge"), id=27 },
    pick = { model=vfn("simplepick"), id=28 },
    boulder = { model=vfn("boulder"), id=29 },
    ore = { model=vfn("ore"), id=30 },
    powder = { model=vfn("powder"), id=31 },
    oil = { model=vfn("oil"), id=32 },
    stonecutter = { model=vfn("stonecutter"), id=33 },
    smelter = { model=vfn("smelter"), id=34 },
    deer = { model=vfn("deer"), id=35 },
    horse = { model=vfn("horse"), id=36 },
    armadillo = { model=vfn("armadillo"), id=37 },
    badger = { model=vfn("badger"), id=38 },
    kiln = { model=vfn("kiln"), id=39 },
    charcoal_hut = { model=vfn("charcoal_hut"), id=40 },
    still = { model=vfn("still"), id=41 },
    loom = { model=vfn("loom"), id=42 },
    glass_furnace = { model=vfn("glass_furnace"), id=43 },
    mason = { model=vfn("mason"), id=44 },
    mechanic = { model=vfn("mechanic"), id=45 },
    tanner = { model=vfn("tanner"), id=46 },
    charcoal = { model=vfn("charcoal"), id=47 },
    ant = { model=vfn("ant"), id=48 },

    person_base = { model=vfn("person_base"), id=49 },
    person_hair_short = { model=vfn("person_hair_short"), id=50 },
    person_hair_long = { model=vfn("person_hair_long"), id=51 },
    person_hair_pigtails = { model=vfn("person_hair_pigtails"), id=52 },
    person_hair_mohawk = { model=vfn("person_hair_mohawk"), id=53 },
    person_hair_balding = { model=vfn("person_hair_balding"), id=54 },
    person_hair_triangle = { model=vfn("person_hair_triangle"), id=55 },

    clothes_batman_hat = { model=vfn("clothes_batman_hat"), id=56 },
    clothes_police_hat = { model=vfn("clothes_police_hat"), id=57 },
    clothes_cat_ears = { model=vfn("clothes_cat_ears"), id=58 },
    clothes_tiara = { model=vfn("clothes_tiara"), id=59 },
    clothes_formal_shirt = { model=vfn("clothes_formal_shirt"), id=60 },
    clothes_bathrobe = { model=vfn("clothes_bathrobe"), id=61 },
    clothes_spandex_shirt = { model=vfn("clothes_spandex_shirt"), id=62 },
    clothes_sports_shirt = { model=vfn("clothes_sports_shirt"), id=63 },
    clothes_tshirt = { model=vfn("clothes_tshirt"), id=64 },
    clothes_cargopants = { model=vfn("clothes_cargopants"), id=65 },
    clothes_longskirt = { model=vfn("clothes_longskirt"), id=66 },
    clothes_hawaiian_shorts = { model=vfn("clothes_hawaiian_shorts"), id=67 },
    clothes_spandex_pants = { model=vfn("clothes_spandex_pants"), id=68 },
    clothes_miniskirt = { model=vfn("clothes_miniskirt"), id=69 },
    clothes_shoes = { model=vfn("clothes_shoes"), id=70 },
    clothes_boots = { model=vfn("clothes_boots"), id=71 },
    clothes_simple_tunic = { model=vfn("clothes_simple_tunic"), id=72 },
    clothes_britches_simple = { model=vfn("clothes_britches_simple"), id=73 },
    clothes_cap_simple = { model=vfn("clothes_cap_simple"), id=74 },

    ground_helmet = { model=vfn("ground_helmet"), id=75 },
    ground_shirt = { model=vfn("ground_shirt"), id=76 },
    ground_pants = { model=vfn("ground_pants"), id=77 },
    ground_shoes = { model=vfn("ground_shoes"), id=78 },

    meat = { model=vfn("meat"), id=79 },
    foodplate = { model=vfn("foodplate"), id=80 },
    liquid = { model=vfn("liquid"), id=81 },
    veggie = { model=vfn("veggie"), id=82 },

    table = { model=vfn("table"), id=83 },
    chair = { model=vfn("chair"), id=84 },
    door_ground = { model=vfn("door_ground"), id=85 },
    bed = { model=vfn("bed"), id=86 },
    axeblade = { model=vfn("axeblade"), id=87 },
    lathe = { model=vfn("lathe"), id=88 },
    hay = { model=vfn("hay"), id=89 },
    thread = { model=vfn("thread"), id=90 },
    cloth = { model=vfn("cloth"), id=91 },
    paper = { model=vfn("paper"), id=92 },
    dung = { model=vfn("dung"), id=93 },
    mechanism = { model=vfn("mechanism"), id=94 },
    circuit = { model=vfn("circuit"), id=95 },
    bone = { model=vfn("bone"), id=96 },
    hide = { model=vfn("hide"), id=97 },
    skull = { model=vfn("skull"), id=98 },
    cage = { model=vfn("cage"), id=99 },
    club = { model=vfn("club"), id=100 },
    club_spiked = { model=vfn("club_spiked"), id=101 },
    pointystick = { model=vfn("pointystick"), id=102 },
    hammer = { model=vfn("hammer"), id=103 },
    sword = { model=vfn("sword"), id=104 },
    knife = { model=vfn("knife"), id=105 },
    atlatl = { model=vfn("atlatl"), id=106 },
    bow = { model=vfn("bow"), id=107 },
    arrow = { model=vfn("arrow"), id=108 },
    crossbow = { model=vfn("crossbow"), id=109 },

    corpse = { model=vfn("corpse"), id=110 },

    pick_held = { model=vfn("simplepick_held"), id=111 },
    axe_held = { model=vfn("simpleaxe_held"), id=112 },
    hoe = { model=vfn("hoe"), id=113 },
    hoe_held = { model=vfn("hoe_held"), id=113 },

    dung_heap = { model=vfn("dung_heap"), id=114 },
    seed = { model=vfn("seed"), id=115 },
    raised_flower_bed = { model=vfn("raised_flower_bed"), id=116 },
    wall = { model=vfn("wall"), id=117 },
    floor = { model=vfn("floor"), id=118 },
    ramp = { model=vfn("ramp"), id=119 },
    carpenter_intermediate = { model=vfn("carpenter_intermediate"), id=120 },
    nitrogen_extractor = { model=vfn("nitrogen_extractor"), id=121 },
    leatherworker = { model=vfn("leatherworker"), id=122 },
    refinery = { model=vfn("refinery"), id=123 },

    lever_on = { model=vfn("lever-on"), id=124 },
    lever_off = { model=vfn("lever-off"), id=125 },
    pressure_plate = { model=vfn("pressureplate"), id=126 },
    energy_door_open = { model=vfn("energydoor-open"), id=127 },
    energy_door_closed = { model=vfn("energydoor-closed"), id=128 },
    spikes_retracted = { model=vfn("spikes-retracted"), id=129 },
    spikes_extended = { model=vfn("spikes-extended"), id=130 },
    blades_retracted = { model=vfn("blades-retracted"), id=131 },
    blades_extended = { model=vfn("blades-extended"), id=132 },
    stonefall = { model=vfn("stonefall"), id=133 },
    door_open = { model=vfn("door_open"), id=134 },
    floodgate_open = { model=vfn("floodgate_open"), id=135 },
    floodgate_closed = { model=vfn("floodgate_closed"), id=136 },
    floodgate_ground = { model=vfn("floodgate_ground"), id=137 },
    float_gauge = { model=vfn("float_gauge"), id=138 },
    proximity_sensor = { model=vfn("proximity_sensor"), id=139 },
    oscillator = { model=vfn("oscillator"), id=140 },
    gate_and = { model=vfn("gate_and"), id=141 },
    gate_or = { model=vfn("gate_or"), id=142 },
    gate_not = { model=vfn("gate_not"), id=143 },
    silicon_refinery = { model=vfn("silicon_refinery"), id=144 },
    gate_nand = { model=vfn("gate_nand"), id=145 },
    gate_nor = { model=vfn("gate_nor"), id=146 },
    gate_xor = { model=vfn("gate_xor"), id=147 },
    support = { model=vfn("support"), id=148 },
}

function voxelId(name)
    if voxel_models[name] ~= nil then
        voxid = voxel_models[name].id
        --print(name .. " : " .. voxid)
        return voxid
    else
        print("Unknown model: "..name)
    end
    return 0
end
