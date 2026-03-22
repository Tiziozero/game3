#ifndef GAME_H
#define GAME_H
#include "logger.h"
#include "raylib.h"
#include "defines.h"
#include "utils.h"
#include <stdint.h>
#include "external/lua/lua.h"
#include "external/lua/lauxlib.h"
#include "external/lua/lualib.h"
// big consts
#define cellw 3*16
#define cellh 3*16
#define srccellw 16
#define srccellh 16
#define MAX_ELEMENTS 100
#define MAX_ABILITIES 10
#define MAX_TEXTURES 100
#define winh 750
#define winw 1200
#define PLAYER_SPEED 300
#define ABILITY_UI_SIZE 60

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

void dbg_table(lua_State* L, int index);

typedef Vector2 vec2;
typedef Rectangle rect;


dec_dynarr(void);


typedef struct {
    char* name;
    int active;
    int ref, instance;
    Texture texture;
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
typedef struct {
    int active, handle, instance_ref, ref;
    uint16_t generation;   // increments every reuse
    vec2 pos;
} element; // like projectiles and what not
dec_dynarr(element);
    // Map: script path -> LUA_REF
typedef struct {
    char* path;
    int ref; // luaL_ref to the loaded chunk/table
} script_entry;
typedef struct {
    int n;
} tile;
typedef struct map map;
struct map {
    tile* tiles;
    int rows,cols;
};

typedef struct {
    dynarr_entity entities;
    vec2 mouse_pos;
    entity* player; // player
    int player_handle; // player
    element* elements;
    int elements_cap;

    int energy;
    int eps; // energy recovered per second.

    Arena cycle_arena; // arena that gets reset every cycle for temporary stuff

    Arena chat_arena; // arena for messages etc
    char** logs; // circular buffer
    int log_count; // at which to start
    int log_cap; // at which to start

    double dt;
    // player movement
    int move_to_dest;
    
    rect* camera;

    Texture textures[MAX_TEXTURES];
    int texture_count;
    script_entry loaded_scripts[128]; // or dynamic map
    int loaded_count;
    lua_State* L;
} game;

int run(game* game);






int projectile_body_hit(int entity, vec2 p, float r, rect body, vec2 prev_pos, float* dist);
int load_env(const char *filename);

int dynarr_add(void* ptr, void* data) ;
vec2 apply_camera(vec2 world_pos, rect camera) ;
vec2 unapply_camera(vec2 screen_pos, rect camera);
int draw_entity(entity* e, rect camera);
int init_entity(entity* ent, float h, char* path);
double get_time();
void sort_entities_by_y(entity **arr, int n);
entity* entities_new_entity(Arena* a, dynarr_entity* entities, float h, char* path);
entity* entities_remove_entity(dynarr_entity* entities, int index);
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
element* get_element(game* g, int handle);
int game_get_or_load_script(game* game, const char* path);
int game_log(game*game, char*msg);
int _game_new_element(
    element elements[100], element e);
int game_spawn_element(game* g, int ref, int instance);
void remove_element(game* g, int handle);

game* _getsetgame(int set, game* g);
game* setgame(game* game);
game* getgame();
entity* find_entity(game* g, int handle);
float damage_target(float total_atk,entity* e);
int l_damage_entity(lua_State* L);
bool entities_overlap(entity** entities, int count, int* out_a, int* out_b);
int update(game* game);
float point_rect_dist(vec2 p, rect r);
int l_projectile_body_hit(lua_State*L);
int element_update(game* g, element* e);
int element_draw(game* g, element* e);
bool point_in_rect(vec2 p, rect r);
bool circle_rect_intersect(vec2 c, float r, rect rect);
// helper: line segment intersection
bool line_intersect(vec2 p1, vec2 p2, vec2 q1, vec2 q2);

bool line_rect_intersect(vec2 p1, vec2 p2, rect r);
int ability_act(lua_State* L, int script_ref, int instance_ref);
int entity_ability(game* game, int handle, int ability);
float point_segment_dist(vec2 p, vec2 a, vec2 b);
float line_segment_shortest_dist(vec2 a, vec2 b, vec2 c, vec2 d);
float line_rect_shortest_dist(vec2 p1, vec2 p2, rect body);
// with updated position and prev
int projectile_body_hit(int entity, vec2 p, float r, rect body, vec2 prev_pos, float* out_dist);

static entity* check_entity_handle(lua_State* L, int index);
int l_engine_get_entity(lua_State* L);
int l_engine_set_entity_pos(lua_State* L);
int l_engine_damage_entity(lua_State* L);
int l_engine_entity_alive(lua_State* L);
int l_engine_get_entity_center(lua_State* L);
int l_engine_move_entity(lua_State* L);
int l_engine_entity_distance(lua_State* L);
int l_engine_get_dt(lua_State* L);
int l_engine_get_mouse_pos_world(lua_State* L);
int l_engine_entity_damage(lua_State* L);

int script_init(lua_State* L, int script_ref, int owner_handle);
int l_engine_new_projectile(lua_State* L);
int l_engine_apply_camera(lua_State* L);
int l_engine_draw_rect(lua_State* L);
int l_engine_draw_line(lua_State* L);
int l_engine_draw_circle(lua_State* L);
int l_engine_draw_image(lua_State* L);
int l_engine_draw_text(lua_State* L);
// screen pos
int l_engine_get_mouse_pos_screen(lua_State* L);
int l_game_get_element(lua_State* L);

int l_engine_remove_element(lua_State* L);
int l_engine_panic(lua_State* L);
int l_engine_element_set_xy(lua_State* L);

int l_engine_element_get_xy(lua_State* L);
int l_engine_get_entities_line_intersect(lua_State* L);
void register_engine(lua_State* L);
void dbg_table(lua_State* L, int index);
int script_init(lua_State* L, int script_ref, int owner_handle);
// Pushes the function from a script table by registry ref
// Returns 1 on success (function pushed), 0 on failure
int get_lua_script_function(lua_State* L, int script_ref, const char* func_name);
// instance metatable contains script ref
int ability_act(lua_State* L, int script_ref, int instance_ref);
int game_get_or_load_script(game* game, const char* path);
int init_ability(game* game, int entity_handle, int ability, char* script);
float ability_get_cooldown(lua_State* L, int instance_ref);


int get_int_from_table(lua_State* L, int instance_ref, const char* index);
float get_float_from_table(lua_State* L, int instance_ref, const char* index);
const char* get_string_from_table(lua_State* L, int instance_ref, const char* index);
int get_bool_from_table(lua_State* L, int instance_ref, const char* index);




#endif // GAME_H
