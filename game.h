#ifndef GAME_H
#define GAME_H
#include "logger.h"
#include "raylib.h"
#include "defines.h"
#include "utils.h"
#include <stdint.h>
// abilities
#define dec_ability(action) \
    action(test) \
    action(big_boy)

#define dec_entity(action) \
    action(basic_enemy) \
    action(sweetie) \

// big consts
#define cellw 3*16
#define cellh 3*16
#define srccellw 16
#define srccellh 16
#define MAX_ELEMENTS 1024
#define MAX_ABILITIES 10
#define MAX_TEXTURES 100
#define _winh 750
#define _winw 1200
#define screenw _winw
#define screenh _winh
#define PLAYER_SPEED 300
#define ABILITY_UI_SIZE 60
char* get_env(char* path);

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
typedef enum ability_kind ability_kind;
typedef enum entity_kind entity_kind;


typedef Vector2 vec2;
typedef Rectangle rect;
typedef struct game game;


dec_dynarr(void);


typedef struct {
    char* name;
    int active;
    int ref, instance;
    Texture2D texture;
} lua_item;

// ability index
typedef void(* init_handler)(game* game, int index, int owner_handle);
typedef int (*ability_act_handler)(game* game, void* payload);

typedef struct {
    int active;
    char* name;
    char* description;
    void* payload;
    Texture2D texture;
    // call for payload
    // init_handler init;
    ability_act_handler act;
    ability_act_handler update; // dt and what not
    ability_act_handler draw;
} item;
typedef struct {
    int active;
    char* name;
    char* description;
    void* payload;
    Texture2D texture;
    double cooldown, cooldown_time;
    // call for payload
    // init_handler init;
    ability_act_handler act, update, draw; // dt and what not
} ability;

typedef enum {
    status_on = 1,
    status_die, // when dying
    status_dead = 0,
}entity_status;

// handle for entities and elements
typedef int (*entity_act_handler)(game* game, int handle, void* payload);
typedef struct {
    entity_status status;
    int handle;
    Texture2D image;
    rect body; // pos/destination
    vec2 direction;
    double atk, def, health, max_health; // def:multuplier for damage
    ability abilities[MAX_ELEMENTS];
    void* payload;
    entity_act_handler update, draw;
} entity;
dec_dynarr(entity);

// handle for entities and elements
typedef int (*element_act_handler)(game* game, int handle, void* payload);
typedef struct {
    int active, handle, instance_ref, ref;
    uint16_t generation;   // increments every reuse
    vec2 pos;
    void* payload;
    // init_handler init;
    element_act_handler update, draw;
} element; // like projectiles and what not
dec_dynarr(element);

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

struct  game{
    dynarr_entity entities;
    
    vec2 mouse_pos; // screen pos
    entity* player; // player
    int player_handle; // player
    element* elements;
    int elements_cap;

    int energy;
    int eps; // energy recovered per second.

    Arena gpa; // general purpose arena
    Arena cycle_arena; // arena that gets reset every cycle for temporary stuff

    Arena chat_arena; // arena for messages etc
    char** logs; // circular buffer
    int log_count; // at which to start
    int log_cap; // at which to start

    // dt
    double dt;
    // player movement
    int move_to_dest;
    
    rect* camera;

    Texture2D textures[MAX_TEXTURES];
    int texture_count;
    script_entry loaded_scripts[128]; // or dynamic map
    int loaded_count;
    // lua_State* L;
};

int run(game* game);






int projectile_body_hit(vec2 p, float r, rect body, vec2 prev_pos, float* dist);
int load_env(const char *filename);

int dynarr_add(void* ptr, void* data) ;
vec2 apply_camera(vec2 world_pos, rect camera) ;
vec2 unapply_camera(vec2 screen_pos, rect camera);
int draw_entity(entity* e, rect camera);
int init_entity(entity* ent, float h, char* path);
double get_time();
void sort_entities_by_y(entity **arr, int n);
entity* game_new_entity(game* game, entity new_e);
entity* entities_remove_entity(dynarr_entity* entities, int index);
// int get_lua_script_function(lua_State* L, int script_ref, const char* func_name);
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
element* game_spawn_element(game* g, element _new);
void remove_element(game* g, int handle);

game* _getsetgame(int set, game* g);
game* setgame(game* game);
game* getgame();
entity* find_entity(game* g, int handle);
float damage_target(float total_atk,entity* e);
// int l_damage_entity(lua_State* L);
bool entities_overlap(entity** entities, int count, int* out_a, int* out_b);
int update(game* game);
float point_rect_dist(vec2 p, rect r);
// int l_projectile_body_hit(lua_State*L);
int element_update(game* g, element* e);
int element_draw(game* g, element* e);
bool point_in_rect(vec2 p, rect r);
bool circle_rect_intersect(vec2 c, float r, rect rect);
// helper: line segment intersection
bool line_intersect(vec2 p1, vec2 p2, vec2 q1, vec2 q2);

