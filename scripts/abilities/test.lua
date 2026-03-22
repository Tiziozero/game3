local M = {}

local utils = require("utils")

function M.init(self, owner, ref)
    print("INIT CALLED")
    -- set base stats
    self.handle = owner;
    self.ref = ref; -- to self
    print("owner handle ref:", owner, ref);
    self.base_damage = 80;
    self.range = 800;
    self.name = "test ability"
    self.desc = "test ability does a whole bunch of stuff to figure out ho to do things"
    self.icon = "imgs/icon5.png"
    self.cooldown_time = 0.2;
    self.cooldown = 0.0;
end

function M.act(self)
    if self.handle == 0 or not self.handle then
        print("no handle")
        return
    end
    --
    if self.cooldown ~= 0 then
        print("cooldown", self.cooldown)
        return 0;
    end
    self.cooldown = self.cooldown_time;
    print("cooldown_time", self.cooldown_time);

    local owner = engine.get_entity(self.handle);
    local x, y = engine.get_mouse_pos_world();
    local dx = x - owner.centerx;
    local dy = y - owner.centery;
    --[[ print("x y dx dy ownerx y centerx y", x, y, dx, dy,
    owner.x, owner.y, owner.centerx, owner.centery,
    self.base_damage, self.range, self.handle)]]
    local radius = 10;
    local p = {
        owner = self.handle,
        x = owner.centerx,
        y = owner.centery,
        r = radius,
        dx = dx,
        dy = dy,
        speed = 800,
        range = 800,
        dmg = self.base_damage,
        projectile_path = "scripts/projectiles/test.lua"
    }
    engine.new_projectile(p);
end
-- decrement
function M.update(self)
    self.cooldown = self.cooldown - engine.get_dt();
    if self.cooldown <= 0 then
        self.cooldown = 0;
    end
end


return M
