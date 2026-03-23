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
#include "game.h"
// /home/kleidi/.tmp/Konachan.com - 346965 sample.adadjpg.jpg

int load_env(const char *filename);

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
int basic_entity_update(game* game, int handle, void* payload) {
    return 1;
}
int basic_entity_draw(game* game, int handle, void* payload) {
    rect camera = *game->camera;
    entity* e = find_entity(game, handle);
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
entity* game_new_entity(game* game, entity new_e) {
    entity* e = arena_alloc(&game->gpa, sizeof(entity));
    *e = new_e;
    e->status = status_on;
    game->entities.data[game->entities.count++] = e;
    return e;
}
entity* entities_remove_entity(dynarr_entity* entities, int index) {
    if (index < 0 || index >= entities->count) return NULL;

    entity* removed = entities->data[index];

    entities->count--;
    entities->data[index] = entities->data[entities->count];
    dbg("Removed entity %d.", index);

    return removed;
}
    // Map: script path -> LUA_REF
int get_lua_script_function(lua_State* L, int script_ref, const char* func_name);
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
element* game_alloc_element(game* g) {
    element e = {0};
    element* _e = game_spawn_element(g, e);
    return _e;
}
int global_elements_count = 0;
element* game_spawn_element(game* g, element _new) {
    for (int i = 0; i < g->elements_cap; i++) {
        element* e = &g->elements[i];
        if (!e->active) {
            uint16_t gen = e->generation;
            *e = _new;
            e->active = 1;
            e->generation = gen + 1;
            if (e->generation == 0) e->generation = 1;
            e->handle = make_handle(i, e->generation);
            global_elements_count++;
            return e;
        }
    }
    dbg("realloc game elements.");
    int old_cap = g->elements_cap;
    g->elements_cap *= 2;
    g->elements = realloc(g->elements, g->elements_cap * sizeof(element));
    memset(g->elements + old_cap, 0, old_cap * sizeof(element));
    return game_spawn_element(g, _new);
}
void remove_element(game* g, int handle) {
    element* e = get_element(g, handle);
    if (!e) {
        panic("no e with handle %d.", handle);
        return;
    }
    e->active = 0;
    free(e->payload);
    global_elements_count--;
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
        if (g->entities.data[i]->handle == handle &&
                g->entities.data[i]->status != status_dead)
            return g->entities.data[i];
    return NULL;
}
float damage_target(float total_atk,entity* e) {
    e->health -= total_atk;
    if (e->health <= 0) {
        e->health = 0;
        e->status = status_die;
        dbg("entity %d dead", e->handle);
    }
    return e->health;
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
int element_update(game* g, element* e) {
    if (!e->active) return 0;
    return e->update(g, e->handle, e->payload);
}
int element_draw(game* g, element* e) {
    if (!e->active) return 0;
    return e->draw(g, e->handle, e->payload);

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
int l_ability_act(lua_State* L, int script_ref, int instance_ref);
int entity_ability(game* game, int handle, int ability) {
    entity* e = find_entity(game, handle);
    if (!e) {
        panic("No entity.");
        return 0;
    }
    if (ability >= 0 && ability < MAX_ABILITIES) {
        if (!e->abilities[ability].active) {
            err("No ability");
            return 0;
        }
        if (!ability_act(game, e->abilities[ability])) {
            panic("Failed to act ability. %s", e->abilities[ability].name);
            return 0;
        }
    } else {
        panic("unhandeled");
    }
    return 1;
}
int ability_act(game* game, ability ability) {
    return ability.act(game, ability.payload);
}
float point_segment_dist(vec2 p, vec2 a, vec2 b) {
    vec2 ab = { b.x - a.x, b.y - a.y };
    vec2 ap = { p.x - a.x, p.y - a.y };
    float t = (ap.x*ab.x + ap.y*ab.y) / (ab.x*ab.x + ab.y*ab.y);
    t = fmaxf(0.0f, fminf(1.0f, t));  // clamp to segment
    vec2 closest = { a.x + t*ab.x, a.y + t*ab.y };
    float dx = p.x - closest.x, dy = p.y - closest.y;
    return sqrtf(dx*dx + dy*dy);
}
float line_segment_shortest_dist(vec2 a, vec2 b, vec2 c, vec2 d) {
    // Shortest dist between segment AB and segment CD
    // = min of all four point-to-segment distances
    //   (if segments don't intersect, which here they don't since we already checked)
    float d1 = point_segment_dist(a, c, d);
    float d2 = point_segment_dist(b, c, d);
    float d3 = point_segment_dist(c, a, b);
    float d4 = point_segment_dist(d, a, b);
    return fminf(fminf(d1, d2), fminf(d3, d4));
}

float line_rect_shortest_dist(vec2 p1, vec2 p2, rect body) {
    vec2 tl = { body.x,              body.y               };
    vec2 tr = { body.x + body.width, body.y               };
    vec2 bl = { body.x,              body.y + body.height };
    vec2 br = { body.x + body.width, body.y + body.height };

    float d0 = line_segment_shortest_dist(p1, p2, tl, tr); // top
    float d1 = line_segment_shortest_dist(p1, p2, bl, br); // bottom
    float d2 = line_segment_shortest_dist(p1, p2, tl, bl); // left
    float d3 = line_segment_shortest_dist(p1, p2, tr, br); // right
    return fminf(fminf(d0, d1), fminf(d2, d3));
}
// with updated position and prev
int projectile_body_hit(vec2 p, float r, rect body, vec2 prev_pos, float* out_dist) {

    if (circle_rect_intersect(p, r, body)) {
        // projectile directly hits body
        if (out_dist) *out_dist = 0.0f; // inside/touching, distance is 0
        return 1;
        return 1;
    }
    if (line_rect_intersect(p, prev_pos, body)) {
        // // projectile has interescted body some time in change in pos
        if (out_dist) *out_dist = line_rect_shortest_dist(p, prev_pos, body);
        return 1;
        return 1;
    }
    return 0; // else no intersection
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



collisions_ret_t* entities_line_intersect(vec2 p1, vec2 p2, float r, int* _count) {
    int cap = 10;
    collisions_ret_t* c = calloc(1,cap*sizeof(collisions_ret_t));
    int count = 0;

    float x1 = p1.x, x2 = p2.x, y1= p1.y, y2 = p2.y;
    for (int i = 0; i < getgame()->entities.count; i++) {
        entity* e = getgame()->entities.data[i];
        if (!e) continue;

        // bounding box check
        if (e->body.x + e->body.width < fmaxf(x1,x2) ||
                e->body.x > fminf(x1,x2) ||
                e->body.y + e->body.height < fmaxf(y1,y2) ||
                e->body.y > fminf(y1,y2))
            continue;

        float dist;
        if (projectile_body_hit(_vec(x1, y1),
                    r, e->body, _vec(x2, y2), &dist)) {
            if (count >= cap) {
                c = realloc(c,
                        (cap*=2)*sizeof(struct{int handle;float distance;}));
            }
            c[count].handle = e->handle;
            c[count].distance = dist;
            count++;
            // create table for this hit

        }
    }
    *_count = count;
    return c;
}

int init_ability(game* game, int entity_handle, int ability_index, ability_kind ak) {
    ability_init_handler a = get_ability(ak);
    ability i = a(game, entity_handle);
    i.active = 1;
    find_entity(game, entity_handle)->abilities[ability_index] = i;
    return 1;
}

entity* init_and_add_entity(game* game, float h, char* path) {
    entity e;
    init_entity(&e, h, path);
    return game_new_entity(game, e);
}

int main(void) {

    // init env, window and lua
    load_env(".env");
    InitWindow(winw, winh, "Hello Raylib");
    // SetTargetFPS(60);
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    // register functions
    // register_engine(L);

    // load map
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
    game.elements_cap = MAX_ELEMENTS;
    game.elements = calloc(1,game.elements_cap*sizeof(element));
    for (int i = 0; i < game.elements_cap; i++) {
        game.elements[i].handle = 0;
        game.elements[i].active = 0;
        game.elements[i].instance_ref = LUA_NOREF;
        game.elements[i].ref = LUA_NOREF;
        game.elements[i].pos = _vec(0,0);
    }

    Arena a = arena_new(1025, sizeof(entity));
    int ecount = 0; // entities count
    new_dynarr(entity, _entities);
    game.gpa = a;

    game.entities = _entities;
    // take reference to player
    entity _player;
    if (!init_entity(&_player, 80, getenv("PLAYER_PATH"))) panic("Failed to init player");
    entity* player = game_new_entity(&game, _player);

    game.player = player;
    player->update = basic_entity_update;
    player->draw = basic_entity_draw;
    game.player_handle = player->handle;
    player->atk = 50;
    player->health = 230;
    player->max_health = 230;
    init_ability(&game, game.player_handle,0, ability_kind_test);
    init_ability(&game, game.player_handle,1, ability_kind_big_boy);

    printf("Player :%s\n", getenv("PLAYER_PATH"));
    /*
    entity* enemy = init_and_add_entity(&game,
            90.0f,getenv("ENEMY1"));
    enemy->atk = 50;
    enemy->health = 230;
    enemy->max_health = 230;
    enemy->body.x += PLAYER_SPEED;
    enemy->body.y += PLAYER_SPEED;
    enemy = init_and_add_entity(&game,
            95.0f,getenv("ENEMY2"));
    enemy->atk = 50;
    enemy->health = 230;
    enemy->max_health = 230;
    enemy->body.x  += -100;
    enemy->body.y  += 150;
    enemy = init_and_add_entity(&game,
            200.0f,getenv("ENEMY3"));
    enemy->atk = PLAYER_SPEED;
    enemy->health = 1030;
    enemy->max_health = 1030;
    enemy->body.x += -100;
    enemy->body.y += -350;*/
    entity* enemy = entity_init_basic_enemy(&game);
    enemy->body.x  += -100;
    enemy->body.y  += 150;
    enemy = entity_init_basic_enemy(&game);
    enemy->body.x += PLAYER_SPEED;
    enemy->body.y += PLAYER_SPEED;
    enemy = entity_init_basic_enemy(&game);
    enemy->body.x += -100;
    enemy->body.y += -350;
    enemy = entity_init_sweetie(&game);
    enemy->body.x += 500;
    enemy->body.y += 500;
    game.log_cap = 10;
    game.log_count = 0;
    game.logs = malloc(game.log_cap*sizeof(char*));


    int CLICK_TO_MOVE = 0;
    int range_v = (int)winh/(cellh)/2+2;
    int range_h = (int)winw/(cellw)/2+2;

    dbg("Player index %d.", game.player_handle);
    int draw_logs = 1;
    game.dt = 0;
    double last = get_time();
    rect camera;
    game.camera= &camera;
    vec2  player_dest = rect_pos(player->body);
    game.move_to_dest = 0;
    while (!WindowShouldClose()) {
        // dt
        double now = get_time();
        double dt = now - last;
        last = now;
        game.dt = dt;
        game.mouse_pos = GetMousePosition();
        player->direction = vec2norm(vec2sub(game.mouse_pos,
                    _vec((float)winw/2, (float)winh/2)));
        if (CLICK_TO_MOVE) {
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                player_dest = unapply_camera(game.mouse_pos, *game.camera);

                // sub half the body to get center, so player moves center there
                player_dest = vec2sub(player_dest,
                        vec2scale(rect_size(player->body), 0.5));
                game.move_to_dest= 1;

            }
            float dd = vec2len(vec2sub(rect_pos(player->body), player_dest));
            if (game.move_to_dest) {
                if (dd >= 0.01) {
                    vec2 change = vec2scale(vec2norm(
                                vec2sub(player_dest,rect_pos(player->body))
                                ), PLAYER_SPEED*(float)dt);
                    if (vec2len(change) >= dd) {
                        // if it's greater than distance, set to distance
                        change = vec2scale(vec2norm(change), dd);
                        game.move_to_dest = 0;
                    }
                    player->body.x += change.x;
                    player->body.y += change.y;
                } else {
                    player->body.x = player_dest.x;
                    player->body.y = player_dest.y;
                    game.move_to_dest = 0;
                }
            }
        } else {
            vec2 ds = {0};
            if (IsKeyDown(KEY_W)) {
                ds.y -= 1;
            }
            if (IsKeyDown(KEY_S)) {
                ds.y += 1;
            }
            if (IsKeyDown(KEY_A)) {
                ds.x -= 1;
            }
            if (IsKeyDown(KEY_D)) {
                ds.x += 1;
            }
            if (ds.x != 0 || ds.y != 0) {
                // playerdest = playerpos + norm(ds)*speed*dt
                vec2 dest = vec2add(vec2scale(vec2norm(ds),
                            PLAYER_SPEED*(float)dt), rect_pos(player->body));
                player->body.x = dest.x;
                player->body.y = dest.y;
            }
        }
        int k = 0;
        while ((k = GetKeyPressed())) {
            if (k == KEY_SPACE) {
            } else if (k == KEY_I) {
                printf("Lua memory: %d KB\n", lua_gc(L, LUA_GCCOUNT));
            } else if (k == KEY_F) {
                game_log(&game, "Welp!");
            } else if (k == KEY_TAB) {
                draw_logs = !draw_logs;
            } else if (k == KEY_ONE) {
                entity_ability(&game, game.player_handle, 0);
            } else if (k == KEY_TWO) {
                entity_ability(&game, game.player_handle, 1);
            } else if (k == KEY_THREE) {
                entity_ability(&game, game.player_handle, 2);
            } else if (k == KEY_FOUR) {
                entity_ability(&game, game.player_handle, 3);
            } else if (k == KEY_E) {
                entity_ability(&game, game.player_handle, 4);
            } else if (k == KEY_X) {
                CloseWindow();
            } else if (k == KEY_U) {
                CLICK_TO_MOVE = !CLICK_TO_MOVE;
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {// left click
            entity_ability(&game, game.player_handle, 0);
        }
        if (IsKeyDown(KEY_SPACE)) {// left click
            entity_ability(&game, game.player_handle, 0);
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

            for (int i = x-range_h < 0 ? 0 : x-range_h; i <= x+range_h && i < map.cols; i++) {
                for (int j = y-range_v < 0 ? 0 : y-range_v; j <= y+range_v && j < map.rows; j++) {
                    if (i < 0 || j < 0) continue;
                    vec2 draw = apply_camera(_vec(i*cellw, j*cellh),
                            *game.camera);
                    if (map.tiles[j*map.cols+i].n < 0 ||
                            map.tiles[j*map.cols+i].n >= 3) {
                        panic("more/less than 3/0 map %d (ij %d %d)",
                            map.tiles[j*map.cols+i].n, i, j);
                    }
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
            entity* e = game.entities.data[i];
            // update entity abilities
            for (int i = 0; i < MAX_ABILITIES; i++) {

                ability a = e->abilities[i];
                if (!a.active) { continue; }
                if (!a.update(&game, a.payload)) {
                    panic("Ability update failed.");
                }
            }
            if (!e->update) {
                panic("No entity update funcrion. %d", e->handle);
            } else
                e->update(&game, e->handle, e->payload);
            // draw_entity(e, camera);
            if (!e->draw) {
                panic("No entity draw funcrion. %d", e->handle);
            } else
                e->draw(&game, e->handle, e->payload);
        }
        for (int i = 0; i < game.elements_cap; i++) {
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
        char b[1024];

        snprintf(b, 1024,"Lua memory    : %d KB\n"
                "dt                     : %.10lf\n"
                "player (to move %d) x/y: %.0f, %.0f\n"
                "movement mode          : %s\n"
                "elements               : %d\n",
                lua_gc(L, LUA_GCCOUNT),
                dt, game.move_to_dest, player->body.x, player->body.y,
                CLICK_TO_MOVE ? "click to move" : "wasd",
                global_elements_count
                );
        DrawText(b, 10, 50, 20, WHITE);
        // draw abilities
        // start at bottom row
        float y = winh-10-ABILITY_UI_SIZE;
        for (int i = 0; i < MAX_ABILITIES; i++) {
            int j = i%(MAX_ABILITIES/2); // row
            int k = i/(MAX_ABILITIES/2); // col
            ability ability =
                find_entity(&game, game.player_handle)->abilities[i];
            float xoffset = 10 + j*(ABILITY_UI_SIZE+10);
            // add sub this if k == 1 (for cells after fifth).
            // to have next five above first five
            float yoffset = y -k*(10+ABILITY_UI_SIZE);
            if (!ability.active) {
                DrawRectangle(xoffset, yoffset,
                        ABILITY_UI_SIZE, ABILITY_UI_SIZE,
                        GetColor(0x444444ff));
            } else {
                float cd = ability.cooldown;
                Texture t = ability.texture;
                DrawTexturePro(t,_rect(0,0,t.width,t.height),
                        _rect(xoffset, yoffset,
                            ABILITY_UI_SIZE, ABILITY_UI_SIZE),
                        _vec(0,0),0,WHITE);
                if (cd > 0) {
                    float cdt = ability.cooldown_time;
                    // draw cooldown
                    DrawRectangle(xoffset, yoffset,
                            ABILITY_UI_SIZE, ABILITY_UI_SIZE,
                            GetColor(0x44444444));
                    DrawRectangle(xoffset, yoffset,
                            ABILITY_UI_SIZE, ABILITY_UI_SIZE*cd/cdt,
                            GetColor(0x111111aa));
                }
            }
        }
        EndDrawing();
        for (int i = 0; i < game.entities.count; i++) {
            entity* e = game.entities.data[i];
            if (e->status == status_dead || e->status == status_die) {
                if (e->handle == game.player_handle) {
                    dbg("Player dead.");
                    exit(0);
                }
                if (!entities_remove_entity(&game.entities, i)) {
                    panic("null entity in remove.");
                    exit(-1);
                }
                i-=1;
            }
        }
    }
    for (int i = 0; i < game.entities.count; i++) {
        UnloadTexture(game.entities.data[i]->image);
    }
    for (int i = 0; i < game.loaded_count; i++) {
        free(game.loaded_scripts[i].path);
    }
    free(game.logs);
    free(game.entities.data);
    free(game.elements);
    game.entities.data = 0;
    arena_free(&a);
    arena_free(&game.cycle_arena);
    arena_free(&game.chat_arena);
    free(map.tiles);
    lua_close(L);
    return 0;
}
