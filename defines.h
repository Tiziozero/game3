#ifndef DEFINES_H
#define DEFINES_H
#include "raymath.h"
#define vec2zero()               Vector2Zero()
#define vec2one()                Vector2One()
#define vec2add(a, b)            Vector2Add(a, b)
#define vec2addf(v, f)           Vector2AddValue(v, f)
#define vec2sub(a, b)            Vector2Subtract(a, b)
#define vec2subf(v, f)           Vector2SubtractValue(v, f)
#define vec2len(v)               Vector2Length(v)
#define vec2lensqr(v)            Vector2LengthSqr(v)
#define vec2dot(a, b)            Vector2DotProduct(a, b)
#define vec2cross(a, b)          Vector2CrossProduct(a, b)
#define vec2dist(a, b)           Vector2Distance(a, b)
#define vec2distsqr(a, b)        Vector2DistanceSqr(a, b)
#define vec2angle(a, b)          Vector2Angle(a, b)
#define vec2lineangle(s, e)      Vector2LineAngle(s, e)
#define vec2scale(v, f)          Vector2Scale(v, f)
#define vec2mul(a, b)            Vector2Multiply(a, b)
#define vec2neg(v)               Vector2Negate(v)
#define vec2div(a, b)            Vector2Divide(a, b)
#define vec2norm(v)              Vector2Normalize(v)
#define vec2transform(v, m)      Vector2Transform(v, m)
#define vec2lerp(a, b, t)        Vector2Lerp(a, b, t)
#define vec2reflect(v, n)        Vector2Reflect(v, n)
#define vec2min(a, b)            Vector2Min(a, b)
#define vec2max(a, b)            Vector2Max(a, b)
#define vec2rotate(v, a)         Vector2Rotate(v, a)
#define vec2towards(v, t, d)     Vector2MoveTowards(v, t, d)
#define vec2invert(v)            Vector2Invert(v)
#define vec2clamp(v, mn, mx)     Vector2Clamp(v, mn, mx)
#define vec2clampf(v, mn, mx)    Vector2ClampValue(v, mn, mx)
#define vec2eq(a, b)             Vector2Equals(a, b)
#define vec2refract(v, n, r)     Vector2Refract(v, n, r)

#define _rect(a, b, c, d) (rect){(a),(b),(c),(d)}
#define _vec(a, b) (vec2){(a),(b)}
#define rect_pos(r) _vec((r).x, (r).y)
#define rect_size(r) _vec((r).width, (r).height)
#define rect_center(r) _vec((r).x+(r).width/2, (r).y+(r).height/2)
#define dec_dynarr(t) typedef struct { t** data; int count; \
                        int cap; int size; } dynarr_##t;
#define new_dynarr(t, name) dynarr_##t name = {}; name.count = 0; \
                      name.size = sizeof(t*); \
                      name.data = malloc((name.cap = 10) * name.size); \
                      memset(name.data, 0, (name.cap = 10) * name.size);
#endif // DEFINES_H
