#include <stdio.h>
#include "logger.h"
#include "raylib.h"
#include "defines.h"
#include "utils.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "external/lua/lua.h"
#include "external/lua/lauxlib.h"
#include "external/lua/lualib.h"
#define MAX_ELEMENTS 100
#define MAX_ABILITIES 10
Texture2D textures[256];
int t_index = 0;;
int player_handle;
// /home/kleidi/.tmp/Konachan.com - 346965 sample.adadjpg.jpg

#define cellw 3*16
#define cellh 3*16
#define srccellw 16
#define srccellh 16
void dbg_table(lua_State* L, int index);

#define winh 750
#define winw 1200
typedef Vector2 vec2;
typedef Rectangle rect;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int load_env(const char *filename);

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
    int ref, instance;
} item;

typedef enum {
    status_on = 1,
    status_die, // when dying
    status_dead = 0,
}entity_status;
typedef struct {
    entity_status status;
    int handle;
    Texture2D image;
    rect body; // pos/destination
    double atk, def, health, max_health; // def:multuplier for damage
    struct {
        rect source;
    } draw;
    item abilities[MAX_ELEMENTS];
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
    int draw_health = 1;
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
    if (draw_health) {
    float health_bar_width = 0.8 * e->body.width;
    float health_bar_height = 5;
    vec2 pos = vec2add(
            // offset by 0.1% of body, sice width is 0.8
            vec2add(vec2scale(_vec(1,0), 0.1*e->body.width), _vec(0,-10)), rect_pos(e->body));
    DrawRectangleV(apply_camera(pos, camera),
            _vec(health_bar_width, health_bar_height), BLACK);
    // draw healt
    DrawRectangleV(apply_camera(pos, camera),
            _vec(health_bar_width*(e->health/e->max_health),
                health_bar_height), RED);
    }
    return 1;
}
int init_entity(entity* ent, float h, char* path) {
    static int handle = 1;
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
    e.handle = handle++;
    dbg("%d: w:h %f:%f", e.handle, e.body.width, e.body.height);
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
    e->body.x = 1000;
    e->body.y = 1000;
    e->status = status_on;
    entities->data[entities->count++] = e;
    return e;
}
entity* entities_remove_entity(dynarr_entity* entities, int index) {
    if (index < 0 || index >= entities->count) return NULL;

    entity* removed = entities->data[index];

    entities->count--;
    entities->data[index] = entities->data[entities->count];

    return removed;
}
typedef struct {
    int active, handle, instance_ref, ref;
    uint16_t generation;   // increments every reuse
} element; // like projectiles and what not
dec_dynarr(element);
    // Map: script path -> LUA_REF
typedef struct {
    char* path;
    int ref; // luaL_ref to the loaded chunk/table
} script_entry;
typedef struct {
    dynarr_entity entities;
    vec2 mouse_pos;
    entity* player; // player
    element elements[MAX_ELEMENTS];
    int energy;
    int eps; // energy recovered per second.

    Arena cycle_arena; // arena that gets reset every cycle for temporary stuff

    Arena chat_arena; // arena for messages etc
    char** logs; // circular buffer
    int log_count; // at which to start
    int log_cap; // at which to start

    double dt;
    
    rect* camera;

    script_entry loaded_scripts[128]; // or dynamic map
    int loaded_count;
    lua_State* L;
} game;
int get_lua_script_function(lua_State* L, int script_ref, const char* func_name);
entity* find_entity(game* g, int handle);
static inline int make_handle(int index, uint16_t gen) {
    return (gen << 16) | index;
}

static inline int handle_index(int handle) {
    return handle & 0xFFFF;
}

