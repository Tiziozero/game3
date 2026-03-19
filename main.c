#include <iso646.h>
#include <stdio.h>
#include "raylib.h"
#include "defines.h"
#include "utils.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int player_id;

#define winh 750
#define winw 1200
typedef Vector2 vec2;
typedef Rectangle rect;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int load_env(const char *filename);

typedef enum {
    ability_kind_none,
    ability_kind_attack,
    attack_kind_arc,
} ability_kind;
dec_dynarr(void);
int dynarr_add(void* ptr, void* data) {
    dynarr_void* dynarr = ptr;
    if (dynarr->count >= dynarr->cap) {
        int new_cap = dynarr->cap*2;
        void** tmp = (void*)realloc(dynarr->data, new_cap*dynarr->size);
        if (!tmp) {
            panic("Failed to realloca dynar for size %d.",new_cap*dynarr->size);
            return 0;
        }
        dynarr->data = tmp;
    }
    TODO("Finish");
}

typedef struct {
    int owner; // index
    float range;
    float damage;
    enum { projectile_straight, projectile_arc } kind; // like an arrow or mortar
    union {
        struct {
            vec2 origin; // where it was shot from
            vec2 direction;
            vec2 position; // current position
            float area; // 0 if single enemy on impact
            float radius; // projactile radius. used for collisions
            float speed;
            Color color;
        } straight;
        struct {
            vec2 destination;
            float area; // 0 if single enemy
        } arc;
    };
    double start; // time at which it was shot;
    char active; // for game to see if it's still active
} projectile;
typedef struct {
    ability_kind attack_kind;
    union {
        projectile projectile;
    };
} entity_ability;

typedef struct {
    int id;
    Texture2D image;
    rect body; // pos/destination
    double atk, def, health; // def:multuplier for damage
    struct {
        rect source;
    } draw;
    entity_ability abilities[4]; // 4 attacks
} entity;
dec_dynarr(entity);

vec2 apply_camera(vec2 world_pos, rect camera) {
    return _vec(world_pos.x - camera.x,
        world_pos.y - camera.y);
}
vec2 unapply_camera(vec2 screen_pos, rect camera) {
    return _vec(screen_pos.x + camera.x,
                screen_pos.y + camera.y);
}
int draw_entity(entity* e, rect camera) {
    rect body = e->body;
    if (body.x > camera.x+camera.width || body.x + body.width < camera.x) return 1;// skip
    if (body.y > camera.y+camera.height || body.y + body.height < camera.y) return 1;// skip
    vec2 origin = _vec(e->body.width/2, e->body.height/2);
    float r = 0;
    vec2 draw = apply_camera(_vec(e->body.x+origin.x,e->body.y+origin.y) , camera);
    DrawTexturePro(e->image,
            _rect(0,0,e->image.width,e->image.height),
            _rect(draw.x, draw.y, e->body.width, e->body.height),
            origin,
            r, WHITE);
    draw = apply_camera(rect_pos(e->body), camera);
    DrawRectangleLines(draw.x, draw.y, e->body.width, e->body.height, RED);
    return 1;
}
int init_entity(entity* ent, float h, char* path) {
    static int id = 0;
    entity e;
    e.body.width = 0;
    e.body.height = 0;
    memset(&e,0, sizeof(entity));
    e.image = LoadTexture(path);
    e.body.x = 0;
    e.body.y = 0;
    float scaler =  h/e.image.height; // scale so that height is h
    e.body.width  = scaler*(float)e.image.width;
    e.body.height = scaler*(float)e.image.height;
    e.id = id++;
    dbg("%d: w:h %f:%f", e.id, e.body.width, e.body.height);
    *ent = e;
    return 1;
}
double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
void sort_entities_by_y(entity **arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j]->body.y+arr[j]->body.height/2 >
                    arr[j + 1]->body.y+arr[j+1]->body.height/2) {
                entity* temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}
