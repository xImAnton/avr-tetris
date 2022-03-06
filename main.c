#define RANDOM_USE_DEFAULT

#include <libavr.h>
#include <led_matrix.h>
#include <led_screen.h>
#include <random.h>
#include <event.h>

#include "tetris.h"
#include "matrix_pins.h"

// button event types + state flags
#define BTN_DOWN  0
#define BTN_LEFT  1
#define BTN_UP    2
#define BTN_RIGHT 3

#define TICK_SPEED 30

ledm_t *matrices[] = {&ol, &or, &ml, &mr, &ul, &ur};

screen_t screen = {
    .matrices = matrices,
    .height = 3,
    .width = 2,
};

uint32_t score = 0;
uint8_t speed = TICK_SPEED;
tetromino_t current_tet = {};

ledm_letter_t gameover[] = {LEDM_G, LEDM_A, LEDM_M, LEDM_E, LEDM_SPACE, LEDM_O, LEDM_V, LEDM_E, LEDM_R,
                            LEDM_EXCLAMATION, LEDM_SPACE}; // GAME OVER!

// register for pressed buttons to detect presses / releases
uint8_t button_states = 0;
// times, the timer ISR was executed
volatile uint32_t time = 0;

#define push_event_if_changed(btn) if ((new_states & (1 << (btn))) && !(button_states & (1 << (btn)))) push_event(btn)

ISR(TIMER0_OVF_vect) {
    time++;

    uint8_t new_states = (pin_read_bit(PINB, PB0) << BTN_RIGHT) | \
                         (pin_read_bit(PINA, PA1) << BTN_UP) | \
                         (pin_read_bit(PINB, PB2) << BTN_LEFT) | \
                          pin_read_bit(PINB, PB1) << BTN_DOWN;

    if (new_states != button_states) {
        push_event_if_changed(BTN_RIGHT);
        push_event_if_changed(BTN_UP);
        push_event_if_changed(BTN_LEFT);
        push_event_if_changed(BTN_DOWN);

        button_states = new_states;
    }
}

void update_screen() {
    tet_draw_tetromino(&screen, &current_tet);
}

typedef struct {
    uint8_t new_x;
    uint8_t new_y;
    uint8_t is_occupied;
} move_enum_payload_t;

void check_occupied_proc(uint8_t screen_x, uint8_t screen_y, void *payload) {
    move_enum_payload_t *pl = (move_enum_payload_t *) payload;
    if (screen_is_set(&screen, screen_x + pl->new_x - current_tet.x_pos, screen_y + pl->new_y - current_tet.y_pos)) {
        pl->is_occupied = 1;
    }
}

/**
 * @brief Tries to move to the given position and updates the screen
 * 
 * @param x the x position to move to
 * @param y the y position to move to
 * @return whether the move was successful (1) or not successful because of 
 *         obstruction (0)
 */
uint8_t move(uint8_t new_x, uint8_t new_y) {
    move_enum_payload_t pl = {
        .new_x = new_x,
        .new_y = new_y,
        .is_occupied = 0
    };

    screen_enable_dry_mode(&screen);
    tet_clear_tetromino(&screen, &current_tet);

    tetromino_enum_pixels(&current_tet, &check_occupied_proc, (void *) &pl);

    tet_draw_tetromino(&screen, &current_tet);
    screen_disable_dry_mode(&screen);

    if (pl.is_occupied) {
        // tet_clear_tetromino(&screen, &current_tet);
        return 0;
    }

    tet_clear_tetromino(&screen, &current_tet);
    current_tet.x_pos = new_x;
    current_tet.y_pos = new_y;
    update_screen();

    return 1;
}

uint8_t is_full_row(screen_t *s, uint8_t y) {
    for (uint8_t x = 0; x < s->width * 4; x++) {
        if (!screen_is_set(s, x, y)) return 0;
    }
    return 1;
}

void game_over() {
    screen_clear_all(&screen);
    ledm_show_word_rotating(matrices, 2, ledm_word(gameover), 100, LEDM_LEFT, 0);
    screen_clear_all(&screen);

    // don't try to catch up the missing ticks from showing the message
    time = 0;

    score = 0;
    speed = TICK_SPEED;
}

void check_for_full_rows() {
    for (uint8_t row = current_tet.y_pos; row < (current_tet.y_pos + current_tet.type->height); row++) {
        while (is_full_row(&screen, row)) {
            for (uint8_t y = row; y > 0; y--) {
                for (uint8_t x = 0; x < screen.width * 4; x++) {
                    if (screen_is_set(&screen, x, y - 1)) {
                        screen_set(&screen, x, y);
                    } else {
                        screen_clear(&screen, x, y);
                    }
                }
            }

            // increase score (& speed) for every cleared line
            score++;
            /*if (speed > 10) {
                speed -= 2;
            }*/
        }
    }
}

void run_game_tick() {
    RUN_TIMED(speed, time);

    // try to fall down, if successful, return
    if (move(current_tet.x_pos, current_tet.y_pos + 1)) return;

    // unsuccessful dot drop in row 1 == loss
    if (current_tet.y_pos == 0) {
        game_over();
    } else {
        check_for_full_rows();
    }

    tet_init_tetromino(&current_tet, &screen);
    update_screen();
}

void rotate_test_proc(uint8_t screen_x, uint8_t screen_y, void *free) {
    if (screen_is_set(&screen, screen_x, screen_y)) {
        *(uint8_t*) free = 0;
    }
}

uint8_t can_rotate() {
    uint8_t initial_rotation = current_tet.rotation;
    uint8_t free = 1;
    screen_enable_dry_mode(&screen);

    tet_clear_tetromino(&screen, &current_tet);
    tetromino_rotate(&current_tet);
    tetromino_enum_pixels(&current_tet, &rotate_test_proc, &free);

    screen_disable_dry_mode(&screen);
    current_tet.rotation = initial_rotation;
    return free;
}

void process_events() {
    while (EVENTS_TO_POLL) {
        switch (poll_event()) {
            case BTN_RIGHT: // move to the right
                move(current_tet.x_pos + 1, current_tet.y_pos);
                break;
            case BTN_LEFT: // move to the left
                move(current_tet.x_pos - 1, current_tet.y_pos);
                break;
            case BTN_UP: // rotate current tetromino
                if (can_rotate()) {
                    tet_clear_tetromino(&screen, &current_tet);
                    tetromino_rotate(&current_tet);
                    update_screen();
                }
                break;
            case BTN_DOWN: // drop as far as possible instantly
                while (move(current_tet.x_pos, current_tet.y_pos + 1));
                break;
        }
    }
}

int main(void) {
    // button pins
    pin_set_mode(&DDRB, PB0, PIN_INPUT); // right
    pin_set_mode(&DDRA, PA1, PIN_INPUT); // up
    pin_set_mode(&DDRB, PB2, PIN_INPUT); // left
    pin_set_mode(&DDRB, PB1, PIN_INPUT); // down

    screen_init(&screen);

    // timer F_CPU / 1024
    TCCR0 = (1 << CS02) | (1 << CS00);
    // enable timer overflow interrupt
    TIMSK |= (1 << TOIE0);
    sei();

    // setup adc for prng seed
    pin_set_mode(&DDRA, PA0, PIN_INPUT);
    adc_enable();
    adc_prescale(ADC_PRESC_128);
    srand_adc();

    tet_init_tetromino(&current_tet, &screen);

    while (1) {
        run_game_tick();
        process_events();
    }

    return 0;
}
