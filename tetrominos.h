#include "libavr.h"

#ifndef __tetrominos_h_included__
#define __tetrominos_h_included__

typedef struct {
    uint8_t width;
    uint8_t height;
    const uint8_t **vector;
} tetromino_type_t;

#define tet_pixel_is_set(type, x, y) (type)->vector[y][x]
#define DEFINE_TETROMINO(name, w, h) const tetromino_type_t tetromino_ ## name = { \
    .width = w,                                                                       \
    .height = h,                                                                      \
    .vector = (const uint8_t **) &tet_ ## name                                        \
}

const uint8_t tet_10[]  = { 1, 0, 0 };
const uint8_t tet_010[] = { 0, 1, 0 };
const uint8_t tet_001[] = { 0, 0, 1 };
const uint8_t tet_110[] = { 1, 1, 0 };
const uint8_t tet_01[]  = { 0, 1, 1 };
const uint8_t tet_1[]   = { 1, 1, 1, 1 };

const uint8_t *tet_I[] = {
    (const uint8_t *) &tet_1
};
DEFINE_TETROMINO(I, 4, 1);

const uint8_t *tet_J[] = {
    (const uint8_t *) &tet_10, (const uint8_t *) &tet_1
};
DEFINE_TETROMINO(J, 3, 2);

const uint8_t *tet_L[] = {
    (const uint8_t *) &tet_001, (const uint8_t *) &tet_1
};
DEFINE_TETROMINO(L, 3, 2);

const uint8_t *tet_O[] = {
    (const uint8_t *) &tet_1, (const uint8_t *) &tet_1
};
DEFINE_TETROMINO(O, 2, 2);

const uint8_t *tet_S[] = {
    (const uint8_t *) &tet_01, (const uint8_t *) &tet_110
};
DEFINE_TETROMINO(S, 3, 2);

const uint8_t *tet_T[] = {
    (const uint8_t *) &tet_010, (const uint8_t *) &tet_1
};
DEFINE_TETROMINO(T, 3, 2);

const uint8_t *tet_Z[] = {
    (const uint8_t *) &tet_110, (const uint8_t *) &tet_01
};
DEFINE_TETROMINO(Z, 3, 2);

const tetromino_type_t *tetromino_types[] = {
    &tetromino_I,
    &tetromino_J,
    &tetromino_L,
    &tetromino_O,
    &tetromino_S,
    &tetromino_T,
    &tetromino_Z
};

#endif // not __tetrominos_h_included__
