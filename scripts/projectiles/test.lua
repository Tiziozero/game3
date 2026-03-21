-- scripts/projectiles/test.lua
local M = {}

function M.init(self, owner,ref)
    print("=== element init called ===");
    self.owner = owner
    self.ref = ref
    -- called once when spawned
end

function M.set_handle(self, handle)
    self.handle = handle
end
function M.update(self)
    if not self.i then
        self.i = 0
    end
    if not self.handle then
        print("NO HANDLE");
        engine.panic();
        return
    end
    print("UPDATE CALLED!");
    if self.i > 10 then
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
    -- when projectile hits entity
end

function M.remove(self)
    -- cleanup
end

return M
