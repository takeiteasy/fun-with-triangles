/* bla.h -- https://github.com/takeiteasy/bla

 bla -- Basic Linear Algebra library

 Copyright (C) 2024  George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.

 Functionality relies on Clang + GCC extensions.
 When building `-fenable-matrix` for Matrix support.
 To disable Matrix support define `BLA_NO_MATRICES`

 Acknowledgements:
 - A lot of this was hand translated from raylib's raymath.h header
 https://github.com/raysan5/raylib/blob/master/src/raymath.h (Zlib) */

#if !defined(__clang__) && !(defined(__GNUC__) || defined(__GNUG__))
#define BLA_NO_MATRICES
#endif

#ifdef __has_extension
#if !__has_extension(c_generic_selections)
#define BLA_NO_GENERICS
#endif
#else
#if !defined(__STDC__) || !__STDC__ || __STDC_VERSION__ < 201112L
#define BLA_NO_GENERICS
#endif
#endif

#ifndef BLA_HEADER
#define BLA_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#ifndef BLA_NO_PRINT
#include <stdio.h>
#endif
#if !defined(__STDC__) || !__STDC__ || __STDC_VERSION__ < 199901L || defined(_MSC_VER)
#if defined(_MSC_VER)
#include <windef.h>
#define bool BOOL
#define true 1
#define false 0
#else
typedef enum bool { false = 0, true = !false } bool;
#endif
#else
#include <stdbool.h>
#endif
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#define _PI 3.14159265358979323846264338327950288
#define _TWO_PI 6.28318530717958647692 // 2 * pi
#define _TAU _TWO_PI
#define _HALF_PI 1.57079632679489661923132169163975144     // pi / 2
#define _QUARTER_PI 0.785398163397448309615660845819875721 // pi / 4
#define _PHI 1.61803398874989484820
#define _INV_PHI 0.61803398874989484820
#define _EULER 2.71828182845904523536
#define _EPSILON 0.000001f
#define _SQRT2 1.41421356237309504880168872420969808

#define _BYTES(n) (n)
#define _KILOBYTES(n) (n << 10)
#define _MEGABYTES(n) (n << 20)
#define _GIGABYTES(n) (((uint64_t)n) << 30)
#define _TERABYTES(n) (((uint64_t)n) << 40)

#define _THOUSAND(n) ((n) * 1000)
#define _MILLION(n) ((n) * 1000000)
#define _BILLION(n) ((n) * 1000000000LL)

#define _MIN(A, B) ((A) < (B) ? (A) : (B))
#define _MAX(A, B) ((A) > (B) ? (A) : (B))
#define _CLAMP(VALUE, MINVAL, MAXVAL) (_MIN(_MAX((VALUE), (MINVAL)), (MAXVAL)))
#define _TO_DEGREES(RADIANS) ((RADIANS) * (180.f / PI))
#define _TO_RADIANS(DEGREES) ((DEGREES) * (PI / 180.f))

#define __BLA_TYPES \
    X(2)            \
    X(3)            \
    X(4)

#define X(SZ)                                                   \
    typedef float vec##SZ __attribute__((ext_vector_type(SZ))); \
    vec##SZ vec##SZ##_zero(void);                               \
    bool vec##SZ##_is_zero(vec##SZ);                            \
    float vec##SZ##_sum(vec##SZ);                               \
    int vec##SZ##_cmp(vec##SZ, vec##SZ);                        \
    float vec##SZ##_length_sqr(vec##SZ);                        \
    float vec##SZ##_length(vec##SZ);                            \
    float vec##SZ##_dot(vec##SZ, vec##SZ);                      \
    vec##SZ vec##SZ##_normalize(vec##SZ);                       \
    float vec##SZ##_distance_sqr(vec##SZ, vec##SZ);             \
    float vec##SZ##_distance(vec##SZ, vec##SZ);                 \
    vec##SZ vec##SZ##_clamp(vec##SZ, vec##SZ, vec##SZ);         \
    vec##SZ vec##SZ##_lerp(vec##SZ, vec##SZ, float);
__BLA_TYPES
#undef X
typedef vec4 quat;
typedef vec4 color;

#ifndef BLA_NO_GENERICS
#endif

#define X(N) \
    typedef int vec##N##i __attribute__((ext_vector_type(N)));
__BLA_TYPES
#undef X

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next) MAP_NEXT1(MAP_GET_END test, next)

#define MAP0(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP1)(f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP0)(f, peek, __VA_ARGS__)

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next) MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST1)(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST0)(f, peek, __VA_ARGS__)

