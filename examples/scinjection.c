/* examples - scinjection.c
 *
 * spacecan_lib - By astrobyte 17/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/

#include <spacecan.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static void print_frame(spacecan_frame_t *f){
  printf("ID=0x%03X DLC=%d DATA=", f->can_id, f->dlc);
  for (int i = 0; i < f->dlc; i++)
    printf("%02X ", f->buffer[i]);
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: %s <CANID> <dlc> [byte1...]\n", argv[0]);
    return 1;
  }
  int fd = sc_bus_connect();

  if (fd < 0){
    printf("ERROR!\n");
    return 1;
  }
  spacecan_frame_t frame;

  frame.can_id = strtol(argv[1], NULL, 16);
  frame.dlc = argc - 2;

  printf("[Injection] Injection to: 0x%02X - DLC: %d\n", frame.can_id, frame.dlc);

  for (int i = 0; i < frame.dlc; i++) {
    frame.buffer[i] = strtol(argv[i+2], NULL, 16);
  }

  print_frame(&frame);
  sc_bus_send(fd, &frame);
  sleep(2);
  close(fd);
  return 0;
}
