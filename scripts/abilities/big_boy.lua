local M = {}

local utils = require("utils")

function M.init(self, owner, ref)
    print("INIT CALLED")
    -- set base stats
    self.handle = owner;
    self.ref = ref; -- to self
    print("owner handle ref:", owner, ref);
    self.base_damage = 240;
    self.range = 800;
    self.name = "BigBoy"
    self.desc = "Same is the guy that hit Japan. Now in orange!"
    self.icon = get_env("ICON2");
    self.cooldown_time = 10;
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
        r = 30,
        dx = dx,
        dy = dy,
        speed = 200,
        range = 300,
        dmg = self.base_damage,
        projectile_path = "scripts/projectiles/big_boy.lua"
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
