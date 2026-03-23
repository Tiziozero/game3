#include "../../game.h"

typedef struct {
    int handle;
    double cooldown, time;
} enemy_sweetie;

int sweetie_update(game* game, int handle, void* payload) {
    enemy_sweetie* p = payload;
    entity* player = game->player;
    entity* self = find_entity(game, handle);
    self->direction = vec2sub(rect_pos(player->body), rect_pos(self->body));
    if (p->cooldown <= 0) {
        entity_ability(game, handle, 0);
        p->cooldown = p->time;
    } else {
        p->cooldown -= game->dt;
    }
    return 1;
}
entity* entity_init_sweetie(game * game) {
    enemy_sweetie* ret = calloc(1,
            sizeof(enemy_sweetie));
    if (!ret) panic("Failed to allocate memory");
    enemy_sweetie p = {0};
    entity e;
    if (!init_entity(&e, 120.0f, get_env("ENEMY4"))) {
        panic("Failed to create entity?");
        return NULL;
    }
    p.cooldown = 0;
    p.time = 1.5f;
    *ret = p;
    e.payload = ret;
    e.atk = 200;
    e.health = 1500;
    e.max_health = 1500;
    e.def = 0;
    e.status = status_on;
    e.draw = basic_entity_draw;
    e.update = sweetie_update;
    entity* entity = game_new_entity(game, e);
    init_ability(game, e.handle, 0, ability_kind_test);
    return entity;
}