static inline uint16_t handle_gen(int handle) {
    return (handle >> 16) & 0xFFFF;
}
element* get_element(game* g, int handle) {

    int idx = handle_index(handle);
    uint16_t gen = handle_gen(handle);

    if (idx < 0 || idx >= MAX_ELEMENTS)
        return NULL;

    element* e = &g->elements[idx];

    if (!e->active)
        return NULL;

    if (e->generation != gen)
        return NULL; // stale handle

    return e;
}
int game_get_or_load_script(game* game, const char* path);
int game_log(game*game, char*msg) {
    if (game->log_count >= game->log_cap) {
        game->logs = realloc(game->logs, (game->log_cap*=2)*sizeof(char*));
        dbg("realloced chat %zu msgs cap", game->log_cap);
    }
    game->logs[game->log_count++] = msg;
    return 1;
}
int _game_new_element(
    element elements[100], element e){
    static int index = 1; // start at 1. 0 is invalid
    for (int i = 0; i < 100; i++) {
        if (elements[i].active == 0) {
            e.active = 1;
            e.handle = index++;
            elements[i] = e;
            dbg("added element.");
            return 1;
        }
    }
    return 0;
}
int game_spawn_element(game* g, int ref, int instance) {
    for (int i = 0; i < MAX_ELEMENTS; i++) {
        element* e = &g->elements[i];
        if (!e->active) {
            e->active = 1;
            e->generation++;     // invalidate old handles
            e->ref = ref;
            e->instance_ref = instance;
            return make_handle(i, e->generation);
        }
    }

    return -1; // full
}
void remove_element(game* g, int handle) {

    element* e = get_element(g, handle);
    if (!e) {
        panic("no e");
        return;
    }

    lua_State* L = g->L;


    if (e->instance_ref != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, e->instance_ref);

    e->active = 0;
    info("Removed element %d", handle);
    // DO NOT increment generation here
    // generation increments on next spawn
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

entity* find_entity(game* g, int handle) {
    for (int i=0;i<g->entities.count;i++)
        if (g->entities.data[i]->handle == handle)
            return g->entities.data[i];
    return NULL;
}
float damage_target(float total_atk,entity* e) {
    e->health -= total_atk;
    if (e->health <= 0) {
        e->health = 0;
        e->status = status_die;
    }
    dbg("AFTER %f", e->health);
    return e->health;
}
int l_damage_entity(lua_State* L)
{
    int handle = luaL_checkinteger(L, 1);
    float dmg = luaL_checknumber(L, 2);

    game* g = getgame();
    entity* e = find_entity(g, handle);

    if (!e) return 0;

    damage_target(dmg, e);
    return 0;
}
int attack(game* g, entity* entity, int atk_index) {
    item atk = entity->abilities[atk_index];
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
int projectile_body_hit(vec2 p, float r,  rect body, vec2 prev_pos);
int l_projectile_body_hit(lua_State*L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    game* game = getgame();

    TODO("Handle");
    return 0;
}
int element_update(game* g, element* e) {
     if (!e->active) return 0;
     lua_State* L = g->L;
     info("Calling update.");
    if (!get_lua_script_function(L, e->ref, "update")) {
        panic("Failed to oget act.");
        return 0;
    }

    // 3. push the instance table as self
    lua_rawgeti(L, LUA_REGISTRYINDEX, e->instance_ref); // stack: script, act, self

    // 4. call act(self)
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) { // 1 arg = self
        printf("Lua act error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // pop error
        lua_pop(L, 1); // pop script table
        return 0;
    }

    // 5. clean up: pop script table
    lua_pop(L, 1);

    return 1;

}
int element_draw(game* g, element* e) {
    // warn("Handle");
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
int ability_act(lua_State* L, int script_ref, int instance_ref);
int player_ability(game* game, int handle, int ability) {
    entity* e = find_entity(game, handle);
    if (!e) {
        panic("No entity.");
        return 0;
    }
    if (ability >= 0 && ability < MAX_ABILITIES) {
        dbg("entity ability %d", ability);
        if (!ability_act(game->L, e->abilities[ability].ref, e->abilities[ability].instance)) {
            panic("Failed to act ability. %d", e->abilities[ability].instance);
            return 0;
        }
    } else {
        panic("unhandeled");
    }
    return 1;
}
// with updated position and prev
int projectile_body_hit(vec2 p, float r, rect body, vec2 prev_pos) {
    if (circle_rect_intersect(p, r, body)) {
        // projectile directly hits body
        return 1;
    }
    if (line_rect_intersect(p, prev_pos, body)) {
        // projectile has interescted body some time in change in pos
        return 1;
    }
    return 0; // else no intersection
}
typedef struct {
    int n;
} tile;
typedef struct map map;
struct map {
    tile* tiles;
    int rows,cols;
};

static entity* check_entity_handle(lua_State* L, int index) {
    int handle = luaL_checkinteger(L, index);

    game* g = getgame();
    entity* e = find_entity(g, handle);

    return e;
}
int l_engine_get_entity(lua_State* L) {
    entity* e = check_entity_handle(L, 1);

    if (!e) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    lua_pushinteger(L, e->handle);
    lua_setfield(L, -2, "handle");

    lua_pushnumber(L, e->body.x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, e->body.y);
    lua_setfield(L, -2, "y");

    lua_pushnumber(L, e->body.width);
    lua_setfield(L, -2, "w");

    lua_pushnumber(L, e->body.height);
    lua_setfield(L, -2, "h");

    lua_pushnumber(L, e->body.x + e->body.width/2);
    lua_setfield(L, -2, "centerx");

    lua_pushnumber(L, e->body.y + e->body.height/2);
    lua_setfield(L, -2, "centery");
    lua_pushnumber(L, e->health);
    lua_setfield(L, -2, "health");

    lua_pushnumber(L, e->max_health);
    lua_setfield(L, -2, "max_health");

    lua_pushnumber(L, e->atk);
    lua_setfield(L, -2, "atk");

    return 1;
}
int l_engine_set_entity_pos(lua_State* L) {
    entity* e = check_entity_handle(L, 1);
    if (!e) return 0;

    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    e->body.x = x;
    e->body.y = y;

    return 0;
}
int l_engine_damage_entity(lua_State* L) {
    entity* e = check_entity_handle(L, 1);
    if (!e) return 0;

    float dmg = luaL_checknumber(L, 2);

    damage_target(dmg, e);

    return 0;
}
int l_engine_entity_alive(lua_State* L) {
    entity* e = check_entity_handle(L, 1);

    lua_pushboolean(L, e && e->status == status_on);
    return 1;
}
int l_engine_get_entity_center(lua_State* L) {
    entity* e = check_entity_handle(L, 1);
    if (!e) return 0;

    lua_pushnumber(L, e->body.x + e->body.width*0.5f);
    lua_pushnumber(L, e->body.y + e->body.height*0.5f);
    return 2;
}
int l_engine_move_entity(lua_State* L) {
    entity* e = check_entity_handle(L, 1);
    if (!e) return 0;

    float dx = luaL_checknumber(L, 2);
    float dy = luaL_checknumber(L, 3);

    e->body.x += dx;
    e->body.y += dy;

    return 0;
}
int l_engine_entity_distance(lua_State* L) {
    entity* a = check_entity_handle(L, 1);
    entity* b = check_entity_handle(L, 2);

    if (!a || !b) {
        lua_pushnumber(L, 1e9);
        return 1;
    }

    float ax = a->body.x + a->body.width*0.5f;
    float ay = a->body.y + a->body.height*0.5f;

    float bx = b->body.x + b->body.width*0.5f;
    float by = b->body.y + b->body.height*0.5f;

    float dx = ax - bx;
    float dy = ay - by;

    lua_pushnumber(L, sqrtf(dx*dx + dy*dy));
    return 1;
}
int l_engine_get_dt(lua_State* L)
{
    game* g = getgame();

    if (!g) {
        lua_pushnumber(L, 0.0);
        return 1;
    }

    lua_pushnumber(L, g->dt);
    return 1;
}
int l_engine_get_mouse_pos_world(lua_State* L)
{
    game* g = getgame();

    if (!g || !g->camera) {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
        return 2;
    }

    vec2 world = unapply_camera(g->mouse_pos, *g->camera);

    lua_pushnumber(L, world.x);
    lua_pushnumber(L, world.y);
    return 2;
}

int script_init(lua_State* L, int script_ref, int owner_handle);
int l_engine_new_projectile(lua_State* L) {
    if (!lua_istable(L, 1)) {
        return luaL_error(L, "new_projectile expects a table");
    }

    // Example: read x, y
    lua_getfield(L, 1, "x");
    float x = luaL_optnumber(L, -1, 0.0f);
    lua_pop(L, 1);

    lua_getfield(L, 1, "y");
    float y = luaL_optnumber(L, -1, 0.0f);
    lua_pop(L, 1);
    lua_getfield(L, 1, "dx");
    float dx = luaL_optnumber(L, -1, 0.0f);
    lua_pop(L, 1);
    lua_getfield(L, 1, "dy");
    float dy = luaL_optnumber(L, -1, 0.0f);
    lua_pop(L, 1);
    lua_getfield(L, 1, "speed");
    float speed = luaL_optnumber(L, -1, 0.0f);
    lua_pop(L, 1);
    lua_getfield(L, 1, "owner");
    int owner_handle = luaL_optinteger(L, -1, 0);
    lua_pop(L, 1);
    if (!owner_handle) {
        panic("No handle");
    }

    lua_getfield(L, 1, "projectile_path");
    const char* projectile_path = luaL_optstring(L, -1, 0);
    lua_pop(L, 1);
    if (!projectile_path) {
        panic("no path in new projectile.");
        return 0;
    }
    vec2 pos = _vec(x,y);
    vec2 dir = vec2norm(_vec(dx,dy));

    // create projectile in C, add to game...
    dbg("New projectile at %.2f %.2f (%.5f %.5f) from %d speed %f",
            pos.x, pos.x, dir.x, dir.y, owner_handle, speed);

    game* g = getgame();

    int script_ref = game_get_or_load_script(g, projectile_path);
    if (!script_ref) {
        panic("Failed to load projectile script");
        return 0;
    }

    // Stack now has the script table, you can create a new 'self'
    // for this projectile
    int self_ref = script_init(L, script_ref, owner_handle);
    if (!self_ref) {
        panic("No self ref.");
        return 0;
    }

    int element_handle = game_spawn_element(g, script_ref, self_ref);
    if (!element_handle) {
        panic("Failed to create element.");
        return 0;
    }
    if (!get_lua_script_function(L,script_ref, "set_handle")) {
        panic("Failed to get set handle.");
        return 0;
    }

    dbg("setting handle.");
    lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref); // stack: script
    lua_pushinteger(L,element_handle);
    if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
        printf("failed to set handle: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    // return the same table back to Lua for convenience
    lua_pushvalue(L, 1);
    return 1;
}
int l_engine_apply_camera(lua_State* L) {
    game* g = getgame();
    if (!g || !g->camera) {
        return luaL_error(L, "No game or camera");
    }

    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);

    vec2 world = _vec(x, y);
    vec2 screen = apply_camera(world, *g->camera);

    lua_pushnumber(L, screen.x);
    lua_pushnumber(L, screen.y);
    return 2;
}
int l_engine_draw_rect(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float w = luaL_checknumber(L, 3);
    float h = luaL_checknumber(L, 4);

    int r = luaL_checkinteger(L, 5);
    int g = luaL_checkinteger(L, 6);
    int b = luaL_checkinteger(L, 7);
    int a = luaL_optinteger(L, 8, 255);

    DrawRectangle((int)x, (int)y, (int)w, (int)h, (Color){r,g,b,a});

    return 0;
}
int l_engine_draw_line(lua_State* L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);

    int r = luaL_checkinteger(L, 5);
    int g = luaL_checkinteger(L, 6);
    int b = luaL_checkinteger(L, 7);
    int a = luaL_optinteger(L, 8, 255);

    DrawLine((int)x1, (int)y1, (int)x2, (int)y2, (Color){r,g,b,a});

    return 0;
}
int l_engine_draw_circle(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);

    int r = luaL_checkinteger(L, 4);
    int g = luaL_checkinteger(L, 5);
    int b = luaL_checkinteger(L, 6);
    int a = luaL_optinteger(L, 7, 255);

    DrawCircle((int)x, (int)y, radius, (Color){r,g,b,a});

    return 0;
}
int l_engine_draw_image(lua_State* L) {
    int idx = luaL_checkinteger(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float w = luaL_checknumber(L, 4);
    float h = luaL_checknumber(L, 5);

    Texture2D tex = textures[idx];

    Rectangle src = {0,0,(float)tex.width,(float)tex.height};
    Rectangle dst = {x,y,w,h};

    DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, WHITE);

    return 0;
}
int l_engine_draw_text(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    int size = luaL_checkinteger(L, 4);

    int r = luaL_checkinteger(L, 5);
    int g = luaL_checkinteger(L, 6);
    int b = luaL_checkinteger(L, 7);
    int a = luaL_optinteger(L, 8, 255);

    DrawText(text, (int)x, (int)y, size, (Color){r,g,b,a});

    return 0;
}
// screen pos
int l_engine_get_mouse_pos_screen(lua_State* L) {
    game* g = getgame();
    lua_pushnumber(L, g->mouse_pos.x);
    lua_pushnumber(L, g->mouse_pos.y);
    return 2;
}
int l_game_get_element(lua_State* L) {
    game* g = getgame();

    int handle = luaL_checkinteger(L, 1);

    if (handle < 0 || handle >= 100) {
        lua_pushnil(L);
        return 1;
    }

    element* e = &g->elements[handle];

    if (!e->active) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushlightuserdata(L, e);
    return 1;
}

