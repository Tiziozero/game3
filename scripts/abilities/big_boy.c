#include "../../game.h"
#include "../../abilities.h"

typedef struct {
    int owner_handle;
    float base_damage, range, speed;
    double cooldown, cooldown_time;
} big_boy_payload;

int big_boy(game* game, void* payload) {
    if (!payload) {
        panic("No payload");
    }
    big_boy_payload* self = (big_boy_payload*)payload;
    if (self->cooldown > 0) {
        dbg("cooldown %f.", self->cooldown);
        return 1; // valid
    }
    self->cooldown = self->cooldown_time;
    entity* owner = find_entity(game,self->owner_handle);
    if (!owner) {
        panic("No owner/invalid handle.");
        return 0;
    }
    vec2 mpos = unapply_camera(game->mouse_pos,*game->camera);
    vec2 ds = vec2sub(mpos,rect_center(owner->body));
    float radius = 30.0f;
    element* _e = game_alloc_element(game);
    if (!big_boy_boom_init(game, self->owner_handle, _e,
        rect_center(owner->body),ds, radius, self->speed)) {
        panic("Failed to init projectile");
        return 0;
    };
    return 1;
}
int big_boy_ability_update(game* game, void* payload) {
    big_boy_payload* p = payload;
    p->cooldown -= game->dt;
    if (p->cooldown <= 0) p->cooldown = 0;
    return 1;
}
ability ability_init_big_boy(game * game, int owner_handle) {
    big_boy_payload* ret = calloc(1,
            sizeof(big_boy_payload));
    if (!ret) panic("Failed to allocate memory");
    big_boy_payload p = {0};
    p.owner_handle = owner_handle;
    p.range = 300.0f;
    p.speed = 500.0f;
    p.base_damage = 50.0f;
    p.cooldown = 0.0f;
    p.cooldown_time = 10.0f;
    *ret = p;
    ability i = {0};
    i.payload = (void*)ret;
    i.act = big_boy;
    i.description = "big boy already whiped a city once, he can do it again.";
    i.name = "big boy";
    i.texture = LoadTexture("imgs/icon6.png");
    i.update = big_boy_ability_update;
    i.draw = default_draw;
    i.cooldown = 0;
    i.cooldown_time = p.cooldown_time;
    return i;
}