entity* entities_new_entity(Arena* a, dynarr_entity* entities, float h, char* path) {
    entity* e = arena_alloc(a, sizeof(entity));
    init_entity(e, h, path);
    e->body.x = 0;
    e->body.y = 0;
    entities->data[entities->count++] = e;
    return e;
}
typedef struct {
    char active;
    enum{ element_projectile } kind;
    union {
        projectile projectile;
    }; 
} element; // like projectiles and what not
dec_dynarr(element);
typedef struct {
    dynarr_entity entities;
    vec2 mouse_pos;
    entity* player; // player
    element elements[100];
    int energy;
    int eps; // energy recovered per second.

    Arena cycle_arena; // arena that gets reset every cycle for temporary stuff

    Arena chat_arena; // arena for messages etc
    char** logs; // circular buffer
    int log_count; // at which to start
    int log_cap; // at which to start

    double dt;
    
    rect* camera;
} game;
int game_log(game*game, char*msg) {
    if (game->log_count >= game->log_cap) {
        game->logs = realloc(game->logs, (game->log_cap*=2)*sizeof(char*));
        dbg("realloced chat %zu msgs cap", game->log_cap);
    }
    game->logs[game->log_count++] = msg;
    return 1;
}
int game_new_element(
    element elements[100], element e){
    for (int i = 0; i < 100; i++) {
        if (elements[i].active == 0) {
            e.active = 1;
            elements[i] = e;
            dbg("added element.");
            return 1;
        }
    }
    return 0;
}

game* _getsetgame(int set, game* g) {
    static game* game = 0;
    if (set) {
        game = g;
    }
    return game;
}
game* setgame(game* game) {
    return _getsetgame(1, game);
}
game* getgame() {
    return _getsetgame(0, 0);
}

int attack(game* g, entity* entity, int atk_index) {
    entity_ability atk = entity->abilities[atk_index];
    switch (atk.attack_kind) {
        case ability_kind_attack:
            {
                projectile prjk = atk.projectile;
                switch (prjk.kind) {
                    case projectile_straight:
                        {
                            vec2 origin = rect_pos(entity->body);
                            // add half of body size to get center
                            origin = vec2add(origin,
                                    vec2scale(rect_size(entity->body), 0.5));
                            // use center of screen, idc
                            vec2 direction = vec2norm(vec2sub(g->mouse_pos,
                                        _vec((float)winw/2, (float)winh/2)));
                            dbg("attackdiredction %f %f radius %f range %f.",
                                    direction.x, direction.y,
                                    prjk.straight.radius, prjk.range);
                            projectile p = prjk;
                            p.straight.origin = origin;
                            p.straight.position = origin;
                            p.straight.direction = direction;
                            p.start = get_time();
                            p.owner = player_id; // player
                            element e = {0};
                            e.kind = element_projectile;
                            e.projectile = p;
                            if (!game_new_element(g->elements,e)) {
                                panic("failed to add projectile, likely full. handle.");
                                return 0;
                            }
                            dbg("new peojectile");
                        } break;
                    default: panic("unhandeled.");
                }
            } break;
        default: panic("unhandeled");
    }
    return 1;
}
bool entities_overlap(entity** entities, int count, int* out_a, int* out_b) {
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (CheckCollisionRecs(entities[i]->body, entities[j]->body)) {
                *out_a = i;
                *out_b = j;
                return true;
            }
        }
    }
    return false;
}
int update(game* game) {
    panic("todo");
    return 0;
}
float point_rect_dist(vec2 p, rect r) {
    // horizontal distance
    float dx = fmax(r.x - p.x, 0.0f);
    dx = fmax(dx, p.x - (r.x + r.width));
    
    // vertical distance
    float dy = fmax(r.y - p.y, 0.0f);
    dy = fmax(dy, p.y - (r.y + r.height));
    
    return sqrtf(dx*dx + dy*dy);
}
int projectile_body_hit(projectile* p, rect body, vec2 prev_pos);
int element_handle(game* g, element* e) {
    if (e->kind == element_projectile) {
        projectile* p = &e->projectile;
        if (p->kind == projectile_straight) {
            vec2 o = p->straight.origin;
            vec2 d = p->straight.direction;
            float s = p->straight.speed;
            float dt = (float)g->dt;
            vec2 pos = p->straight.position;
            vec2 prev = pos; // for intersection
            // change in displacement
            vec2 ds = vec2scale(vec2scale(d, s), dt);
            pos = vec2add(pos, ds);
            if (vec2len(vec2sub(pos, o)) >= p->range) {
                info("Over");
                e->active = 0; // set to inactive
                return 1;
            }
            p->straight.position = pos; // update pos
            float min_distance = 1e9; // why not
            int target_index = -1;
            int i = 0;
            DrawLineV(apply_camera(prev, *g->camera), apply_camera(pos, *g->camera), GREEN);
            while (i < g->entities.count) {
                entity* ent = g->entities.data[i];
                if (ent->id == p->owner) { i++; continue; } // skip
                if(projectile_body_hit(p, ent->body, prev)) {
                    rect b = ent->body;
                    dbg("%f %f %f %f (%f %f)\n",b.x,b.y,b.width,b.height,pos.x, pos.y);
                    // get shortest
                    float d = point_rect_dist(prev, ent->body);
                    if (d < min_distance) {
                        min_distance = d;
                        target_index = i;
                        break; // handle more
                    }
                }
                i++;
            }
            if (target_index != -1) { // if a hit
                printf("HIT %d (%fm).\n", target_index, min_distance/50);
                e->active = 0; // remove
            }
        } else  { panic("unhandeled element projectile kind."); return 0; }
    } else  { panic("unhandeled element kind."); return 0; }
    return 1;
}
bool point_in_rect(vec2 p, rect r) {
    return p.x >= r.x && p.x <= r.x + r.width &&
           p.y >= r.y && p.y <= r.y + r.height;
}
bool circle_rect_intersect(vec2 c, float r, rect rect) {
    float closest_x = fmax(rect.x, fmin(c.x, rect.x + rect.width));
    float closest_y = fmax(rect.y, fmin(c.y, rect.y + rect.height));

    float dx = c.x - closest_x;
    float dy = c.y - closest_y;

    return (dx*dx + dy*dy) <= r*r;
}

