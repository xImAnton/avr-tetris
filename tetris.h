#include "led_matrix.h"
#include "random.h"
/*
#define TET_I 0
#define TET_J 1
#define TET_L 2
#define TET_O 3
#define TET_S 4
#define TET_T 5
#define TET_Z 6

typedef struct {
    uint8_t x_pos;
    uint8_t y_pos;
    uint8_t rotation;
    uint8_t type;
} tetromino_t;

void tet_init_tetromino(tetromino_t *t, rand_state_t *random) {
    uint32_t random_data = rand_xorshift32(random);

    t->rotation = random_data & 0x7;
    // we only have 4 rotation states, so we correct it if its above 3
    if (t->rotation > 3) {
        t->rotation = 7 - t->rotation;
    }

    t->type = random_data & 0x38 >> 3;
    // we have 7 types of stones (0..6), so we set it to a valid stone, when
    // its above 6
    if (t->type == 7) {
        t->type = TET_O;
    }
    t->x_pos = (random_data & 0x1C0) >> 6;
    t->y_pos = 0;
}
*/