int l_engine_remove_element(lua_State* L) {
    game* g = getgame();

    int handle = luaL_checkinteger(L, 1);

    remove_element(g, handle);
    info("REMOVED ELEMENT.");

    return 0;
}
int l_engine_panic(lua_State* L) {
    panic("0");
    return 0;
}
void register_engine(lua_State* L) {
    lua_newtable(L); // engine

    lua_pushcfunction(L, l_engine_get_entity);
    lua_setfield(L, -2, "get_entity");

    lua_pushcfunction(L, l_engine_set_entity_pos);
    lua_setfield(L, -2, "set_entity_pos");

    lua_pushcfunction(L, l_engine_damage_entity);
    lua_setfield(L, -2, "damage_entity");

    lua_pushcfunction(L, l_engine_entity_alive);
    lua_setfield(L, -2, "entity_alive");

    lua_pushcfunction(L, l_engine_get_dt);
    lua_setfield(L, -2, "get_dt");

    lua_pushcfunction(L, l_engine_get_mouse_pos_world);
    lua_setfield(L, -2, "get_mouse_pos_world");

    lua_pushcfunction(L, l_engine_get_mouse_pos_screen);
    lua_setfield(L, -2, "get_mouse_pos_screen");

    lua_pushcfunction(L, l_engine_new_projectile);
    lua_setfield(L, -2, "new_projectile");

    lua_pushcfunction(L, l_engine_apply_camera);
    lua_setfield(L, -2, "apply_camera");

    lua_pushcfunction(L, l_engine_remove_element);
    lua_setfield(L, -2, "remove_element");

    lua_pushcfunction(L, l_engine_panic);
    lua_setfield(L, -2, "panic");

    lua_setglobal(L, "engine");
}
void dbg_table(lua_State* L, int index) {
    if (!lua_istable(L, index)) {
        printf("Not a table\n");
        return;
    }

    lua_pushnil(L); // first key
    printf("Table contents:\n");
    while (lua_next(L, index)) {
        // key at -2, value at -1
        const char* k = lua_tostring(L, -2);
        if (lua_isstring(L, -1)) {
            printf("  %s = %s\n", k, lua_tostring(L, -1));
        } else if (lua_isnumber(L, -1)) {
            printf("  %s = %f\n", k, lua_tonumber(L, -1));
        } else if (lua_isfunction(L, -1)) {
            printf("  %s = <function>\n", k);
        } else if (lua_istable(L, -1)) {
            printf("  %s = <table>\n", k);
        } else {
            printf("  %s = <other>\n", k);
        }
        lua_pop(L, 1); // remove value, leave key for next
    }
}
int script_init(lua_State* L, int script_ref, int owner_handle)
{
    // 1. Create instance table
    lua_newtable(L); // stack: instance

    // 2. Optional: attach the script table inside the instance
    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref); // stack: instance, script
    lua_setfield(L, -2, "script");                // instance.script = script; stack: instance

    // 3. Store the instance table as a ref
    int instance_ref = luaL_ref(L, LUA_REGISTRYINDEX); // pops the instance

    // call init if it exists
    lua_getfield(L, -1, "init"); // stack: script, init_function
    if (lua_isfunction(L, -1)) {
        // push instance first
        lua_rawgeti(L, LUA_REGISTRYINDEX, instance_ref); // self
        lua_pushinteger(L, owner_handle);               // owner
        lua_pushinteger(L, script_ref);                // optional script ref

        if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
            printf("Lua init error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1); // pop non-function
        err("No ini.t");
        panic("0");
    }

    lua_pop(L, 1); // pop script

    return instance_ref;
}
// Pushes the function from a script table by registry ref
// Returns 1 on success (function pushed), 0 on failure
int get_lua_script_function(lua_State* L, int script_ref, const char* func_name)
{
    // 1. push script table
    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref); // stack: script

    // 2. get the requested function
    lua_getfield(L, -1, func_name);               // stack: script, func

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2); // pop script table and non-function value
        err("'%s' is not a function in script %d", func_name, script_ref);
        return 0;
    }

    // At this point, the function is on top of the stack
    // The script table is still below it in case you need it
    return 1;
}
// instance metatable contains script ref
int ability_act(lua_State* L, int script_ref, int instance_ref) {
    dbg("Ability script/instance %d/%d.", script_ref, instance_ref);
    // 1. push the script table
    if (!get_lua_script_function(L, script_ref, "act")) {
        panic("Failed to oget act.");
        return 0;
    }

    // 3. push the instance table as self
    lua_rawgeti(L, LUA_REGISTRYINDEX, instance_ref); // stack: script, act, self

    // 4. call act(self)
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) { // 1 arg = self
        printf("Lua act error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // pop error
        lua_pop(L, 1); // pop script table
        return 0;
    }

    // 5. clean up: pop script table
    lua_pop(L, 1);

    return 1;


    dbg("Ability script/instance %d/%d.", script_ref, instance_ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, instance_ref); // push instance
    lua_getfield(L, -1, "script");                  // push instance.script (script)
    lua_getfield(L, -1, "act");                     // push script.act
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        err("Not a function");
        return 0;
    }
    // pus self
    lua_pushvalue(L, -3);                           // push instance as self
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        printf("Lua act error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return 0;
    }

    lua_pop(L, 1); // pop instance
    return 1;
}