#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define MAP_LIST(f, ...) EVAL(MAP_LIST1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define MAP_LIST0_UD(f, userdata, x, peek, ...) f(x, userdata) MAP_LIST_NEXT(peek, MAP_LIST1_UD)(f, userdata, peek, __VA_ARGS__)
#define MAP_LIST1_UD(f, userdata, x, peek, ...) f(x, userdata) MAP_LIST_NEXT(peek, MAP_LIST0_UD)(f, userdata, peek, __VA_ARGS__)

#define MAP_LIST_UD(f, userdata, ...) EVAL(MAP_LIST1_UD(f, userdata, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define SWZL_MAP(V, VEC) VEC.V
#define SWZL_MAP(V, VEC) VEC.V
#define swizzle(V, ...) {MAP_LIST_UD(SWZL_MAP, V, __VA_ARGS__)}

// Taken from: https://gist.github.com/61131/7a22ac46062ee292c2c8bd6d883d28de
#define N_ARGS(...) _NARG_(__VA_ARGS__, _RSEQ())
#define _NARG_(...) _SEQ(__VA_ARGS__)
#define _SEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,_125,_126,_127,N,...) N
#define _RSEQ() 127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

#define GETMACRO___(NAME, N) NAME##N
#define GETMACRO__(NAME, N) GETMACRO___(NAME, N)
#define GETMACRO_(FUNC, ...) GETMACRO__(FUNC, N_ARGS(__VA_ARGS__)) (__VA_ARGS__)
#define Vec(...) GETMACRO_(Vec, __VA_ARGS__)
#define Vec2(x, y) (vec2){x, y}
#define Vec3(x, y, z) (vec3){x, y, z}
#define Vec4(x, y, z, w) (vec4){x, y, z, w}

#ifndef BLA_NO_MATRICES
#define __DEF_BLA_MATRIX(COLUMNS, ROWS) \
    typedef float mat##COLUMNS##ROWS __attribute__((matrix_type((COLUMNS), (ROWS))));

#define X(N)                                      \
    __DEF_BLA_MATRIX(N, N)                        \
    typedef mat##N##N mat##N;                     \
    mat##N Mat##N(void);                          \
    mat##N mat##N##_identity(void);               \
    bool mat##N##_is_identity(mat##N mat);        \
    bool mat##N##_is_zero(mat##N mat);            \
    float mat##N##_trace(mat##N mat);             \
    mat##N mat##N##_transpose(mat##N);            \
    vec##N mat##N##_column(mat##N, unsigned int); \
    vec##N mat##N##_row(mat##N, unsigned int);
__BLA_TYPES
#undef X
#endif

#ifndef BLA_NO_PRINT
#ifndef BLA_NO_MATRICES
#define X(SZ) void mat##SZ##_print(mat##SZ);
__BLA_TYPES
#undef X
#endif // BLA_NO_MATRICES
#define X(SZ) void vec##SZ##_print(vec##SZ);
__BLA_TYPES
#undef X

#ifndef BLA_NO_GENERICS
#ifndef BLA_NO_MATRICES
#define bla_print(data) _Generic((data), \
    vec2: vec2_print,                    \
    vec3: vec3_print,                    \
    vec4: vec4_print,                    \
    mat2: mat2_print,                    \
    mat3: mat3_print,                    \
    mat4: mat4_print,                    \
    default: printf("Unsupported type\n"))(data)
#else
#define bla_print(data) _Generic((data), \
    vec2: vec2_print,                    \
    vec3: vec3_print,                    \
    vec4: vec4_print,                    \
    default: printf("Unsupported type\n"))(data)
#endif // BLA_NO_MATRICES
#endif // BLA_NO_GENERICS
#endif // BLA_NO_PRINT

float vec2_angle(vec2 v1, vec2 v2);
vec2 vec2_rotate(vec2 v, float angle);
vec2 vec2_move_towards(vec2 v, vec2 target, float maxDistance);
vec2 vec2_reflect(vec2 v, vec2 normal);
float vec2_cross(vec2 a, vec2 b);

vec3 vec3_reflect(vec3 v, vec3 normal);
vec3 vec3_cross(vec3 v1, vec3 v2);
vec3 vec3_perpendicular(vec3 v);
float vec3_angle(vec3 v1, vec3 v2);
vec3 quat_rotate_vec3(vec3 v, quat q);
vec3 vec3_rotate_axis_angle(vec3 v, vec3 axis, float angle);
vec3 vec3_refract(vec3 v, vec3 n, float r);
vec3 vec3_barycenter(vec3 p, vec3 a, vec3 b, vec3 c);

quat quat_identity(void);
quat quat_mul(quat q1, quat q2);
quat quat_invert(quat q);
quat quat_to_from_vec3(vec3 from, vec3 to);
quat quat_from_axis_angle(vec3 axis, float angle);
void quat_to_axis_angle(quat q, vec3 *outAxis, float *outAngle);
quat quat_from_euler(float pitch, float yaw, float roll);
vec3 quat_to_euler(quat q);
int quat_cmp(quat p, quat q);

