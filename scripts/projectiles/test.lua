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
    self.hits = {};
    print("init projectil3.");
end
function M.update(self)
    --[[ for key, value in pairs(self) do
        print("kv:", key, value)
    end ]]
    if not self.i then
        self.i = 0
    end
    if not self.handle then
        print("NO HANDLE");
        engine.panic();
        return
    end
    local dt =  engine.get_dt()
    local x1, y1 = self.x, self.y
    local x2 = self.x + self.dx * self.speed * dt
    local y2 = self.y + self.dy * self.speed * dt


    local hits = engine.entities_line_intersect(self.handle,x1,y1,x2,y2,self.r);
    local min, eh = -1,-1;
    for _,entry in ipairs(hits) do
        if entry.handle ~= self.owner then
            -- print("Hit:", entry.handle, "distance", entry.distance)
            if min == -1 then
                min = entry.distance; eh = entry.handle
            end
            if entry.distance < min then
                min = entry.distance
                eh = entry.handle
            end
        end
    end
    self.x, self.y = x2, y2
    engine.element_set_xy(self.handle, self.x, self.y);
    if eh ~= -1 then
        print("HITTING EH", eh);
        print("REMOVING", self.handle);;
        engine.entity_damage(eh, 15);
        engine.remove_element(self.handle)
    end
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
