#include "../../game.h"

typedef struct {
    int handle;
} basic_enemy;

int entity_act_basic_enemy(game* game, void* payload) {
    if (!payload) {
        panic("No payload");
    }
    basic_enemy* self = (basic_enemy*)payload;
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
    float radius = 2.0f;
    element* _e = game_alloc_element(game);
    if (!proj_test_init(game, self->owner_handle, _e,
        rect_center(owner->body),ds, radius, self->speed)) {
        panic("Failed to init projectile");
        return 0;
    };
    return 1;
}
int basic_enemy_update(game* game, void* payload) {
    basic_enemy* p = payload;
    p->cooldown -= game->dt;
    if (p->cooldown <= 0) p->cooldown = 0;
    return 1;
}
entity* entity_init_basic_enemy(game * game) {
    basic_enemy* ret = calloc(1,
            sizeof(basic_enemy));
    if (!ret) panic("Failed to allocate memory");
    basic_enemy p = {0};
    entity e;
    if (!init_entity(&e, 80.0f, getenv("ENEMY_BASIC"))) {
        panic("Failed to create entity?");
        return NULL;
    }
    return e;
}