#ifndef BLA_NO_MATRICES
vec3 vec3_transform(vec3 v, mat4 mat);
vec3 vec3_unproject(vec3 source, mat4 projection, mat4 view);
float mat4_determinant(mat4 mat);

quat quat_from_mat4(mat4 mat);
mat4 mat4_from_quat(quat q);
quat quat_transform(quat q, mat4 mat);

mat4 mat4_invert(mat4 mat);
mat4 mat4_translate(vec3 v);
mat4 mat4_rotate(vec3 axis, float angle);
mat4 mat4_scale(vec3 scale);
mat4 frustum(float left, float right, float bottom, float top, float near, float far);
mat4 perspective(float fovY, float aspect, float nearPlane, float farPlane);
mat4 ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
mat4 look_at(vec3 center, vec3 target, vec3 up);
#endif

#if defined(__cplusplus)
}
#endif
#endif // BLA_HEADER

#ifdef BLA_IMPLEMENTATION
static bool float_cmp(float a, float b) {
    return fabsf(a - b) <= _EPSILON * fmaxf(1.f, fmaxf(fabsf(a), fabsf(b)));
}

#ifndef BLA_NO_MATRICES
#define X(N)                                                \
    mat##N Mat##N(void)                                     \
    {                                                       \
        mat##N mat;                                         \
        memset(&mat, 0, sizeof(float) * (N * N));           \
        return mat;                                         \
    }                                                       \
    mat##N mat##N##_identity(void)                          \
    {                                                       \
        mat##N mat = Mat##N();                              \
        for (int i = 0; i < N; i++)                         \
            mat[i][i] = 1.f;                                \
        return mat;                                         \
    }                                                       \
    bool mat##N##_is_identity(mat##N mat)                   \
    {                                                       \
        for (int y = 0; y < N; y++)                         \
            for (int x = 0; x < N; x++)                     \
                if (x == y) {                               \
                    if (mat[y][x] != 1.f)                   \
                        return false;                       \
                } else {                                    \
                    if (mat[y][x] != 0.f)                   \
                        return false;                       \
                }                                           \
        return true;                                        \
    }                                                       \
    bool mat##N##_is_zero(mat##N mat)                       \
    {                                                       \
        for (int y = 0; y < N; y++)                         \
            for (int x = 0; x < N; x++)                     \
                if (mat[y][x] != 0.f)                       \
                    return false;                           \
        return true;                                        \
    }                                                       \
    float mat##N##_trace(mat##N mat)                        \
    {                                                       \
        float result = 0.f;                                 \
        for (int i = 0; i < N; i++)                         \
            result += mat[i][i];                            \
        return result;                                      \
    }                                                       \
    mat##N mat##N##_transpose(mat##N mat)                   \
    {                                                       \
        mat##N result = Mat##N();                           \
        for (int x = 0; x < N; x++)                         \
            for (int y = 0; y < N; y++)                     \
                result[x][y] = mat[y][x];                   \
        return result;                                      \
    }                                                       \
    vec##N mat##N##_column(mat##N mat, unsigned int column) \
    {                                                       \
        vec##N result = vec##N##_zero();                    \
        if (column >= N)                                    \
            return result;                                  \
        for (int i = 0; i < N; i++)                         \
            result[i] = mat[i][column];                     \
        return result;                                      \
    }                                                       \
    vec##N mat##N##_row(mat##N mat, unsigned int row)       \
    {                                                       \
        vec##N result = vec##N##_zero();                    \
        if (row >= N)                                       \
            return result;                                  \
        for (int i = 0; i < N; i++)                         \
            result[i] = mat[row][i];                        \
        return result;                                      \
    }
__BLA_TYPES
#undef X
#endif // BLA_NO_MATRICES

