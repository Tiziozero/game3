#include <iso646.h>
#include <stdio.h>
#include "defines.h"
#include "raylib.h"
#include "raymath.h"
#include "utils.h"
#include <time.h>
#include <unistd.h>


#define winh 750
#define winw 1200
typedef Vector2 vec2;
typedef Rectangle rect;

typedef enum {
    attack_kind_none,
    attack_kind_projectile,
    attack_kind_arc,
} atk_kind;
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
    float range;
    float damage;
    enum { projectile_straight, projectile_arc } kind; // like an arrow or mortar
    union {
        struct {
            vec2 origin; // where it was shot from
            vec2 direction;
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
    atk_kind attack_kind;
    union {
        projectile projectile;
    };
} entity_attack;

typedef struct {
    int id;
    Texture2D image;
    rect body; // pos/destination
    double atk, def, health; // def:multuplier for damage
    struct {
        rect source;
    } draw;
    entity_attack attacks[4]; // 4 attacks
} entity;
dec_dynarr(entity);

vec2 apply_camera(vec2 world_pos, rect camera) {
    return _vec(world_pos.x - camera.x,
        world_pos.y - camera.y);
}
int draw_entity(entity* e, rect camera) {
    rect body = e->body;
    if (body.x > camera.x+camera.width || body.x + body.width < camera.x) return 1;// skip
    if (body.y > camera.y+camera.height || body.y + body.height < camera.y) return 1;// skip
    vec2 origin = _vec(e->body.width/2, e->body.height/2);
    // printf("drawing entity %d... ", e->id);
    float r = 0;
    vec2 draw = apply_camera(_vec(e->body.x+origin.x,e->body.y+origin.y) , camera);
    DrawTexturePro(e->image,
            _rect(0,0,e->image.width,e->image.height),
            _rect(draw.x, draw.y, e->body.width, e->body.height),
            origin,
            r, WHITE);
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
    printf("%d: w:h %f:%f\n", e.id, e.body.width, e.body.height);
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
    entity* enemy_2 = arena_alloc(a, sizeof(entity));
    init_entity(enemy_2, h, path);
    enemy_2->body.x = 0;
    enemy_2->body.y = 0;
    entities->data[entities->count++] = enemy_2;
    return enemy_2;
}
typedef struct {
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
    projectile projectiles[100];
} game;
int projectiles_add_new(
    projectile projectiles[100], projectile p){
    for (int i = 0; i < 100; i++) {
        if (projectiles[i].active == 0) {
            p.active = 1;
            projectiles[i] = p;
            printf("added projectile.\n");
            return 1;
        }
    }
    return 0;
}

int attack(game* g, entity* entity, int atk_index) {
    entity_attack atk = entity->attacks[atk_index];
    switch (atk.attack_kind) {
        case attack_kind_projectile:
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
                            printf("attackdiredction %f %f radius %f range %f.\n",
                                    direction.x, direction.y,
                                    prjk.straight.radius, prjk.range);
                            projectile p = prjk;
                            p.straight.origin = origin;
                            p.straight.direction = direction;
                            p.start = get_time();
                            if (!projectiles_add_new(g->projectiles,p)) {
                                panic("failed to add projectile, likely full. handle.");
                                return 0;
                            }
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
int main(void) {
    printf("Hello World!\n");
    InitWindow(winw, winh, "Hello Raylib");
    SetTargetFPS(60);

    Arena a = arena_new(1025, sizeof(entity));
    int ecount = 0; // entities count
    new_dynarr(entity, entities);
    // take reference to player
    entity* player = entities_new_entity(&a, &entities, 150.0f,"/home/kleidi/.tmp/$RPC8M8N/35a07685477d119f91d25f938e0df5193e29f6ae - Copy.jpg");
    entity_attack* pa1 = &player->attacks[0];
    pa1->attack_kind = attack_kind_projectile;
    pa1->projectile.kind=projectile_straight;
    pa1->projectile.damage=1.2*player->atk;
    pa1->projectile.range=400;
    pa1->projectile.straight.area=0; // one enemy
    pa1->projectile.straight.color = RED;
    pa1->projectile.straight.radius=2; // radius of 2;
    pa1->projectile.straight.speed= 1000;
    entity* enemy;
    enemy = entities_new_entity(&a, &entities, 120.0f,"/home/kleidi/.tmp/New folder/GwZQHkFXYAAYjkQ.jpg");
    enemy->body.x = 200;
    enemy->body.y = 200;
    enemy = entities_new_entity(&a, &entities, 120.0f,"/home/kleidi/.tmp/New folder/G0P8taRXoAAlmj_.jpg");
    enemy->body.x = -100;
    enemy->body.y = 150;
    enemy = entities_new_entity(&a, &entities, 120.0f,"/home/kleidi/.tmp/HDjhNy5XAAAj08i?format=jpg.jpg");
    enemy->body.x = -100;
    enemy->body.y = -350;
    game game;
    memset(&game, 0, sizeof(game));

    double last = get_time();

    while (!WindowShouldClose()) {
        // dt
        double now = get_time();
        double dt = now - last;
        last = now;
        vec2 vel = {0,0};
        game.mouse_pos = GetMousePosition();
        if (IsKeyDown(KEY_A)) vel.x += -1;
        if (IsKeyDown(KEY_D)) vel.x += 1;
        if (IsKeyDown(KEY_W)) vel.y += -1;
        if (IsKeyDown(KEY_S)) vel.y += 1;
        vel = Vector2Normalize(vel);
        vel.x *= 200.0f*(float)dt;
        vel.y *= 200.0f*(float)dt;
        player->body.x += vel.x;
        player->body.y += vel.y;
        int k = 0;
        while ((k = GetKeyPressed())) {
            if (k == KEY_SPACE) {
                attack(&game, player, 0);
            }
        }

        rect camera;
        camera.x = player->body.x + player->body.width/2 - (float)winw/2;
        camera.y = player->body.y + player->body.height/2 - (float)winh/2;
        camera.width = winw;
        camera.height= winh;
        BeginDrawing();
        ClearBackground(BLACK);
        sort_entities_by_y(entities.data, entities.count);
        for (int i = 0; i < entities.count; i++) {
            draw_entity(entities.data[i], camera);
            for (int i = 0; i < 100; i++) {
                projectile p = game.projectiles[i];
                if (p.active) {
                    float range = p.range;
                    float r = p.straight.radius;
                    Color c = p.straight.color;
                    double from_start = now - p.start;
                    vec2 traveled = vec2scale(p.straight.direction,
                            from_start*p.straight.speed);
                    if (vec2len(traveled) >= p.range) {
                        p.active = 0;
                        continue;
                    }
                    vec2 pos = vec2add(p.straight.origin, traveled);
                    pos = apply_camera(pos, camera);
                    info("World Pos %f %f (%f) len %f.", pos.x, pos.y, from_start*p.straight.speed, range);
                    DrawCircleV(pos, r, c);
                }
            }
        }
        EndDrawing();
    }
    for (int i = 0; i < entities.count; i++) {
        UnloadTexture(entities.data[i]->image);
    }
    free(entities.data);
    for (int i = 0; i < a.pages_count; i++) {
        free(a.pages[i]);
    }
    free(a.pages);
    entities.data = 0;
    return 0;
}
