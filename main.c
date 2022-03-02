#include <libavr.h>
#include <libshift.h>
#include <led_matrix.h>
#include <led_screen.h>
#include <random.h>
#include <event.h>

#include "tetris.h"
#include "matrix_pins.h"

#define arr(s, t) s, sizeof(s) / sizeof(t)

// button event types + state flags
#define BTN_DOWN  0
#define BTN_LEFT  1
#define BTN_UP    2
#define BTN_RIGHT 3

#define TICK_SPEED 30

ledm_t* matrices[] = {&ol, &or, &ml, &mr, &ul, &ur};

screen_t screen = {
    .matrices = matrices, 
    .height = 3,
    .width = 2,
};

// tetromino_t current_tet;

// keep track of current dot position
uint8_t x_pos = 4;
uint8_t y_pos = 0;

uint32_t score = 0;
uint8_t speed = TICK_SPEED;

ledm_letter_t gameover[] = {LEDM_G, LEDM_A, LEDM_M, LEDM_E, LEDM_SPACE, LEDM_O, LEDM_V, LEDM_E, LEDM_R, LEDM_EXCLAMATION, LEDM_SPACE}; // GAME OVER! 

// register for pressed buttons to detect presses / releases
uint8_t button_states = 0;
// times, the timer ISR was executed
volatile uint32_t time = 0;

#define push_event_if_changed(btn) if ((new_states & (1 << btn)) && !(button_states & (1 << btn))) push_event(btn)

ISR( TIMER0_OVF_vect ) {
    time++;

    uint8_t new_states = (pin_read_bit(PINB, PB0) << BTN_RIGHT) | \
                         (pin_read_bit(PINA, PA1) << BTN_UP)    | \
                         (pin_read_bit(PINB, PB2) << BTN_LEFT)  | \
                          pin_read_bit(PINB, PB1) << BTN_DOWN;

    if (new_states != button_states) {
        push_event_if_changed(BTN_RIGHT);
        push_event_if_changed(BTN_UP);
        push_event_if_changed(BTN_LEFT);
        push_event_if_changed(BTN_DOWN);

        button_states = new_states;
    }
}

// random generator
rand_state_t rand_state;
#define random_column() (rand_xorshift32(&rand_state) & 0b111000) >> 3 // mask by 0b111 to get numbers in range 0..7, is not very reliable

void update_screen() {
    screen_set(&screen, x_pos, y_pos);
}

/**
 * @brief Tries to move to the given position and updates the screen
 * 
 * @param x the x position to move to
 * @param y the y position to move to
 * @return whether the move was successful (1) or not successful because of 
 *         obstruction (0)
 */
uint8_t move(uint8_t x, uint8_t y) {
    if (screen_is_set(&screen, x, y)) return 0;

    screen_clear(&screen, x_pos, y_pos);
    x_pos = x;
    y_pos = y;
    update_screen();

    return 1;
}

uint8_t is_full_row(screen_t *s, uint8_t y) {
    for (uint8_t x = 0; x < s->width * 4; x++) {
        if (!screen_is_set(s, x, y)) return 0;
    }
    return 1;
}

void run_game_tick() {
    RUN_TIMED(speed, time);

    // try to fall down, if successful, return
    if (move(x_pos, y_pos + 1)) return;

    // unsuccessful dot drop in row 1 == loss
    if (y_pos == 1) {
        screen_clear_all(&screen);
        ledm_show_word_rotating(matrices, 2, ledm_word(gameover), 200, LEDM_LEFT, 0);
        screen_clear_all(&screen);
    }

    // check if line was filled
    if (is_full_row(&screen, y_pos)) {
        for (uint8_t y = y_pos; y > 0; y--) {
            for (uint8_t x = 0; x < screen.width * 4; x++) {
                if (screen_is_set(&screen, x, y - 1)) {
                    screen_set(&screen, x, y);
                } else {
                    screen_clear(&screen, x, y);
                }
            }
        }
        score++;
        if (speed > 10) {
            speed -= 2;
        }
    }

    // create new dot at the top in a random column
    x_pos = random_column();
    y_pos = 0;

    update_screen();
}

void process_events() {
    while (EVENTS_TO_POLL) {
        switch (poll_event()) {
            case BTN_RIGHT: // move to the right
                move(x_pos + 1, y_pos);
                break;
            case BTN_LEFT: // move to the left
                move(x_pos - 1, y_pos);
                break;
            case BTN_UP:
                break;
            case BTN_DOWN: // drop as far as possible instantly
                while (move(x_pos, y_pos + 1));
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
    rand_seed_adc(&rand_state, 0);

    // initialise and show first dot
    x_pos = random_column();
    update_screen();

    // tet_init_tetromino(&current_tet, &rand_state);

    while (1) {
        run_game_tick();
        process_events();
    }

    return 0;
}
