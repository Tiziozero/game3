#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "utils.h"

#define winh 750
#define winw 1200
typedef Vector2 vec2;
typedef Rectangle rect;
#define _rect(a, b, c, d) (rect){a,b,c,d}
#define _vec(a, b) (vec2){a,b}
#define dec_dynarr(t) typedef struct { t** data; int count; \
                        int cap; int size; } dynarr_##t;
#define new_dynarr(t, name) dynarr_##t name = {}; name.count = 0; \
                      name.size = sizeof(t*); \
                      name.data = malloc((name.cap = 10) * name.size); \
                      memset(name.data, 0, (name.cap = 10) * name.size);

typedef struct {
    int id;
    Texture2D image;
    rect body; // pos/destination
    double atk, def, health; // def:multuplier for damage
    struct {
        rect source;
    } draw;
} entity;
dec_dynarr(entity);

int draw_entity(entity* e, rect camera) {
    rect body = e->body;
    if (body.x > camera.x+camera.width || body.x + body.width < camera.x) return 1;// skip
    if (body.y > camera.y+camera.height || body.y + body.height < camera.y) return 1;// skip
    vec2 origin = _vec(e->body.width/2, e->body.height/2);
    printf("drawing entity %d... ", e->id);
    float r = 0;
    DrawTexturePro(e->image,
            _rect(0,0,e->image.width,e->image.height),
            _rect(e->body.x+origin.x-camera.x, e->body.y+origin.y-camera.y,
                e->body.width, e->body.height),
            origin,
            r, WHITE);
    return 1;
}
int init_entity(entity* ent, float h, char* path) {
    static int id = 0;
    entity e;
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
#include <time.h>

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
int main(void) {
    printf("Hello World!\n");
    InitWindow(winw, winh, "Hello Raylib");
    SetTargetFPS(60);

    Arena a = arena_new(1025, sizeof(entity));
    int ecount = 0; // entities count
    new_dynarr(entity, entities);
    // take reference to player
    entity* player = entities_new_entity(&a, &entities, 150.0f,"/home/kleidi/.tmp/$RPC8M8N/35a07685477d119f91d25f938e0df5193e29f6ae - Copy.jpg");
    entity* enemy;
    enemy = entities_new_entity(&a, &entities, 120.0f,"/home/kleidi/.tmp/New folder/GwZQHkFXYAAYjkQ.jpg");
    enemy->body.x = 200;
    enemy->body.y = 200;
    enemy = entities_new_entity(&a, &entities, 120.0f,"/home/kleidi/.tmp/New folder/G0P8taRXoAAlmj_.jpg");
    enemy->body.x = -100;
    enemy->body.y = 150;

    double last = get_time();

    while (!WindowShouldClose()) {
        // dt
        double now = get_time();
        double dt = now - last;
        last = now;
        vec2 vel = {0,0};
        if (IsKeyDown(KEY_A)) vel.x += -1;
        if (IsKeyDown(KEY_D)) vel.x += 1;
        if (IsKeyDown(KEY_W)) vel.y += -1;
        if (IsKeyDown(KEY_S)) vel.y += 1;
        vel = Vector2Normalize(vel);
        vel.x *= 200.0f*(float)dt;
        vel.y *= 200.0f*(float)dt;
        player->body.x += vel.x;
        player->body.y += vel.y;

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
        }
        printf("\n");
        EndDrawing();
    }
    for (int i = 0; i < entities.count; i++) {
        UnloadTexture(entities.data[i]->image);
    }
    free(entities.data);
    entities.data = 0;
    return 0;
}
