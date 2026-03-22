-- scripts/firebolt.lua

local firebolt = {}

function firebolt.on_spawn(self)
    self.speed = 900
    self.range = 400
end

function firebolt.on_update(self, dt)
    self.pos.x = self.pos.x + self.dir.x * self.speed * dt
end

function firebolt.on_hit(self, target)
    target:damage(40)
    self:destroy()
end

return firebolt