bool line_rect_intersect(vec2 p1, vec2 p2, rect r);
int ability_act(game* game, ability ability);
int entity_ability(game* game, int handle, int ability);
float point_segment_dist(vec2 p, vec2 a, vec2 b);
float line_segment_shortest_dist(vec2 a, vec2 b, vec2 c, vec2 d);
float line_rect_shortest_dist(vec2 p1, vec2 p2, rect body);
// with updated position and prev
// static entity* check_entity_handle(lua_State* L, int index);
// int l_engine_get_entity(lua_State* L);
// int l_engine_set_entity_pos(lua_State* L);
// int l_engine_damage_entity(lua_State* L);
// int l_engine_entity_alive(lua_State* L);
// int l_engine_get_entity_center(lua_State* L);
// int l_engine_move_entity(lua_State* L);
// int l_engine_entity_distance(lua_State* L);
// int l_engine_get_dt(lua_State* L);
// int l_engine_get_mouse_pos_world(lua_State* L);
// int l_engine_entity_damage(lua_State* L);

// int script_init(lua_State* L, int script_ref, int owner_handle);
// int l_engine_new_projectile(lua_State* L);
// int l_engine_apply_camera(lua_State* L);
// int l_engine_draw_rect(lua_State* L);
// int l_engine_draw_line(lua_State* L);
// int l_engine_draw_circle(lua_State* L);
// int l_engine_draw_image(lua_State* L);
// int l_engine_draw_text(lua_State* L);
// screen pos
// int l_engine_get_mouse_pos_screen(lua_State* L);
// int l_game_get_element(lua_State* L);

// int l_engine_remove_element(lua_State* L);
// int l_engine_panic(lua_State* L);
// int l_engine_element_set_xy(lua_State* L);
// 
// int l_engine_element_get_xy(lua_State* L);
// int l_engine_get_entities_line_intersect(lua_State* L);
// void register_engine(lua_State* L);
// void dbg_table(lua_State* L, int index);
// int script_init(lua_State* L, int script_ref, int owner_handle);
// Pushes the function from a script table by registry ref
// Returns 1 on success (function pushed), 0 on failure
// int get_lua_script_function(lua_State* L, int script_ref, const char* func_name);
// instance metatable contains script ref
// int l_ability_act(lua_State* L, int script_ref, int instance_ref);
int l_init_ability(game* game, int entity_handle, int ability, char* script);
int game_get_or_load_script(game* game, const char* path);
// float ability_get_cooldown(lua_State* L, int instance_ref);


// int get_int_from_table(lua_State* L, int instance_ref, const char* index);
// float get_float_from_table(lua_State* L, int instance_ref, const char* index);
// const char* get_string_from_table(lua_State* L, int instance_ref, const char* index);
// int get_bool_from_table(lua_State* L, int instance_ref, const char* index);

// collisions
typedef struct{int handle; float distance;} collisions_ret_t;
collisions_ret_t* entities_line_intersect
                                (vec2 p1, vec2 p2, float r, int* _count);
element* game_alloc_element(game* g);
int init_ability(game* game, int entity_handle, int ability_index, ability_kind ak);
int entity_ability(game* game, int handle, int ability);
static inline int default_draw(game* game, void* payload) {
    return 1;
}
int basic_entity_draw(game* game, int handle, void* payload);


#define a(n) ability_init_##n
#define ab(n) ability  a(n)(game * game, int index, int owner_handle);
#define b(n) ability_kind_##n
#define bc(n) b(n),
dec_ability(ab)
enum ability_kind{
    dec_ability(bc)
    ability_kind_count,
};
#define c(n) if (k == b(n)) return a(n);
typedef ability (*ability_init_handler)(game* game, int index, int handle);

#define HANDLER_ENTRY(n) ability_init_##n,
static ability_init_handler ability_table[] = {
    dec_ability(HANDLER_ENTRY)
};
static inline ability_init_handler get_ability(ability_kind k) {
    if (k < 0 || k >= ability_kind_count)
        panic("Unknown ability");
    return ability_table[k];
}
int proj_test_init(game * game, int owner_handle, element* e,
        vec2 pos, vec2 dir, float r, float speed);
int big_boy_boom_init(game * game, int owner_handle, element* e,
        vec2 pos, vec2 dir, float r, float speed);
#undef HANDLER_ENTRY
#undef a
#undef ab
#undef b
#undef bc
#undef c

// entities stuff
#define a(n) entity_init_##n
#define ab(n) entity*  a(n)(game * game);
#define b(n) entity_kind_##n
#define bc(n) b(n),
dec_entity(ab)
enum entity_kind{
    dec_ability(bc)
    entity_kind_count,
};
#define c(n) if (k == b(n)) return a(n);
typedef entity* (*entity_init_handler)(game* game);


#define HANDLER_ENTRY(n) entity_init_##n,
static entity_init_handler entity_table[] = {
    dec_entity(HANDLER_ENTRY)
};
static inline entity_init_handler get_entity(entity_kind k) {
    if (k < 0 || k >= entity_kind_count)
        panic("Unknown entity");
    return entity_table[k];
}
int proj_test_init(game * game, int owner_handle, element* e,
        vec2 pos, vec2 dir, float r, float speed);
int big_boy_boom_init(game * game, int owner_handle, element* e,
        vec2 pos, vec2 dir, float r, float speed);
#undef a
#undef ab
#undef b
#undef bc
#undef c
#endif // GAME_H
