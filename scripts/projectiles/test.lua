-- scripts/projectiles/test.lua
local M = {}
local utils = require("utils")

function M.init(self, owner,ref)
    print("=== element init called ===");
    self.owner = owner
    self.ref = ref
    self.x = 0;
    self.y = 0;
    -- called once when spawned
end
function M.init_projectile(self, x, y, dx, dy, speed, r, range, element_handle)
    self.x = x;
    self.y = y;
    self.originx = x;
    self.originy = y;
    self.dx = dx;
    self.dy = dy;
    self.r = r;
    self.range = range;
    self.speed = speed
    self.handle = element_handle;
    print("init projectil3.");
end
function M.update(self)
    for key, value in pairs(self) do
        -- print("kv:", key, value)
    end
    if not self.i then
        self.i = 0
    end
    if not self.handle then
        print("NO HANDLE");
        engine.panic();
        return
    end
    local dt =  engine.get_dt()
    local oldx, oldy = self.x, self.y
    self.x = self.x + self.dx * self.speed * dt
    self.y = self.y + self.dy * self.speed * dt
    engine.element_set_xy(self.handle, self.x, self.y);
    -- check distance
    if math.sqrt((self.x-self.originx)^2 +
        (self.y-self.originy)^2) >= self.range then
        print("REMOVING", self.handle);;
        engine.remove_element(self.handle)
    end
    self.i = self.i + 1;
    -- per-frame movement, collision checks
end

function M.onhit(self, target)
    -- when projectile hits entity
end
function M.draw(self, target)
    -- print("DRAWING")
    local x = self.x;
    local y = self.y;
    local rad = 5;
    -- print("x y:", x, y);
    local x1, y1 = engine.apply_camera(x,y)
    engine.draw_circle(x1,y1,rad,0xff,0xff,0xff,0xff);
    -- when projectile hits entity
end

function M.remove(self)
    -- cleanup
end

return M
