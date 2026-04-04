/* examples - screplay.c
 *
 * spacecan_lib - By astrobyte 17/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include <spacecan.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>

typedef struct {
    uint64_t timestamp_ns;
    spacecan_frame_t frame;
} replay_record_t;

static void sleep_ns(uint64_t ns){
    struct timespec ts;
    ts.tv_sec  = ns / 1000000000ULL;
    ts.tv_nsec = ns % 1000000000ULL;
    nanosleep(&ts, NULL);
}

static void print_frame(spacecan_frame_t *f){
  printf("Replay ID=0x%03X DLC=%d DATA=", f->can_id, f->dlc);
  for (int i = 0; i < f->dlc; i++){
    printf("%02X ", f->buffer[i]);
  }
  printf("\n");
}

int main(int argc, char *argv[]){
  if (argc < 2) {
    printf("Usage: %s <replay_file>\n", argv[0]);
    return 1;
  }

  FILE *f = fopen(argv[1], "rb");
  if (!f) {
    perror("fopen");
    return 1;
  }

  int fd = sc_bus_connect();
  if (fd < 0) {
    printf("Failed to connect to bus\n");
    fclose(f);
    return 1;
  }

  replay_record_t rec;
  uint64_t first_ts = 0;
  uint64_t prev_ts  = 0;

  printf("[Replay] Starting...\n");

  while (fread(&rec, sizeof(rec), 1, f) == 1)
  {
    if (first_ts == 0) {
      first_ts = rec.timestamp_ns;
      prev_ts  = rec.timestamp_ns;
    }

    uint64_t delta = rec.timestamp_ns - prev_ts;

    if (delta > 0){
      sleep_ns(delta);
    }
    print_frame(&rec.frame);

    sc_bus_send(fd, &rec.frame);

    prev_ts = rec.timestamp_ns;
  }

  printf("[Replay] Finished.\n");

  fclose(f);
  close(fd);
  return 0;
}
