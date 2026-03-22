#include "external/lua/lua.h"
#include "game.h"
#include "raylib.h"


int get_int_from_table(lua_State* L, int instance_ref, const char* index) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, instance_ref); // push table
    lua_getfield(L, -1, index);
    int x = lua_tointeger(L, -1);
    lua_pop(L, 1); // pop x
    lua_pop(L, 1); // pop table
    return x;
}
float get_float_from_table(lua_State* L, int instance_ref, const char* index) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, instance_ref); // push table
    lua_getfield(L, -1, index);
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1); // pop x
    lua_pop(L, 1); // pop table
    return x;
}
const char* get_string_from_table(lua_State* L, int instance_ref, const char* index) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, instance_ref); // push table
    lua_getfield(L, -1, index);
    const char* x = lua_tostring(L, -1);
    lua_pop(L, 1); // pop x
    lua_pop(L, 1); // pop table
    return x;
}
int get_bool_from_table(lua_State* L, int instance_ref, const char* index) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, instance_ref); // push table
    lua_getfield(L, -1, index);
    int x = lua_toboolean(L, -1);
    lua_pop(L, 1); // pop x
    lua_pop(L, 1); // pop table
    return x;
}
// gets integer from arg
static entity* check_entity_handle(lua_State* L, int index) {
    int handle = luaL_checkinteger(L, index);

    game* g = getgame();
    if (!g) {
        panic("No game.");
        return 0;
    }
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
    if (!e) {
        panic("No e l_engine_set_entity_pos");
        return 0;
    }

    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    info("New pos %f %f.", x, y);

    e->body.x = x;
    e->body.y = y;
    if (e->handle == getgame()->player_handle)
        getgame()->move_to_dest = 0; // interupt player movement

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

    lua_pushnumber(L,g->dt);
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
    lua_getfield(L, 1, "r");
    float r = luaL_optnumber(L, -1, 0.0f);
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

    lua_getfield(L, 1, "range");
    float range = luaL_optnumber(L, -1, 0.0f);
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
    // call init_projectile
    if (!get_lua_script_function(L, script_ref, "init_projectile")) {
        panic("script has no init_projectile function.");
        return 0;
    }
    // push self, x, y, r, handle
    lua_rawgeti(L, LUA_REGISTRYINDEX, self_ref); // stack: script
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, dir.x);
    lua_pushnumber(L, dir.y);
    lua_pushnumber(L, speed);
    lua_pushnumber(L, r);
    lua_pushnumber(L, range);
    lua_pushinteger(L,element_handle); // 9 args

    if (lua_pcall(L, 9, 0, 0) != LUA_OK) {
        panic("failed to set handle: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        
    }
    info("called init prijectile.");

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

    Texture2D tex = getgame()->textures[idx];

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
    dbg("REMOVED ELEMENT.");

    return 0;
}
int l_engine_panic(lua_State* L) {
    panic("0");
    return 0;
}
int l_engine_element_set_xy(lua_State* L) {
    int handle = luaL_checkinteger(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    element* e = get_element(getgame(), handle);
    if (!e) return luaL_error(L, "invalid element handle");

    e->pos.x = x;
    e->pos.y = y;

    return 0;
}

int l_engine_element_get_xy(lua_State* L) {
    int handle = luaL_checkinteger(L, 1);

    
    element* e = get_element(getgame(), handle);
    if (!e) {
        lua_pushnil(L);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnumber(L, e->pos.x);
    lua_pushnumber(L, e->pos.y);
    return 2;
}
int l_engine_get_entities_line_intersect(lua_State* L) {
    int handle = luaL_checknumber(L, 1);
    double x1 = luaL_checknumber(L, 2);
    double y1 = luaL_checknumber(L, 3);
    double x2 = luaL_checknumber(L, 4);
    double y2 = luaL_checknumber(L, 5);
    double r = luaL_checknumber(L, 6);

    lua_newtable(L);  // top of stack: results table
    int lua_index = 1;

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
        if (projectile_body_hit(handle, _vec(x1, y1),
                    r, e->body, _vec(x2, y2), &dist)) {
            // create table for this hit
            lua_newtable(L);                  // stack: results, hit_table
            lua_pushinteger(L, e->handle);
            lua_setfield(L, -2, "handle");    // hit_table.handle = e->handle
            lua_pushnumber(L, dist);
            lua_setfield(L, -2, "distance");  // hit_table.distance = dist

            // insert hit_table into results array
            lua_rawseti(L, -2, lua_index++);
        }
    }
    return 1;
}
int l_engine_entity_damage(lua_State* L) {
    int handle = luaL_checknumber(L, 1);
    double damage = luaL_checknumber(L, 2);
    damage_target(damage, find_entity(getgame(),handle));
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

    lua_pushcfunction(L, l_engine_draw_circle);
    lua_setfield(L, -2, "draw_circle");

    lua_pushcfunction(L, l_engine_element_get_xy);
    lua_setfield(L, -2, "element_get_xy");

    lua_pushcfunction(L, l_engine_element_set_xy);
    lua_setfield(L, -2, "element_set_xy");

    lua_pushcfunction(L, l_engine_get_entities_line_intersect);
    lua_setfield(L, -2, "entities_line_intersect");
    lua_pushcfunction(L, l_engine_entity_damage);
    lua_setfield(L, -2, "entity_damage");

    lua_setglobal(L, "engine");
}
float ability_get_cooldown(lua_State* L, int instance_ref) {
    // Push instance table
    lua_rawgeti(L, LUA_REGISTRYINDEX, instance_ref); // stack: instance

    // Get field "current_cooldown"
    lua_getfield(L, -1, "current_cooldown"); // stack: instance, current_cooldown

    float cd = 0.0f;
    if (lua_isnumber(L, -1)) {
        cd = lua_tonumber(L, -1);
    }

    lua_pop(L, 2); // pop current_cooldown and instance
    return cd;
}