int game_get_or_load_script(game* game, const char* path){
    // 1️⃣ Check if already loaded
    for (int i = 0; i < game->loaded_count; i++) {
        if (strcmp(game->loaded_scripts[i].path, path) == 0) {
            // Push the existing script table onto the Lua stack
            lua_rawgeti(game->L, LUA_REGISTRYINDEX,
                    game->loaded_scripts[i].ref);
            return game->loaded_scripts[i].ref;
        }
    }

    // 2️⃣ Not loaded yet, load it
    if (luaL_dofile(game->L, path) != 0) {
        fprintf(stderr, "Error loading script '%s': %s\n", path,
                lua_tostring(game->L, -1));
        lua_pop(game->L, 1);
        return 0; // failure
    }

    // 3️⃣ Create a registry reference
    int ref = luaL_ref(game->L, LUA_REGISTRYINDEX);

    // 4️⃣ Store in game struct
    if (game->loaded_count >= 128) {
        fprintf(stderr, "Exceeded max loaded scripts\n");
        return 0;
    }

    game->loaded_scripts[game->loaded_count].path = strdup(path);
    game->loaded_scripts[game->loaded_count].ref = ref;
    game->loaded_count++;

    // Push the script table back onto the stack for convenience
    lua_rawgeti(game->L, LUA_REGISTRYINDEX, ref);

    info("New Script Ref %d.", ref);
    return ref;
}


