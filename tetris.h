#include "led_screen.h"
#include "random.h"
#include "tetrominos.h"

#ifndef __tetris_h_included__
#define __tetris_h_included__

#define TETROMINO_ROTATION_TOP    0
#define TETROMINO_ROTATION_LEFT   1
#define TETROMINO_ROTATION_BOTTOM 2
#define TETROMINO_ROTATION_RIGHT  3

typedef struct {
    uint8_t x_pos;
    uint8_t y_pos;
    tetromino_type_t *type;
    uint8_t rotation;
} tetromino_t;

void tet_init_tetromino(tetromino_t *t, rand_state_t *random, screen_t *s) {
    uint32_t random_data = rand_xorshift32(random);

    uint8_t rotation = random_data & 0b11;
    rotation = TETROMINO_ROTATION_TOP;
    uint8_t type = (random_data & 0b11100) >> 2;
    if (type == 7) {
        type = 0;
    }
    const tetromino_type_t *tet_type = tetromino_types[type];

    t->type = (tetromino_type_t*) tet_type;
    t->rotation = rotation;

    t->x_pos = (random_data & 0x1C0) >> 6;
    if (t->x_pos + tet_type->width > (s->width * 4)) {
        t->x_pos = s->width - 1;
    }

    t->y_pos = 0;
}

#define asc(_var, _max) (int8_t _var = 0; _var < _max; _var++)
#define desc(_var, _start) (int8_t _var = _start; _var >= 0; _var--)

void tet_draw_tetromino(screen_t *s, tetromino_t *t) {
    const tetromino_type_t *type = t->type;
    uint8_t screen_x = t->x_pos;
    uint8_t screen_y = t->y_pos;

    switch (t->rotation) {
        case TETROMINO_ROTATION_TOP:
            for asc(x, type->width) {
                for asc(y, type->height) {
                    if (tet_pixel_is_set(type, x, y)) {
                        screen_set(s, screen_x, screen_y);
                    }
                    screen_y++;
                }
                screen_x++;
                screen_y = t->y_pos;
            }
            break;
        case TETROMINO_ROTATION_LEFT:
            for desc(x, type->width - 1) {                
                for asc(y, type->height) {
                    if (tet_pixel_is_set(type, x, y)) {
                        screen_set(s, screen_x, screen_y);
                    }
                    screen_x++;
                }
                screen_y++;
                screen_x = t->x_pos;
            }
            break;
        case TETROMINO_ROTATION_BOTTOM:
            for desc(x, type->width - 1) {
                for desc(y, type->height - 1) {
                    if (tet_pixel_is_set(type, x, y)) {
                        screen_set(s, screen_x, screen_y);
                    }
                    screen_y++;
                }
                screen_x++;
                screen_y = t->y_pos;
            }
            break;
        case TETROMINO_ROTATION_RIGHT:
            for asc(x, type->width) {
                for desc(y, type->height - 1) {
                    if (tet_pixel_is_set(type, x, y)) {
                        screen_set(s, screen_x, screen_y);
                    }
                    screen_x++;
                }
                screen_y++;
                screen_x = t->x_pos;
            }
            break;
    }
}

void tetromino_rotate(tetromino_t *t) {
    t->rotation++;
    t->rotation &= 0b11;
}

#endif // not __tetris_h_included__
