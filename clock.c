#include <ncurses.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

// seven-segment display
//   |a|b|c|d|e|f|g|
// 0 |1|1|1|1|1|1|0|
// 1 |0|1|1|0|0|0|0|
// 2 |1|1|0|1|1|0|1|
// 3 |1|1|1|1|0|0|1|
// 4 |0|1|1|0|0|1|1|
// 5 |1|0|1|1|0|1|1|
// 6 |1|0|1|1|1|1|1|
// 7 |1|1|1|0|0|0|0|
// 8 |1|1|1|1|1|1|1|
// 9 |1|1|1|1|0|1|1|
int LED_DIGITS[10] = {
    0b1111110,
    0b0110000,
    0b1101101,
    0b1111001,
    0b0110011,
    0b1011011,
    0b1011111,
    0b1110000,
    0b1111111,
    0b1111011
};

void print_led_digit(int dig, int y, int x) {
    if ((dig >> 6) & 1) // a
        mvprintw(y, x + 1, "__");
     if ((dig >> 5) & 1) // b
        mvprintw(y + 1, x + 3, "|");
     if ((dig >> 4) & 1) // c
        mvprintw(y + 2, x + 3, "|");
     if ((dig >> 3) & 1) // d
        mvprintw(y + 2, x + 1, "__");
     if ((dig >> 2) & 1) // e
        mvprintw(y + 2, x, "|");
     if ((dig >> 1) & 1) // f
        mvprintw(y + 1, x, "|");
     if ((dig >> 0) & 1) // g
        mvprintw(y + 1, x + 1, "__");
}


void *routine(void *arg) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    uint64_t tenths = tv.tv_sec * 10 + (tv.tv_usec / 100000);

    int dig1, dig2;
    while (1) {
        int row, col;
        getmaxyx(stdscr, row, col);

        mvprintw(row - 1, 0, "(q)uit, (t)imer");

        int row_off = (row / 2) - 1;
        int col_off = (col / 2) - (33 / 2);

        int hr = (tenths / 10 / 60 / 60) % 24;
        dig1 = (hr / 10) % 10;
        dig2 = hr % 10;
        print_led_digit(LED_DIGITS[dig1], row_off, col_off);
        print_led_digit(LED_DIGITS[dig2], row_off, col_off + 4);
        mvaddch(row_off + 2, col_off + 8, ':');

        int min = (tenths / 10 / 60) % 60;
        dig1 = (min / 10) % 10;
        dig2 = min % 10;
        print_led_digit(LED_DIGITS[dig1], row_off, col_off + 9);
        print_led_digit(LED_DIGITS[dig2], row_off, col_off + 13);
        mvaddch(row_off + 2, col_off + 17, ':');

        int sec = (tenths / 10) % 60;
        dig1 = (sec / 10) % 10;
        dig2 = sec % 10;
        print_led_digit(LED_DIGITS[dig1], row_off, col_off + 18);
        print_led_digit(LED_DIGITS[dig2], row_off, col_off + 22);
        mvaddch(row_off + 2, col_off + 26, '.');

        int tenth = tenths % 10;
        print_led_digit(LED_DIGITS[tenth], row_off, col_off + 27);
        refresh();
        usleep(100000);
        clear();
        tenths++;
    }
    return NULL;
}

void handle_winch(int sig) {
    endwin();
    refresh();
    clear();
}

int main() {
    initscr();
    keypad(stdscr, TRUE);
    curs_set(0);

    // struct sigaction sa;
    // memset(&sa, 0, sizeof(struct sigaction));
    // sa.sa_handler = handle_winch;
    // sigaction(SIGWINCH, &sa, NULL);

    signal(SIGWINCH, handle_winch);

    if (has_colors() == TRUE) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_GREEN, -1);
        attron(COLOR_PAIR(1));
    }

    refresh();
    pthread_t thread;
    pthread_create(&thread, NULL, routine, NULL);

    getch();
    pthread_cancel(thread);
    endwin();

    return 0;
}
