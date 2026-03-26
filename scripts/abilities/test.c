#include "../../game.h"
#include "../../abilities.h"

typedef struct {
    int owner_handle, index;
    float base_damage, range, speed;
    double cooldown, cooldown_time;
} ability_test_payload;

int ability_test_act(game* game, void* payload) {
    if (!payload) {
        panic("No payload");
    }
    ability_test_payload* self = (ability_test_payload*)payload;
    if (self->cooldown > 0) {
        // dbg("cooldown %f.", self->cooldown);
        return 1; // valid
    }
    self->cooldown = self->cooldown_time;
    entity* owner = find_entity(game,self->owner_handle);
    if (!owner) {
        panic("No owner/invalid handle.");
        return 0;
    }
    float radius = 2.0f;
    element* _e = game_alloc_element(game);
    if (!proj_test_init(game, self->owner_handle, _e,
        rect_center(owner->body),owner->direction, radius, self->speed)) {
        panic("Failed to init projectile");
        return 0;
    };
    return 1;
}
int test_update(game* game, void* payload) {
    ability_test_payload* p = payload;
    p->cooldown -= game->dt;
    if (p->cooldown <= 0) p->cooldown = 0;
    ability_test_payload* self = (ability_test_payload*)payload;
    ability* a = &find_entity(game, self->owner_handle)->abilities[self->index];
    a->cooldown_time = self->cooldown_time;
    a->cooldown = self->cooldown;
    return 1;
}
ability ability_init_test(game * game, int index, int owner_handle) {
    ability_test_payload* ret = calloc(1,
            sizeof(ability_test_payload));
    if (!ret) panic("Failed to allocate memory");
    ability_test_payload p = {0};
    p.owner_handle = owner_handle;
    p.index = index;
    p.range = 500.0f;
    p.speed = 800.0f;
    p.base_damage = 15.0f;
    p.cooldown = 0.0f;
    p.cooldown_time = 0.2f;
    *ret = p;
    ability i = {0};
    i.payload = (void*)ret;
    i.act = ability_test_act;
    i.description = "test ability to see how things work in c rather than lua.";
    i.name = "test ability";
    i.texture = LoadTexture(get_env("ICON4"));
    dbg("%d %d %d ability texture.", i.texture.width, i.texture.height, i.texture.id);
    i.update = test_update;
    i.draw = default_draw;
    i.cooldown = 0;
    i.cooldown_time = p.cooldown_time;
    return i;
}
