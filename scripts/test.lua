local M = {}

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
end

function M.act(self)
    print("act CALLED")
    if self.handle == 0 or not self.handle then
        print("no handle")
        return
    end
    print("handle", self.handle)
    local owner = engine.get_entity(self.handle);
    local x, y = engine.get_mouse_pos_world();
    local dx = x - owner.centerx;
    local dy = y - owner.centery;
    print("dx dy", dx, dy, self.base_damage, self.range, self.handle)
    local p = {
        owner = self.handle,
        x = owner.x,
        y = owner.y,
        dx = dx,
        dy = dy,
        speed = 800,
        range = self.range,
        dmg = self.base_damage,
        projectile_path = "scripts/projectiles/test.lua"
    }
    engine.new_projectile(p);
end

return M
