#include "../../game.h"

typedef struct {
    int handle;
    double cooldown, time;
} basic_enemy;

int basic_enemy_update(game* game, int handle, void* payload) {
    basic_enemy* p = payload;
    entity* player = game->player;
    entity* self = find_entity(game, handle);
    self->direction = vec2sub(
            vec2add(vec2scale(rect_size(player->body),0.5),rect_pos(player->body)),
            vec2add(vec2scale(rect_size(self->body),0.5),rect_pos(self->body)));
    if (p->cooldown <= 0) {
        entity_ability(game, handle, 0);
        p->cooldown = p->time;
    } else {
        p->cooldown -= game->dt;
    }
    return 1;
}
entity* entity_init_basic_enemy(game * game) {
    basic_enemy* ret = calloc(1,
            sizeof(basic_enemy));
    if (!ret) panic("Failed to allocate memory");
    basic_enemy p = {0};
    entity e;
    if (!init_entity(&e, 80.0f, get_env("ENEMY1"))) {
        panic("Failed to create entity?");
        return NULL;
    }
    p.cooldown = 0;
    p.time = 1.5f;
    *ret = p;
    e.payload = ret;
    e.atk = 20;
    e.health = 150;
    e.max_health = 150;
    e.def = 0;
    e.status = status_on;
    e.draw = basic_entity_draw;
    e.update = basic_enemy_update;
    entity* entity = game_new_entity(game, e);
    init_ability(game, e.handle, 0, ability_kind_test);
    return entity;
}
