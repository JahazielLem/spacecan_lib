#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <spacecan.h>

#define UI_FPS 20
#define FRAME_TIME_NS (1000000000 / UI_FPS)

typedef struct {
  int battery;
  int sensor_x;
  int sensor_y;
  int sensor_z;
  int motor_x;
  int motor_y;
  int motor_z;
} spacecraft_state_t;

static spacecraft_state_t state;

long now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

void draw_bar(WINDOW *win, int y, int x, int width, int value, int max) {
  int filled = (value * width) / max;

  mvwprintw(win, y, x, "[");

  for (int i = 0; i < width; i++) {
    if (i < filled) {
      wattron(win, COLOR_PAIR(1));
      waddch(win, '#');
      wattroff(win, COLOR_PAIR(1));
    } else {
      waddch(win, ' ');
    }
  }

  wprintw(win, "] %3d", value);
}

void render_ui(WINDOW *header, WINDOW *left, WINDOW *right, WINDOW *bottom) {
  werase(header);
  box(header, 0, 0);
  wattron(header, COLOR_PAIR(2));
  mvwprintw(header, 1, 2, "PWNSAT MISSION CONTROL - SPACECRAFT TELEMETRY");
  wattroff(header, COLOR_PAIR(2));
  wrefresh(header);

  werase(left);
  box(left, 0, 0);
  mvwprintw(left, 0, 2, " AOCS ");

  mvwprintw(left, 2, 2, "SENSORS");
  mvwprintw(left, 4, 4, "X: %4d", state.sensor_x);
  mvwprintw(left, 5, 4, "Y: %4d", state.sensor_y);
  mvwprintw(left, 6, 4, "Z: %4d", state.sensor_z);

  mvwprintw(left, 8, 2, "Thrusters");
  mvwprintw(left, 10, 4, "Thruster 0: %4d", state.motor_x);
  draw_bar(left, 11, 4, 40, state.motor_x, 100);
  mvwprintw(left, 13, 4, "Thruster 1: %4d", state.motor_y);
  draw_bar(left, 14, 4, 40, state.motor_y, 100);
  mvwprintw(left, 16, 4, "Thruster 2: %4d", state.motor_z);
  draw_bar(left, 17, 4, 40, state.motor_z, 100);

  wrefresh(left);

  werase(right);
  box(right, 0, 0);
  mvwprintw(right, 0, 2, " EPS ");

  mvwprintw(right, 2, 2, "BATTERY LEVEL");
  draw_bar(right, 4, 2, 30, state.battery, 100);

  mvwprintw(right, 7, 2, "BUS VOLTAGE: 28.0 V");
  mvwprintw(right, 8, 2, "BUS CURRENT:  3.2 A");

  wrefresh(right);

  werase(bottom);
  box(bottom, 0, 0);
  mvwprintw(bottom, 1, 2, "STATUS: NOMINAL");
  mvwprintw(bottom, 1, 40, "Press 'q' to exit");
  wrefresh(bottom);
}

int main(void) {
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  nodelay(stdscr, TRUE);

  start_color();
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_CYAN, COLOR_BLACK);

  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  WINDOW *header = newwin(3, cols, 0, 0);
  WINDOW *left = newwin(rows - 6, cols / 2, 3, 0);
  WINDOW *right = newwin(rows - 6, cols / 2, 3, cols / 2);
  WINDOW *bottom = newwin(3, cols, rows - 3, 0);

  long last_render = 0;

  const int fd = sc_bus_connect();
  if (fd < 0) { return 1; }

  while (1) {
    bus_packet_t pkt;
    if (sc_bus_receive(fd, &pkt) == 0) {
      if (pkt.frame.can_id == 0x301) {
        state.sensor_x = pkt.frame.buffer[1];
        state.sensor_y = pkt.frame.buffer[2];
        state.sensor_z = pkt.frame.buffer[3];
      } else if (pkt.frame.can_id == 0x304) {
        state.motor_x = pkt.frame.buffer[1];
        state.motor_y = pkt.frame.buffer[1];
        state.motor_z = pkt.frame.buffer[1];
      } else if (pkt.frame.can_id == 0x307) {
        state.battery = pkt.frame.buffer[1];
      }
    }

    long now = now_ns();
    if (now - last_render >= FRAME_TIME_NS) {
      render_ui(header, left, right, bottom);
      last_render = now;
    }

    int ch = getch();
    if (ch == 'q')
      break;

    usleep(10000);
  }

  endwin();
  return 0;
}
