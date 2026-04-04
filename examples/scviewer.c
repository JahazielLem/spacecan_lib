/* examples - can-viewer.c
 *
 * spacecan_lib - By astrobyte 15/03/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/

#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <spacecan.h>

#define UI_FPS 20
#define FRAME_TIME_NS (1000000000 / UI_FPS)

#define MAX_RECORDS 128

typedef struct {
  uint16_t can_id;
  uint32_t count;
  uint64_t last_ts;
  uint8_t data[8];
  uint8_t len;
} packet_record_t;

typedef struct {
  uint16_t count;
  packet_record_t records[MAX_RECORDS];
} records_list_t;

static records_list_t records_list;

static long now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

static packet_record_t *get_or_create_record(const uint16_t can_id){
  for (int i = 0; i < records_list.count; i++) {
    if (records_list.records[i].can_id == can_id) {
      return &records_list.records[i];
    }
  }

  if (records_list.count >= MAX_RECORDS) {
    return NULL;
  }

  packet_record_t *rec = &records_list.records[records_list.count++];

  memset(rec, 0, sizeof(packet_record_t));
  rec->can_id = can_id;

  return rec;
}

static void process_packet(const bus_packet_t *pkt){
  packet_record_t *rec = get_or_create_record(pkt->frame.can_id);

  if (!rec) { return; }
  rec->count++;
  rec->last_ts = now_ns();
  rec->len = pkt->frame.dlc;
  memcpy(rec->data, pkt->frame.buffer, pkt->frame.dlc);
}

static void render_table(WINDOW *win){
  werase(win);
  box(win, 0, 0);

  mvwprintw(win, 0, 2, " CAN PACKETS ");

  wattron(win, A_BOLD);
  mvwprintw(win, 1, 2,  "ID");
  mvwprintw(win, 1, 10, "COUNT");
  mvwprintw(win, 1, 20, "TIMESTAMP");
  mvwprintw(win, 1, 40, "DATA");
  wattroff(win, A_BOLD);

  for (int i = 0; i < records_list.count; i++) {

    packet_record_t *rec = &records_list.records[i];

    mvwprintw(win, i + 2, 2, "%03X", rec->can_id);

    mvwprintw(win, i + 2, 10, "%u", rec->count);

    mvwprintw(win, i + 2, 20, "%lu", rec->last_ts);

    char buf[32];
    char *p = buf;

    for (int j = 0; j < rec->len; j++)
      p += sprintf(p, "%02X ", rec->data[j]);

    mvwprintw(win, i + 2, 40, "%s", buf);
  }

  wrefresh(win);
}

void render_ui(WINDOW *header, WINDOW *left, WINDOW *bottom) {
  werase(header);
  box(header, 0, 0);
  wattron(header, COLOR_PAIR(1));
  mvwprintw(header, 1, 2, "SPACECAN VIEWER");
  wattroff(header, COLOR_PAIR(1));
  wrefresh(header);

  werase(left);
  box(left, 0, 0);

  wrefresh(left);

  werase(bottom);
  box(bottom, 0, 0);
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
  WINDOW *left = newwin(rows, cols, 3, 0);
  WINDOW *bottom = newwin(3, cols, rows - 3, 0);

  long last_render = 0;
  records_list.count = 0;
  memset(&records_list, 0, sizeof(records_list));

  const int fd = sc_bus_connect();
  if (fd < 0) { return 1; }

  while (1) {
    bus_packet_t pkt;

    if (sc_bus_receive(fd, &pkt) == 0) {
      process_packet(&pkt);
    }

    const long now = now_ns();

    if (now - last_render >= FRAME_TIME_NS) {
      render_ui(header, left, bottom);
      render_table(left);
      last_render = now;
    }

    const int ch = getch();
    if (ch == 'q')
      break;

    usleep(10000);
  }

  endwin();
  return 0;
}
