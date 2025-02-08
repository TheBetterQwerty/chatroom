#include <ncurses.h>
#include <string.h>

#include "template.h"
#define MAX_MESSAGES 80
#define MAX_INPUT 500

void add_message(const char *message) {
    if (message_count >= MAX_MESSAGES) {
        for (int i = 1; i < MAX_MESSAGES; i++) {
            strcpy(messages[i - 1], messages[i]);
        }
        message_count--;
    }
    strcpy(messages[message_count], message);
    message_count++;
}

int client(const char* username) {
    char input[MAX_INPUT - 30] = { 0 };
    int input_len = 0;
    char buffer[MAX_INPUT] = { 0 };

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    WINDOW *msg_win = newwin(rows - 1, cols, 0, 0);
    WINDOW *input_win = newwin(1, cols, rows - 1, 0);
    add_message("Welcome to the IRC-like client!");

    while (1) {
        wclear(msg_win);
        for (int i = 0; i < message_count; i++) {
            mvwprintw(msg_win, i, 0, "%s", messages[i]);
        }
        wrefresh(msg_win);

        wclear(input_win);
        mvwprintw(input_win, 0, 0, "(%s) %s", username, input);
        wrefresh(input_win);

        int ch = wgetch(input_win); // Get a single character

        switch (ch) {
            case 112:
                return 0;
            case 27:
                input_len = 0;
                memset(input, 0, sizeof(input));
                break;
            case 127:
            case 8:
                if (input_len <= 0) {
                    break;
                }
                input[--input_len] = '\0';
                break;
            case '\n':
                if (input_len > 0) {
                    sprintf(buffer, "(%s) %s", username, input);
                    add_message(buffer);
                }
                input_len = 0;
                memset(input, 0, sizeof(input));
                memset(buffer, 0, sizeof(buffer));
                break;
            default:
                if (input_len < MAX_INPUT - 1) {
                    input[input_len++] = (char)ch;
                }
                break;
        }
    }

    endwin();
    return 0;
}