#define X(N)                                                  \
    vec##N vec##N##_zero(void)                                \
    {                                                         \
        return (vec##N){0};                                   \
    }                                                         \
    bool vec##N##_is_zero(vec##N vec)                         \
    {                                                         \
        for (int i = 0; i < N; i++)                           \
            if (vec[i] != 0.f)                                \
                return false;                                 \
                return true;                                  \
    }                                                         \
    float vec##N##_sum(vec##N vec)                            \
    {                                                         \
        float result = 0.f;                                   \
        for (int i = 0; i < N; i++)                           \
            result += vec[i];                                 \
        return result;                                        \
    }                                                         \
    int vec##N##_cmp(vec##N a, vec##N b)                      \
    {                                                         \
        int result = 1;                                       \
        for (int i = 0; i < N; i++)                           \
            if (!float_cmp(a[i], b[i]))                       \
                return 0;                                     \
        return result;                                        \
    }                                                         \
    float vec##N##_length_sqr(vec##N vec)                     \
    {                                                         \
        return vec##N##_sum(vec * vec);                       \
    }                                                         \
    float vec##N##_length(vec##N vec)                         \
    {                                                         \
        return sqrtf(vec##N##_length_sqr(vec));               \
    }                                                         \
    float vec##N##_dot(vec##N a, vec##N b)                    \
    {                                                         \
        return vec##N##_sum(a * b);                           \
    }                                                         \
    vec##N vec##N##_normalize(vec##N vec)                     \
    {                                                         \
        vec##N result = vec##N##_zero();                      \
        float L = vec##N##_length(vec);                       \
        for (int i = 0; i < N; i++)                           \
            result[i] = vec[i] * (1.f / L);                   \
        return result;                                        \
    }                                                         \
    float vec##N##_distance_sqr(vec##N a, vec##N b)           \
    {                                                         \
        return vec##N##_length_sqr(b - a);                    \
    }                                                         \
    float vec##N##_distance(vec##N a, vec##N b)               \
    {                                                         \
        return sqrtf(vec##N##_distance_sqr(a, b));            \
    }                                                         \
    vec##N vec##N##_clamp(vec##N vec, vec##N min, vec##N max) \
    {                                                         \
        vec##N result = vec##N##_zero();                      \
        for (int i = 0; i < N; i++)                           \
            result[i] = _CLAMP(vec[i], min[i], max[i]);       \
        return result;                                        \
    }                                                         \
    vec##N vec##N##_lerp(vec##N a, vec##N b, float t)         \
    {                                                         \
        return a + t * (b - a);                               \
    }
__BLA_TYPES
#undef X

#ifndef BLA_NO_PRINT
#ifndef BLA_NO_MATRICES
#define X(N)                                \
    void mat##N##_print(mat##N mat)         \
    {                                       \
        for (int x = 0; x < N; x++)         \
        {                                   \
            printf("| ");                   \
            for (int y = 0; y < N; y++)     \
            {                               \
                printf("%.2f ", mat[y][x]); \
            }                               \
            puts("|");                      \
        }                                   \
    }
__BLA_TYPES
#undef X
#define X(N)                        \
    void vec##N##_print(vec##N vec) \
    {                               \
        printf("{ ");               \
        for (int i = 0; i < N; i++) \
            printf("%f ", vec[i]);  \
        puts("}");                  \
    }
__BLA_TYPES
#undef X
#endif // BLA_NO_MATRICES
#endif // BLA_NO_PRINT

float vec2_angle(vec2 v1, vec2 v2) {
    return atan2f(v2.y, v2.x) - atan2f(v1.y, v1.x);
}

vec2 vec2_rotate(vec2 v, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    return (vec2) { v.x*c - v.y*s,  v.x*s + v.y*c };
}

vec2 vec2_move_towards(vec2 v, vec2 target, float maxDistance) {
    vec2 delta = target - v;
    float dist = vec2_length_sqr(delta);
    if (!dist || (maxDistance >= 0 && dist <= maxDistance * maxDistance))
        return target;
    else
        return v + (delta / (sqrtf(dist) * maxDistance));
}

vec2 vec2_reflect(vec2 v, vec2 normal) {
    return v - (2.f * normal) * vec2_dot(v, normal);
}

float vec2_cross(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}

vec3 vec3_reflect(vec3 v, vec3 normal) {
    return v - (2.f * normal) * vec3_dot(v, normal);
}

vec3 vec3_cross(vec3 v1, vec3 v2) {
    return (vec3) { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
}

vec3 vec3_perpendicular(vec3 v) {
    float min = (float) fabs(v.x);
    vec3 a = {1.0f, 0.0f, 0.0f};
    if (fabsf(v.y) < min) {
        min = (float) fabs(v.y);
        a = (vec3){0.0f, 1.0f, 0.0f};
    }
    if (fabsf(v.z) < min)
        a = (vec3){0.0f, 0.0f, 1.0f};
    return (vec3) { v.y*a.z - v.z*a.y,
                    v.z*a.x - v.x*a.z,
                    v.x*a.y - v.y*a.x };
}

float vec3_angle(vec3 v1, vec3 v2) {
    return atan2f(vec3_length(vec3_cross(v1, v2)), vec3_dot(v1, v2));
}

vec3 quat_rotate_vec3(vec3 v, quat q) {
    return (vec3) {
        v.x*(q.x*q.x + q.w*q.w - q.y*q.y - q.z*q.z) + v.y*(2*q.x*q.y - 2*q.w*q.z) + v.z*(2*q.x*q.z + 2*q.w*q.y),
        v.x*(2*q.w*q.z + 2*q.x*q.y) + v.y*(q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z) + v.z*(-2*q.w*q.x + 2*q.y*q.z),
        v.x*(-2*q.w*q.y + 2*q.x*q.z) + v.y*(2*q.w*q.x + 2*q.y*q.z)+ v.z*(q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z)
    };
}

vec3 vec3_rotate_axis_angle(vec3 v, vec3 axis, float angle) {
    // Using Euler-Rodrigues Formula
    // Ref.: https://en.wikipedia.org/w/index.php?title=Euler%E2%80%93Rodrigues_formula

    axis *= (1.0f / (_CLAMP(vec3_length(axis), 0.f, 1.f)));

    angle /= 2.0f;
    vec3 w = axis * sinf(angle);
    vec3 wv = vec3_cross(w, v);
    return v + (wv * (cosf(angle) * 2.f)) + (vec3_cross(w, wv) * 2.f);
}

vec3 vec3_refract(vec3 v, vec3 n, float r) {
    float dot = vec3_dot(v, n);
    float d = 1.0f - r*r*(1.0f - dot*dot);
    return d < 0 ? vec3_zero() : r * v - (r * dot + sqrtf(d)) * n;
}

vec3 vec3_barycenter(vec3 p, vec3 a, vec3 b, vec3 c) {
    vec3 v0 = b - a;
    vec3 v1 = c - a;
    vec3 v2 = p - a;
    float d00 = vec3_dot(v0, v1);
    float d01 = vec3_dot(v1, v1);
    float d11 = vec3_dot(v1, v1);
    float d20 = vec3_dot(v2, v0);
    float d21 = vec3_dot(v2, v1);
    float denom = d00*d11 - d01*d01;
    float y = (d11*d20 - d01*d21)/denom;
    float z = (d00*d21 - d01*d20)/denom;
    return (vec3) {1.0f - (z + y), y, z};
}

quat quat_zero(void) {
    return (quat){0};
}

quat quat_identity(void) {
    return (quat){0.f, 0.f, 0.f, 1.f};
}

quat quat_mul(quat q1, quat q2) {
    float qax = q1.x, qay = q1.y, qaz = q1.z, qaw = q1.w;
    float qbx = q2.x, qby = q2.y, qbz = q2.z, qbw = q2.w;
    return (quat) {
        qax*qbw + qaw*qbx + qay*qbz - qaz*qby,
        qay*qbw + qaw*qby + qaz*qbx - qax*qbz,
        qaz*qbw + qaw*qbz + qax*qby - qay*qbx,
        qaw*qbw - qax*qbx - qay*qby - qaz*qbz
    };
}

quat quat_invert(quat q) {
    float lsqr = vec4_length_sqr(q);
    quat result = q;
    if (lsqr != 0.f) {
        float inv = 1.f / lsqr;
        result *= (vec4){-inv, -inv, -inv, inv};
    }
    return result;
}

quat quat_to_from_vec3(vec3 from, vec3 to) {
    vec3 cross = vec3_cross(from, to);
    return vec4_normalize((quat){cross.x, cross.y, cross.z, 1.f + vec3_dot(from, to)});
}

quat quat_from_mat4(mat4 mat) {
    float fourWSquaredMinus1 = mat[0][0] + mat[1][1] + mat[2][2];
    float fourXSquaredMinus1 = mat[0][0] - mat[1][1] - mat[2][2];
    float fourYSquaredMinus1 = mat[1][1] - mat[0][0] - mat[2][2];
    float fourZSquaredMinus1 = mat[2][2] - mat[0][0] - mat[1][1];

    int biggestIndex = 0;
    float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
    if (fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourXSquaredMinus1;
        biggestIndex = 1;
    }
    if (fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourYSquaredMinus1;
        biggestIndex = 2;
    }
    if (fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourZSquaredMinus1;
        biggestIndex = 3;
    }

    float biggestVal = sqrtf(fourBiggestSquaredMinus1 + 1.f) * .5f;
    float mult = .25f / biggestVal;
    switch (biggestIndex)
    {
        case 0:
            return (quat) {
                (mat[2][1] - mat[1][2]) * mult,
                (mat[0][2] - mat[2][0]) * mult,
                (mat[1][0] - mat[0][1]) * mult,
                biggestVal
            };
            break;
        case 1:
            return (quat) {
                biggestVal,
                (mat[1][0] + mat[0][1]) * mult,
                (mat[0][2] + mat[2][0]) * mult,
                (mat[2][1] - mat[1][2]) * mult
            };
        case 2:
            return (quat) {
                biggestVal,
                (mat[1][0] + mat[0][1]) * mult,
                (mat[0][2] + mat[2][0]) * mult,
                (mat[2][1] - mat[1][2]) * mult
            };
        case 3:
            return (quat) {
                biggestVal,
                (mat[1][0] + mat[0][1]) * mult,
                (mat[0][2] + mat[2][0]) * mult,
                (mat[2][1] - mat[1][2]) * mult
            };
        default:
            return quat_zero();
    }
}

vec3 vec3_unproject(vec3 source, mat4 projection, mat4 view) {
    quat p = quat_transform((quat){source.x, source.y, source.z, 1.f }, mat4_invert(view * projection));
    return (vec3) {
        p.x / p.w,
        p.y / p.w,
        p.z / p.w
    };
}

#ifndef BLA_NO_MATRICES
vec3 vec3_transform(vec3 v, mat4 mat) {
    return (vec3) {
        mat[0][0]*v.x + mat[0][1]*v.y + mat[0][2]*v.z + mat[0][3],
        mat[1][0]*v.x + mat[1][1]*v.y + mat[1][2]*v.z + mat[1][3],
        mat[2][0]*v.x + mat[2][1]*v.y + mat[2][2]*v.z + mat[2][3]
    };
}

mat4 mat4_from_quat(quat q) {
    float a2 = q.x*q.x;
    float b2 = q.y*q.y;
    float c2 = q.z*q.z;
    float ac = q.x*q.z;
    float ab = q.x*q.y;
    float bc = q.y*q.z;
    float ad = q.w*q.x;
    float bd = q.w*q.y;
    float cd = q.w*q.z;

    mat4 result = mat4_identity();
    result[0][0] = 1 - 2*(b2 + c2);
    result[1][0] = 2*(ab + cd);
    result[2][0] = 2*(ac - bd);

    result[0][1] = 2*(ab - cd);
    result[1][1] = 1 - 2*(a2 + c2);
    result[2][1] = 2*(bc + ad);

    result[0][2] = 2*(ac + bd);
    result[1][2] = 2*(bc - ad);
    result[2][2] = 1 - 2*(a2 + b2);
    return result;
}

quat quat_from_axis_angle(vec3 axis, float angle) {
    float axisLength = vec3_length(axis);
    if (axisLength == 0.f)
        return quat_identity();

    axis = vec3_normalize(axis);
    angle *= .5f;
    float sinres = sinf(angle);
    float cosres = cosf(angle);
    return (quat) {
        axis.x*sinres,
        axis.y*sinres,
        axis.z*sinres,
        cosres
    };
}

void quat_to_axis_angle(quat q, vec3 *outAxis, float *outAngle) {
    if (fabsf(q.w) > 1.0f)
        q = vec4_normalize(q);
    float resAngle = 2.0f*acosf(q.w);
    float den = sqrtf(1.0f - q.w*q.w);
    vec3 qxyz = (vec3){q.x, q.y, q.z};
    vec3 resAxis = den > _EPSILON ? qxyz / den : (vec3){1.f, 0.f, 0.f};
    *outAxis = resAxis;
    *outAngle = resAngle;
}

quat quat_from_euler(float pitch, float yaw, float roll) {
    float x0 = cosf(pitch*0.5f);
    float x1 = sinf(pitch*0.5f);
    float y0 = cosf(yaw*0.5f);
    float y1 = sinf(yaw*0.5f);
    float z0 = cosf(roll*0.5f);
    float z1 = sinf(roll*0.5f);

    return (quat) {
        x1*y0*z0 - x0*y1*z1,
        x0*y1*z0 + x1*y0*z1,
        x0*y0*z1 - x1*y1*z0,
        x0*y0*z0 + x1*y1*z1
    };
}

vec3 quat_to_euler(quat q) {
    // Roll (x-axis rotation)
    float x0 = 2.0f*(q.w*q.x + q.y*q.z);
    float x1 = 1.0f - 2.0f*(q.x*q.x + q.y*q.y);
    // Pitch (y-axis rotation)
    float y0 = 2.0f*(q.w*q.y - q.z*q.x);
    y0 = y0 > 1.0f ? 1.0f : y0;
    y0 = y0 < -1.0f ? -1.0f : y0;
    // Yaw (z-axis rotation)
    float z0 = 2.0f*(q.w*q.z + q.x*q.y);
    float z1 = 1.0f - 2.0f*(q.y*q.y + q.z*q.z);
    return (vec3) {
        atan2f(x0, x1),
        asinf(y0),
        atan2f(z0, z1)
    };
}

quat quat_transform(quat q, mat4 mat) {
    return (quat) {
        mat[0][0]*q.x + mat[0][1]*q.y + mat[0][2]*q.z + mat[0][3]*q.w,
        mat[1][0]*q.x + mat[1][1]*q.y + mat[1][2]*q.z + mat[1][3]*q.w,
        mat[2][0]*q.x + mat[2][1]*q.y + mat[2][2]*q.z + mat[2][3]*q.w,
        mat[3][0]*q.x + mat[3][1]*q.y + mat[3][2]*q.z + mat[3][3]*q.w
    };
}

int quat_cmp(quat p, quat q) {
    return (((fabsf(p.x - q.x)) <= (_EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) && ((fabsf(p.y - q.y)) <= (_EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) && ((fabsf(p.z - q.z)) <= (_EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) && ((fabsf(p.w - q.w)) <= (_EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w)))))) || (((fabsf(p.x + q.x)) <= (_EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) && ((fabsf(p.y + q.y)) <= (_EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) && ((fabsf(p.z + q.z)) <= (_EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) && ((fabsf(p.w + q.w)) <= (_EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w))))));
}

float mat4_determinant(mat4 mat) {
    return mat[0][3]*mat[1][2]*mat[2][1]*mat[3][0] - mat[0][2]*mat[1][3]*mat[2][1]*mat[3][0] - mat[0][3]*mat[1][1]*mat[2][2]*mat[3][0] + mat[0][1]*mat[1][3]*mat[2][2]*mat[3][0] + mat[0][2]*mat[1][1]*mat[2][3]*mat[3][0] - mat[0][1]*mat[1][2]*mat[2][3]*mat[3][0] - mat[0][3]*mat[1][2]*mat[2][0]*mat[3][1] + mat[0][2]*mat[1][3]*mat[2][0]*mat[3][1] + mat[0][3]*mat[1][0]*mat[2][2]*mat[3][1] - mat[0][0]*mat[1][3]*mat[2][2]*mat[3][1] - mat[0][2]*mat[1][0]*mat[2][3]*mat[3][1] + mat[0][0]*mat[1][2]*mat[2][3]*mat[3][1] + mat[0][3]*mat[1][1]*mat[2][0]*mat[3][2] - mat[0][1]*mat[1][3]*mat[2][0]*mat[3][2] - mat[0][3]*mat[1][0]*mat[2][1]*mat[3][2] + mat[0][0]*mat[1][3]*mat[2][1]*mat[3][2] + mat[0][1]*mat[1][0]*mat[2][3]*mat[3][2] - mat[0][0]*mat[1][1]*mat[2][3]*mat[3][2] - mat[0][2]*mat[1][1]*mat[2][0]*mat[3][3] + mat[0][1]*mat[1][2]*mat[2][0]*mat[3][3] + mat[0][2]*mat[1][0]*mat[2][1]*mat[3][3] - mat[0][0]*mat[1][2]*mat[2][1]*mat[3][3] - mat[0][1]*mat[1][0]*mat[2][2]*mat[3][3] + mat[0][0]*mat[1][1]*mat[2][2]*mat[3][3];
}

mat4 mat4_invert(mat4 mat) {
    float b00 = mat[0][0]*mat[1][1] - mat[1][0]*mat[0][1];
    float b01 = mat[0][0]*mat[2][1] - mat[2][0]*mat[0][1];
    float b02 = mat[0][0]*mat[3][1] - mat[3][0]*mat[0][1];
    float b03 = mat[1][0]*mat[2][1] - mat[2][0]*mat[1][1];
    float b04 = mat[1][0]*mat[3][1] - mat[3][0]*mat[1][1];
    float b05 = mat[2][0]*mat[3][1] - mat[3][0]*mat[2][1];
    float b06 = mat[0][2]*mat[1][3] - mat[1][2]*mat[0][3];
    float b07 = mat[0][2]*mat[2][3] - mat[2][2]*mat[0][3];
    float b08 = mat[0][2]*mat[3][3] - mat[3][2]*mat[0][3];
    float b09 = mat[1][2]*mat[2][3] - mat[2][2]*mat[1][3];
    float b10 = mat[1][2]*mat[3][3] - mat[3][2]*mat[1][3];
    float b11 = mat[2][2]*mat[3][3] - mat[3][2]*mat[2][3];
    float invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    mat4 result = Mat4();
    result[0][0] = (mat[1][1]*b11 - mat[2][1]*b10 + mat[3][1]*b09)*invDet;
    result[1][0] = (-mat[1][0]*b11 + mat[2][0]*b10 - mat[3][0]*b09)*invDet;
    result[2][0] = (mat[1][3]*b05 - mat[2][3]*b04 + mat[3][3]*b03)*invDet;
    result[3][0] = (-mat[1][2]*b05 + mat[2][2]*b04 - mat[3][2]*b03)*invDet;
    result[0][1] = (-mat[0][1]*b11 + mat[2][1]*b08 - mat[3][1]*b07)*invDet;
    result[1][1] = (mat[0][0]*b11 - mat[2][0]*b08 + mat[3][0]*b07)*invDet;
    result[2][1] = (-mat[0][3]*b05 + mat[2][3]*b02 - mat[3][3]*b01)*invDet;
    result[3][1] = (mat[0][2]*b05 - mat[2][2]*b02 + mat[3][2]*b01)*invDet;
    result[0][2] = (mat[0][1]*b10 - mat[1][1]*b08 + mat[3][1]*b06)*invDet;
    result[1][2] = (-mat[0][0]*b10 + mat[1][0]*b08 - mat[3][0]*b06)*invDet;
    result[2][2] = (mat[0][3]*b04 - mat[1][3]*b02 + mat[3][3]*b00)*invDet;
    result[3][2] = (-mat[0][2]*b04 + mat[1][2]*b02 - mat[3][2]*b00)*invDet;
    result[0][3] = (-mat[0][1]*b09 + mat[1][1]*b07 - mat[2][1]*b06)*invDet;
    result[1][3] = (mat[0][0]*b09 - mat[1][0]*b07 + mat[2][0]*b06)*invDet;
    result[2][3] = (-mat[0][3]*b03 + mat[1][3]*b01 - mat[2][3]*b00)*invDet;
    result[3][3] = (mat[0][2]*b03 - mat[1][2]*b01 + mat[2][2]*b00)*invDet;
    return result;
}

mat4 mat4_translate(vec3 v) {
    mat4 result = mat4_identity();
    result[0][3] = v.x;
    result[1][3] = v.y;
    result[2][3] = v.z;
    return result;
}

mat4 mat4_rotate(vec3 axis, float angle) {
    vec3 a = vec3_length_sqr(axis);
    float s = sinf(angle);
    float c = cosf(angle);
    float t = 1.f - c;

    mat4 result = mat4_identity();
    result[0][0] = a.x*a.x*t + c;
    result[1][0] = a.y*a.x*t + a.z*s;
    result[2][0] = a.z*a.x*t - a.y*s;
    result[0][1] = a.x*a.y*t - a.z*s;
    result[1][1] = a.y*a.y*t + c;
    result[2][1] = a.z*a.y*t + a.x*s;
    result[0][2] = a.x*a.z*t + a.y*s;
    result[1][2] = a.y*a.z*t - a.x*s;
    result[2][2] = a.z*a.z*t + c;
    return result;
}

mat4 mat4_scale(vec3 scale) {
    mat4 result = Mat4();
    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;
    result[3][3] = 1.f;
    return result;
}

mat4 frustum(float left, float right, float bottom, float top, float near, float far) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;

    mat4 result = Mat4();
    result[0][0] = (near*2.0f)/rl;
    result[1][1] = (near*2.0f)/tb;
    result[0][2] = (right + left)/rl;
    result[1][2] = (top + bottom)/tb;
    result[2][2] = -(far + near)/fn;
    result[3][2] = -1.0f;
    result[2][3] = -(far*near*2.0f)/fn;
    return result;
}

mat4 perspective(float fovY, float aspect, float nearPlane, float farPlane) {
    float top = nearPlane*tan(fovY*0.5);
    float bottom = -top;
    float right = top*aspect;
    float left = -right;
    float rl = right - left;
    float tb = top - bottom;
    float fn = farPlane - nearPlane;

    mat4 result = Mat4();
    result[0][0] = (nearPlane*2.0f)/rl;
    result[1][1] = (nearPlane*2.0f)/tb;
    result[0][2] = (right + left)/rl;
    result[1][2] = (top + bottom)/tb;
    result[2][2] = -(farPlane + nearPlane)/fn;
    result[3][2] = -1.0f;
    result[2][3] = -(farPlane*nearPlane*2.0f)/fn;
    return result;
}

mat4 ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = farPlane - nearPlane;

    mat4 result = Mat4();
    result[0][0] = 2.0f/rl;
    result[1][1] = 2.0f/tb;
    result[2][2] = -2.0f/fn;
    result[0][3] = -(left + right)/rl;
    result[1][3] = -(top + bottom)/tb;
    result[2][3] = -(farPlane + nearPlane)/fn;
    result[3][3] = 1.0f;
    return result;
}

mat4 look_at(vec3 eye, vec3 target, vec3 up) {
    vec3 vz = vec3_normalize(eye - target);
    vec3 vx = vec3_normalize(vec3_cross(up, vz));
    vec3 vy = vec3_cross(vz, vx);

    mat4 result = Mat4();
    result[0][0] = vx.x;
    result[1][0] = vy.x;
    result[2][0] = vz.x;
    result[0][1] = vx.y;
    result[1][1] = vy.y;
    result[2][1] = vz.y;
    result[0][2] = vx.z;
    result[1][2] = vy.z;
    result[2][2] = vz.z;
    result[0][3] = -vec3_dot(vx, eye);
    result[1][3] = -vec3_dot(vy, eye);
    result[2][3] = -vec3_dot(vz, eye);
    result[3][3] = 1.0f;
    return result;
}
#endif
#endif // BLA_IMPLEMENTATION