// helper: line segment intersection
bool line_intersect(vec2 p1, vec2 p2, vec2 q1, vec2 q2) {
    float s1_x = p2.x - p1.x;
    float s1_y = p2.y - p1.y;
    float s2_x = q2.x - q1.x;
    float s2_y = q2.y - q1.y;

    float s = (-s1_y * (p1.x - q1.x) + s1_x * (p1.y - q1.y)) /
              (-s2_x * s1_y + s1_x * s2_y);
    float t = ( s2_x * (p1.y - q1.y) - s2_y * (p1.x - q1.x)) /
              (-s2_x * s1_y + s1_x * s2_y);

    return s >= 0 && s <= 1 && t >= 0 && t <= 1;
}

bool line_rect_intersect(vec2 p1, vec2 p2, rect r) {
    // check endpoints
    if (point_in_rect(p1, r) || point_in_rect(p2, r)) return true;

    vec2 tl = {r.x, r.y};
    vec2 tr = {r.x + r.width, r.y};
    vec2 bl = {r.x, r.y + r.height};
    vec2 br = {r.x + r.width, r.y + r.height};

    // check each rectangle edge
    if (line_intersect(p1, p2, tl, tr)) return true;
    if (line_intersect(p1, p2, tr, br)) return true;
    if (line_intersect(p1, p2, br, bl)) return true;
    if (line_intersect(p1, p2, bl, tl)) return true;

    return false;
}
int player_ability(int ability, game* game) {
    if (ability == 0) {
    } else if (ability == 1) {
    } else if (ability == 2) {
    } else if (ability == 3) {
    } else if (ability == 4) {
    } else if (ability == 5) {
        attack(game, game->player, 0);
    } else {
        panic("unhandeled");
    }
    return 0;
}
// with updated position and prev
int projectile_body_hit(projectile* p, rect body, vec2 prev_pos) {
    if (p->kind == projectile_straight) {
        if (circle_rect_intersect(
                    p->straight.position, p->straight.radius, body)) {
            // projectile directly hits body
            return 1;
        }
        /*if (line_rect_intersect(p->straight.position, prev_pos, body)) {
            return 1; // projectile has interescted body some time in change in pos
        }*/
        return 0; // else no intersection
    } else  { panic("unhandeled."); return 0; }
    return 0;
}
int main(void) {
    load_env(".env");
    printf("Hello World!\n");
    InitWindow(winw, winh, "Hello Raylib");
    SetTargetFPS(60);
    game game;
    memset(&game, 0, sizeof(game));
    setgame(&game);

    Arena a = arena_new(1025, sizeof(entity));
    int ecount = 0; // entities count
    new_dynarr(entity, _entities);

    game.entities = _entities;
    // take reference to player
    entity* player = entities_new_entity(&a, &game.entities, 80.0f,getenv("PLAYER_PATH"));
    player_id = player->id;
    game.player = player;
    entity_ability* pa1 = &player->abilities[0];
    pa1->attack_kind = ability_kind_attack;
    pa1->projectile.kind=projectile_straight;
    pa1->projectile.damage=1.2*player->atk;
    pa1->projectile.range=400;
    pa1->projectile.straight.area=0; // one enemy
    pa1->projectile.straight.color = RED;
    pa1->projectile.straight.radius=2; // radius of 2;
    pa1->projectile.straight.speed= 1000;
    printf("Player :%s\n", getenv("PLAYER_PATH"));
    entity* enemy = entities_new_entity(&a, &game.entities, 90.0f,getenv("ENEMY1"));
    enemy->body.x = 200;
    enemy->body.y = 200;
    enemy = entities_new_entity(&a, &game.entities, 95.0f,getenv("ENEMY2"));
    enemy->body.x = -100;
    enemy->body.y = 150;
    enemy = entities_new_entity(&a, &game.entities, 100.0f,getenv("ENEMY3"));
    enemy->body.x = -100;
    enemy->body.y = -350;
    game.log_cap = 10;
    game.log_count = 0;
    game.logs = malloc(game.log_cap*sizeof(char*));



    info("Player index %d.", player_id);
    int draw_logs = 1;
    game.dt = 0;
    double last = get_time();
    rect camera;
    game.camera= &camera;
    vec2  player_dest;
    while (!WindowShouldClose()) {
        // dt
        double now = get_time();
        double dt = now - last;
        last = now;
        game.dt = dt;
        game.mouse_pos = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            player_dest = unapply_camera(game.mouse_pos, *game.camera);
            /* velocity, not ideal
             * vec2 dest = vec2scale(vec2norm(vec2sub(game.mouse_pos,
                        _vec((float)winw/2, (float)winh/2))), 200.0f*(float)dt);
            player->body.x += dest.x;
            player->body.y += dest.y;
            printf("Vec2 %f %f\n", dest.x, dest.y); */
        }
        int k = 0;
        while ((k = GetKeyPressed())) {
            if (k == KEY_SPACE) {
                attack(&game, player, 0);
            } else if (k == KEY_F) {
                game_log(&game, "Welp!");
            } else if (k == KEY_TAB) {
                draw_logs = !draw_logs;
            } else if (k == KEY_Q) {
                player_ability(0, &game);
            } else if (k == KEY_W) {
                player_ability(1, &game);
            } else if (k == KEY_E) {
                player_ability(2, &game);
            } else if (k == KEY_R) {
                player_ability(3, &game);
            } else if (k == KEY_D) {
                player_ability(4, &game);
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) // left clicownk
            player_ability(5, &game);

        camera.x = player->body.x + player->body.width/2 - (float)winw/2;
        camera.y = player->body.y + player->body.height/2 - (float)winh/2;
        camera.width = winw;
        camera.height= winh;
        BeginDrawing();
        ClearBackground(BLACK);
        sort_entities_by_y(game.entities.data, game.entities.count);
        for (int i = 0; i < game.entities.count; i++) {
            draw_entity(game.entities.data[i], camera);
        }
        for (int i = 0; i < 100; i++) {
            element* e = &game.elements[i];
            if (!e->active) continue;
            element_handle(&game, e);
            if (e->kind == element_projectile) {
                projectile p = e->projectile;
                float range = p.range;
                float r = p.straight.radius;
                Color c = p.straight.color;
                /* double from_start = now - p.start;
                   vec2 traveled = vec2scale(p.straight.direction,
                   from_start*p.straight.speed);
                   if (vec2len(traveled) >= p.range) {
                   p.active = 0;
                   continue;
                   } */
                vec2 pos = p.straight.position; // = vec2add(p.straight.origin, traveled);
                pos = apply_camera(pos, camera);
                DrawCircleV(pos, r, c);
            } else {
                panic("unknown elemen kind %d.", e->kind);
            }
        }
        if (draw_logs) {
            for (int i = game.log_count-1;
                    i >= game.log_count-20 && i >= 0; i--) { // max 20
                DrawText(game.logs[i], 0, 500 - 20*(game.log_count-i),20, WHITE);
            }
        }
        EndDrawing();
    }
    for (int i = 0; i < game.entities.count; i++) {
        UnloadTexture(game.entities.data[i]->image);
    }
    free(game.entities.data);
    game.entities.data = 0;
    arena_free(&a);
    arena_free(&game.cycle_arena);
    arena_free(&game.chat_arena);
    return 0;
}
