#include "../../game.h"
#include "../../abilities.h"

typedef struct {
    int owner_handle, handle;
    vec2 origin, pos, dir;
    float damage, damage_mltp, range, speed, r;
    double cooldown, cooldown_time;
    Color color;
} proj_test_payload;

int big_boy_boom_init(game * game, int owner_handle, element* e,
        vec2 pos, vec2 dir, float r, float speed);
// update feature
int big_boy_boom_update(game* game, int handle, void* payload) {
    if (!payload) {
        panic("No payload");
        return 0;
    }
    proj_test_payload* self = (proj_test_payload*)payload;
    if (self->cooldown > 0) {
        dbg("cooldown %f.", self->cooldown);
        return 1; // valid
    }
    self->cooldown = self->cooldown_time;
    entity* owner = find_entity(game,self->owner_handle);
    if (!owner) {
        panic("No owner/invalid handle. (projectile big boy)");
        return 0;
    }
    float dt = (float)game->dt;
    vec2 pos1 = self->pos;
    vec2 pos2 = vec2add(self->pos, vec2scale(self->dir, self->speed*dt));

    int count;
    collisions_ret_t* cols = entities_line_intersect(pos1,pos2,self->r, &count);
    if (!cols) {
        panic("Failed to get collisions");
        return 0;
    }
    int _handle = -1;
    float distance = 0;
    for (int i = 0; i < count; i++) {
        collisions_ret_t c = cols[i];
        if (c.handle == self->owner_handle) continue;
        if (_handle==-1) {
            _handle = c.handle;
            distance = c.distance;
        } else {
            if (c.distance < distance) {
                _handle = c.handle;
                distance = c.distance;
            }
        }
    }
    int remove = 0;
    if (_handle != -1) {
        info("Damaging for %f", (owner->atk + self->damage)*self->damage_mltp);
        damage_target((owner->atk + self->damage)*self->damage_mltp,
                find_entity(game, _handle));
        remove = 1;
    } else {
        if (vec2dist(self->origin, pos2) >self->range){ 
            /*dbg("range. %f %f %f %f %f %f", self->range,
                    self->origin.x, self->origin.y, 
                    pos2.x, pos2.y,
                    vec2dist(self->origin, pos2)
                    );*/
            remove = 1;
        }
    }
    if (remove) {
        // dbg("removing element %d.", self->handle);
        remove_element(game, self->handle);
        return 1;
    }
    // update pos and element pos
    self->pos = pos2;
    get_element(game, self->handle)->pos = self->pos;


    return 1;
}

int big_boy_boom_draw(game* game, int handle, void* payload) {
    if (!payload) {
        panic("No payload");
        return 0;
    }
    proj_test_payload* self = (proj_test_payload*)payload;
    vec2 draw = apply_camera(self->pos, *game->camera);

    DrawCircleV(draw, self->r,self->color);
    return 1;
}
int big_boy_boom_init(game * game, int owner_handle, element* e,
        vec2 pos, vec2 dir, float r, float speed) {
    proj_test_payload* ret = calloc(1,
            sizeof(proj_test_payload));
    if (!ret) panic("Failed to allocate memory");
    proj_test_payload p = {0};
    p.owner_handle = owner_handle;
    p.range = 500.0f;
    p.damage = 20;
    p.speed = speed;
    p.damage_mltp = 4.0f;
    p.color = GetColor(0xffcccccc);
    p.pos = pos;
    p.origin = pos;
    p.dir = vec2norm(dir);
    p.r = r;
    p.handle = e->handle;
    *ret = p;
    e->payload = (void*)ret;
    e->update = big_boy_boom_update;
    e->draw = big_boy_boom_draw;
    e->pos = p.pos;
    return 1;
}

