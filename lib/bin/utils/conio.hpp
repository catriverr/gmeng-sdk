#ifndef MYCONIO_MAC_CPP_INCLUDED
#define MYCONIO_MAC_CPP_INCLUDED

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <sys/termios.h>
#include <termios.h>
#include <unistd.h>

enum {
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    LIGHTGRAY,
    DARKGRAY,
    LIGHTRED,
    LIGHTGREEN,
    YELLOW,
    LIGHTBLUE,
    LIGHTMAGENTA,
    LIGHTCYAN,
    WHITE,
};  // text background

#define CLEARELN printf("\x1B[2K")  // Clear entire line
#define WHITEBOLD 9
#define ESC 033  // ESCAPE character

static struct termios new_settings;
static int peek_character = -1;

struct TextInfo {
    int curx;
    int cury;
} text{0, 0};

// claim functions

void mode_raw(int);
void _fflush();
void underline(int);
char read_key();
int key_pressed();
void get_pass(char *, char *);
int getche();
int getch();
int where_x();
int where_y();
int keyboard_hit();
void line(int, char, char);
void goto_xy(int, int);
void sleep_for(int);
void bold(int);
void text_color(int);
void test_background(int);
void clear_eol();
void clear_screen();
void clear_win(int, int, int, int);

// definitions

// what is this use for?
void mode_raw(int activator) {
    static struct termios cooked;
    static int raw_actif{};
    if (raw_actif == activator) {
        return;
    }
    if (activator != 0) {
        struct termios raw;
        tcgetattr(STDIN_FILENO, &cooked);
        raw = cooked;
        cfmakeraw(&raw);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
    }
    raw_actif = activator;
}

// flush
void _fflush() {
    int stdin_var;
    while ((stdin_var = getchar()) != '\n' and stdin_var != EOF) {
    }
}

// underline
void underline(int activator) {
    (activator) ? printf("\e[4m") : printf("\e[0m");
}

// read key
char read_key() {
    char key;
    mode_raw(1);
    key = getchar();
    mode_raw(0);
    return key;
}

// key press
int key_pressed() {
    struct timeval timev = {0, 0};
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timev);
}

// get pass
void get_pass(char *prompt, char *pass) {
    int cpt{};
    printf("%s", prompt);
    while ((pass[cpt] = read_key()) != 13 and cpt != 100) {
        putchar('*');
        cpt++;
    }
    printf("\n");
    pass[cpt] = 0;
}

// what is this for?
int getche() {
    struct termios t;
    int c{};
    tcgetattr(0, &t);
    t.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &t);
    fflush(stdout);
    c = getchar();
    t.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &t);
    return c;
}

// get character
int getch() {
    struct termios old_t, new_t;
    int ch;
    tcgetattr(STDIN_FILENO, &old_t);
    new_t = old_t;
    new_t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
    return ch;
}

// show x
int where_x() {
    return text.curx;
}

// show y
int where_y() {
    return text.cury;
}

// keyboard hit
int keyboard_hit() {
    unsigned char ch;
    int nread;
    if (peek_character != -1) {
        return 1;
    }
    new_settings.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0, &ch, 1);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);
    if (nread == 1) {
        peek_character = ch;
        return 1;
    }
    return 0;
}

// line
void line(int x, char c1, char c2) {
    putchar(c1);
    for (int cpt{1}; cpt <= x - 2; ++cpt) {
        putchar(c2);
    }
    putchar(c1);
    putchar('\n');
}

// move cursor to (x, y)
void goto_xy(int x, int y) {
    if (x == 1 and y == 1) {
        printf("%c[%d;%dH", ESC, 0, 0);
        text.curx = 0;
        text.cury = 0;
    } else {
        printf("%c[%d;%dH", ESC, y, x);
        text.curx = x;
        text.cury = y;
    }
}

// sleep
void sleep_for(int pause) {
    usleep(pause * 1000);
    fflush(stdout);
}

// set bold font
void bold(int activator) {
    (activator) ? printf("\e[1m") : printf("\e[0m");
}

// set text color
void text_color(int foreground_color) {
    int x{foreground_color};
    if (foreground_color < 0 or foreground_color > 7) {
        x = 0;
    }
    printf("\e[3%dm", x);
}

// set text background color
void test_background(int background_color) {
    printf("%c[%dm", ESC, 40 + background_color);
}

// clear characters to end of line
void clear_eol() {
    CLEARELN;
}

// clear screen and go back to (0, 0)
void clear_screen() {
    printf("%c[2J", ESC);
    goto_xy(0, 0);
}

// clear window?
void clear_win(int x, int y, int xx, int yy) {
    for (int i{1}; i <= (yy / 2) - 2; ++i) {
        goto_xy(x, y);
        y++;
        line(xx, ' ', ' ');
    }
}

#endif 