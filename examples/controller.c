/* examples - controller.c
 *
 * spacecan_lib - By astrobyte 17/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/

#include <spacecan.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>

static int fd_server = 0;

typedef struct {
  spacecan_bus_node_t base;
  spacecan_bus_t *bus;
  uint8_t counter;
} sensor_node_t;
typedef struct {
  spacecan_bus_node_t base;
  spacecan_bus_t *bus;
  uint8_t speed;
} motor_node_t;

static void print_frame(spacecan_frame_t *f){
  printf("ID=0x%03X DLC=%d DATA=", f->can_id, f->dlc);
  for (int i = 0; i < f->dlc; i++)
    printf("%02X ", f->buffer[i]);
  printf("\n");
}

void can_logger(spacecan_bus_node_t *node, spacecan_frame_t *frame) {
  (void)node;
  print_frame(frame);
}

int main(void) {

  signal(SIGPIPE, SIG_IGN);

  spacecan_bus_t bus;
  sc_bus_init(&bus);

  if (sc_bus_create_server_socket() < 0) {
    printf("Failed to create bus server\n");
    return 1;
  }

  fd_server = sc_bus_connect();
  if (fd_server < 0) {
    printf("Failed to connect controller to bus\n");
    return 1;
  }

  int ret = sc_sensor_begin();
  if (ret < 0 ) {
    printf("Failed to start sensor node\n");
  }
  ret = sc_motor_begin();
  if (ret < 0 ) {
    printf("Failed to start motor node\n");
  }
  ret = sc_battery_begin();
  if (ret < 0 ) {
    printf("Failed to battery motor node\n");
  }
  spacecan_bus_node_t logger_node;

  logger_node.node_id = 0x00;
  logger_node.rx_callback = can_logger;

  sc_bus_add_node(&bus, &logger_node);
  while (1){
    sc_bus_poll();
    sc_motor_worker();
    sc_sensor_worker();
    sc_battery_worker();
    usleep(1000);
  }

  return 0;
}