int init_ability(game* game, int entity_handle, int ability, char* script) {
    entity* e = find_entity(game, entity_handle);
    if (!e) {
        panic("entity does not exist.");
        return 0;
    }
    if (e->abilities[ability].instance != LUA_NOREF) {
        luaL_unref(game->L, LUA_REGISTRYINDEX, e->abilities[ability].instance);
        e->abilities[ability].instance = LUA_NOREF;
    }
    if (e->abilities[ability].ref != LUA_NOREF) {
        luaL_unref(game->L, LUA_REGISTRYINDEX, e->abilities[ability].ref);
        e->abilities[ability].ref = LUA_NOREF;
    }
    int ref = game_get_or_load_script(game, script);
    if (!ref) {
        panic("No ref.");
        return 0;
    }
    item i;
    int instance = script_init(game->L, ref, entity_handle);
    if (!instance) {
        panic("No instance.");
        return 0;
    }
    i.ref = ref;
    i.instance = instance;
    e->abilities[ability] = i;
    return 1;
}
int main(void) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    register_engine(L);


    load_env(".env");
    printf("Hello World!\n");
    InitWindow(winw, winh, "Hello Raylib");
    SetTargetFPS(60);

    map map;
    map.rows = map.cols = 1024;
    dbg("%d tiles.",map.rows*map.cols*sizeof(tile));
    map.tiles = calloc(1, map.rows*map.cols*sizeof(tile));
    for (int i = 0; i < map.cols; i++) {
        for (int j = 0; j < map.rows; j++) {
            map.tiles[j*map.cols+i].n = rand()%3;
        }
    }

    Texture tiles = LoadTexture(getenv("TILES"));
    int cols = tiles.width/16;
    int rows = tiles.height/16;

    game game;
    memset(&game, 0, sizeof(game));
    setgame(&game);
    game.L = L;

    Arena a = arena_new(1025, sizeof(entity));
    int ecount = 0; // entities count
    new_dynarr(entity, _entities);

    game.entities = _entities;
    // take reference to player
    entity* player = entities_new_entity(&a, &game.entities,
            80.0f,getenv("PLAYER_PATH"));
    player_handle = player->handle;
    if (player_handle == 0) { panic("0");}
    game.player = player;
    player->atk = 50;
    player->health = 230;
    player->max_health = 230;
    init_ability(&game, player_handle,0, "scripts/test.lua");
    printf("Player :%s\n", getenv("PLAYER_PATH"));
    entity* enemy = entities_new_entity(&a, &game.entities,
            90.0f,getenv("ENEMY1"));
    enemy->atk = 50;
    enemy->health = 230;
    enemy->max_health = 230;
    enemy->body.x += 200;
    enemy->body.y += 200;
    enemy = entities_new_entity(&a, &game.entities,
            95.0f,getenv("ENEMY2"));
    enemy->atk = 50;
    enemy->health = 230;
    enemy->max_health = 230;
    enemy->body.x  += -100;
    enemy->body.y  += 150;
    enemy = entities_new_entity(&a, &game.entities,
            200.0f,getenv("ENEMY3"));
    enemy->atk = 200;
    enemy->health = 1030;
    enemy->max_health = 1030;
    enemy->body.x += -100;
    enemy->body.y += -350;
    game.log_cap = 10;
    game.log_count = 0;
    game.logs = malloc(game.log_cap*sizeof(char*));


    int range_v = (int)winh/(cellh)/2+2;
    int range_h = (int)winw/(cellw)/2+2;

    info("Player index %d.", player_handle);
    int draw_logs = 1;
    game.dt = 0;
    double last = get_time();
    rect camera;
    game.camera= &camera;
    vec2  player_dest = rect_pos(player->body);
    while (!WindowShouldClose()) {
        // dt
        double now = get_time();
        double dt = now - last;
        last = now;
        game.dt = dt;
        game.mouse_pos = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            player_dest = unapply_camera(game.mouse_pos, *game.camera);

            // sub half the body to get center, so player moves center there
            player_dest = vec2sub(player_dest,
                    vec2scale(rect_size(player->body), 0.5));

        }
        float dd = vec2len(vec2sub(rect_pos(player->body), player_dest));
        if (dd > 0.01) {
            vec2 change = vec2scale(vec2norm(
                        vec2sub(player_dest,rect_pos(player->body))
                        ), 200.0f*(float)dt);
            if (vec2len(change) > dd) {
                // if it's greater than distance, set to distance
                change = vec2scale(vec2norm(change), dd);
            }
            player->body.x += change.x;
            player->body.y += change.y;
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
                player_ability(&game, player_handle, 0);
            } else if (k == KEY_W) {
                player_ability(&game, player_handle, 1);
            } else if (k == KEY_E) {
                player_ability(&game, player_handle, 2);
            } else if (k == KEY_R) {
                player_ability(&game, player_handle, 3);
            } else if (k == KEY_D) {
                player_ability(&game, player_handle, 4);
            } else if (k == KEY_X) {
                CloseWindow();
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {// left click
            player_ability(&game, player_handle, 0);
        }

        camera.x = player->body.x + player->body.width/2 - (float)winw/2;
        camera.y = player->body.y + player->body.height/2 - (float)winh/2;
        camera.width = winw;
        camera.height= winh;
        BeginDrawing();
        ClearBackground(BLACK);
        // draw bg
        {
            // get player quadrants. about 5 on each side
            // map starts at 0,0
            vec2 offset = rect_pos(*game.camera);
            int x = (int)(player->body.x / (float)(cellw));
            int y = (int)(player->body.y / (float)(cellh));

            for (int i = x-range_h; i <= x+range_h && i < map.cols; i++) {
                for (int j = y-range_v; j <= y+range_v && j < map.rows; j++) {
                    vec2 draw = apply_camera(_vec(i*cellw, j*cellh),
                            *game.camera);
                    DrawTexturePro(tiles,
                    _rect(304+srccellw*map.tiles[j*map.cols+i].n, 16,
                        srccellw,srccellh),
                    _rect(draw.x, draw.y, cellw, cellh),
                    _vec(0,0),0,WHITE);
                }
            }
        }
        sort_entities_by_y(game.entities.data, game.entities.count);
        for (int i = 0; i < game.entities.count; i++) {
            draw_entity(game.entities.data[i], camera);
        }
        for (int i = 0; i < 100; i++) {
            element* e = &game.elements[i];
            if (!e->active) continue;
            element_update(&game, e);
            element_draw(&game, e);
        }
        if (draw_logs) {
            for (int i = game.log_count-1;
                    i >= game.log_count-20 && i >= 0; i--) { // max 20
                DrawText(game.logs[i], 0, 500 - 20*(game.log_count-i),
                        20, WHITE);
            }
        }
        DrawCircleV(apply_camera(vec2add(player_dest,
                        vec2scale(rect_size(player->body), 0.5)),
                    *game.camera), 4, WHITE);
        DrawFPS(10, 10);
        EndDrawing();
        for (int i = 0; i < game.entities.count; i++) {
            entity* e = game.entities.data[i];
            if (e->status == status_dead || e->status == status_die) {
                if (!entities_remove_entity(&game.entities, i)) {
                    panic("null entity in remove.");
                    exit(-1);
                }
            }
        }
    }
    for (int i = 0; i < game.entities.count; i++) {
        UnloadTexture(game.entities.data[i]->image);
    }
    free(game.entities.data);
    game.entities.data = 0;
    arena_free(&a);
    arena_free(&game.cycle_arena);
    arena_free(&game.chat_arena);
    free(map.tiles);
    lua_close(L);
    return 0;
}
