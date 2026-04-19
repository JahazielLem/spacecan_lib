/* src - sensor.c
 *
 * spacecan_lib - By astrobyte 20/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include <stdlib.h>
#include <unistd.h>
#include <spacecan.h>

static spacecan_bus_node_t sensor_node;
static int sensor_fd;
static uint32_t last_telemetry = 0;

static void rx_callback(spacecan_bus_node_t *self, spacecan_frame_t *f) {
  printf("[Node %02x] ← ID=0x%03x DLC=%d  ", self->node_id, f->can_id, f->dlc);
  for (int i = 0; i < f->dlc; i++) printf("%02x ", f->buffer[i]);
  printf("\n");
}

int sc_sensor_begin(void) {
  sensor_fd = sc_bus_connect();
  if (sensor_fd < 0) {
    return -1;
  }

  sensor_node.node_id = 0x01;
  sensor_node.state   = SC_STATE_OPERATIONAL;
  sensor_node.heartbeat_interval_ms = 1500;
  sensor_node.sync_interval_ms      = 200;
  sensor_node.last_heartbeat_ms     = 0;
  sensor_node.last_sync_ms          = 0;
  sensor_node.rx_callback           = rx_callback;
  return 0;
}

void sc_sensor_close(void) {
  close(sensor_fd);
}

void sc_sensor_worker(void){
  const uint32_t now = sc_get_monotonic_ms();

  // Heartbeat & sync
  if (now - sensor_node.last_heartbeat_ms >= sensor_node.heartbeat_interval_ms) {
    spacecan_frame_t hb;
    sc_build_heartbeat(&hb, sensor_node.node_id, sensor_node.state);
    sc_bus_send(sensor_fd, &hb);
    sensor_node.last_heartbeat_ms = now;
  }
  if (now - sensor_node.last_sync_ms >= sensor_node.sync_interval_ms) {
    spacecan_frame_t s;
    sc_build_sync(&s, sensor_node.node_id);
    sc_bus_send(sensor_fd, &s);
    sensor_node.last_sync_ms = now;
  }

  // Fake telemetry every ~800 ms
  if (now - last_telemetry >= 800) {
    uint8_t payload[4];
    payload[0] = 0x10; // Telemetry ID
    payload[1] = rand() % 100;
    payload[2] = rand() % 100;
    payload[3] = rand() % 100;

    spacecan_frame_t f;
    sc_build_reply(&f, sensor_node.node_id, payload, sizeof(payload));
    sc_bus_send(sensor_fd, &f);
    last_telemetry = now;
  }
}
