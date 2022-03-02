#include "led_matrix.h"
#include "random.h"

#define DEFINE_TETROMINO(name, w, h, m) const tetromino_type_t tetromino_ ## name = { \
    .width = w,                                                                  \
    .height = h,                                                                 \
    .vector = (const uint8_t **) &tet_ ## name,                                  \
    .mirrored = m                                                                \
}

typedef struct {
    uint8_t rotated_variants;
    uint8_t width;
    uint8_t height;
    const uint8_t **vector;
    uint8_t mirrored;
} tetromino_type_t;

const uint8_t tet_1_1[] = {1, 1};
const uint8_t tet_1_1_1[] = {1, 1, 1};
const uint8_t tet_0_1_0[] = {0, 1, 0};
const uint8_t tet_1_0[] = {0, 1};
const uint8_t tet_0_1_1[] = {0, 1, 1};
const uint8_t tet_1_1_0[] = {1, 1, 0};
const uint8_t tet_1_1_1_1[] = {1, 1, 1, 1};

const uint8_t *tet_O[] = {
    (const uint8_t *) &tet_1_1, (const uint8_t *) &tet_1_1
};
DEFINE_TETROMINO(O, 2, 2, 0);

const uint8_t *tet_T[] = {
    (const uint8_t *) &tet_1_1_1, (const uint8_t *) &tet_0_1_0
};
DEFINE_TETROMINO(T, 3, 2, 0);

const uint8_t *tet_L[] = {
    (const uint8_t *) &tet_1_0, (const uint8_t *) &tet_1_0, (const uint8_t *) &tet_1_1
};
DEFINE_TETROMINO(L, 2, 3, 1);

const uint8_t *tet_Z[] = {
    (const uint8_t *) &tet_0_1_1, (const uint8_t *) &tet_1_1_0
};
DEFINE_TETROMINO(Z, 3, 2, 1);

const uint8_t *tet_I[] = {
    (const uint8_t *) &tet_1_1_1_1
};
DEFINE_TETROMINO(I, 4, 1, 0);

const tetromino_type_t *tetromino_types[] = {&tetromino_I, &tetromino_L, &tetromino_O, &tetromino_T, &tetromino_Z};

#define TETROMINO_ROTATION_TOP    0
#define TETROMINO_ROTATION_LEFT   1
#define TETROMINO_ROTATION_BOTTOM 2
#define TETROMINO_ROTATION_RIGHT  3

#define tetromino_is_mirrored(t) (((t)->flags & 0b100000) >> 5)
#define tetromino_get_rotation(t) (((t)->flags & 0b11000) >> 3)
#define tetromino_get_type(t) ((t)->flags & 0b111)
#define tetromino_set_flags(t, type, rotation, mirrored) (t)->flags = (!!(mirrored) << 5) | (((rotation) & 0b11) << 3) | ((type) & 0b111)

typedef struct {
    uint8_t x_pos;
    uint8_t y_pos;
    /*
      Bit 0..2 - Type
      Bit 3..4 - Rotation
      Bit    5 - Mirrored
    */
    uint8_t flags;
} tetromino_t;

void tet_init_tetromino(tetromino_t *t, rand_state_t *random) {
    uint32_t random_data = rand_xorshift32(random);

    uint8_t rotation = random_data & 0b11;
    uint8_t type = (random_data & 0b11100) >> 2;
    if (type > 4) {
        type = 7 - type;
    }
    const tetromino_type_t *tet_type = tetromino_types[type];

    tetromino_set_flags(
        t,
        type,
        rotation,
        tet_type->mirrored ? (random_data & 0b100000) >> 5 : 0
    );

    t->x_pos = (random_data & 0x1C0) >> 6;
    t->y_pos = 0;
}
