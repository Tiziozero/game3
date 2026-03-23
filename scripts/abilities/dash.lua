local M = {}

local utils = require("utils")

function M.init(self, owner, ref)
    print("INIT CALLED")
    -- set base stats
    self.handle = owner;
    self.ref = ref; -- to self
    print("owner handle ref:", owner, ref);
    self.range = 400;
    self.name = "test dash ability"
    self.desc = "test ability to test dash"
    self.icon = get_env("ICON3");
    self.cooldown_time = 3;
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
    if dx*dx + dy*dy > self.range^2 then
        local m = math.sqrt(dx*dx + dy*dy);
        dx = dx/m*self.range;
        dy = dy/m*self.range;
    end
    print("owner, dx dy",self.handle, owner.x +dx, owner.y+dy);
    engine.set_entity_pos(self.handle, owner.x+dx, owner.y+dy)
end
-- decrement
function M.update(self)
    self.cooldown = self.cooldown - engine.get_dt();
    if self.cooldown <= 0 then
        self.cooldown = 0;
    end
end


return M
