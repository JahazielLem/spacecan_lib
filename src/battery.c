/* src - battery.c
 *
 * spacecan_lib - By astrobyte 20/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include <stdlib.h>
#include <unistd.h>
#include <spacecan.h>

static spacecan_bus_node_t battery_node;
static int batter_fd;
static uint32_t last_telemetry = 0;
static uint8_t  value = 80;

static void rx_callback(spacecan_bus_node_t *self, spacecan_frame_t *f) {
  if (sc_frame_get_id_rep(f->can_id) != self->node_id) { return;}
  if ((f->can_id & CAN_FUNCTION_MASK) == CANID_REQ) {
    if (f->dlc >= 2) {
      const uint8_t cmd = f->buffer[0];

      if (cmd == 0x01) {
        value = f->buffer[1];
        printf("[Battery Node %02x] Set to %d\n", self->node_id, value);
      }
    }
  }
}

int sc_battery_begin(void) {
  batter_fd = sc_bus_connect();
  if (batter_fd < 0) {
    return -1;
  }

  battery_node.node_id = 0x07;
  battery_node.state   = SC_STATE_OPERATIONAL;
  battery_node.heartbeat_interval_ms = 5000;
  battery_node.sync_interval_ms      = 500;
  battery_node.last_heartbeat_ms     = 0;
  battery_node.last_sync_ms          = 0;
  battery_node.rx_callback           = rx_callback;
  return 0;
}

void sc_battery_close(void) {
  close(batter_fd);
}

void sc_battery_worker(void){
  const uint32_t now = sc_get_monotonic_ms();

  // Heartbeat & sync
  if (now - battery_node.last_heartbeat_ms >= battery_node.heartbeat_interval_ms) {
    spacecan_frame_t hb;
    sc_build_heartbeat(&hb, battery_node.node_id, battery_node.state);
    sc_bus_send(batter_fd, &hb);
    battery_node.last_heartbeat_ms = now;
  }
  if (now - battery_node.last_sync_ms >= battery_node.sync_interval_ms) {
    spacecan_frame_t s;
    sc_build_sync(&s, battery_node.node_id);
    sc_bus_send(batter_fd, &s);
    battery_node.last_sync_ms = now;
  }

  // Fake telemetry every ~800 ms
  if (now - last_telemetry >= 5000) {
    uint8_t payload[2];
    payload[0] = 0x10; // Telemetry ID
    payload[1] = value;

    spacecan_frame_t f;
    sc_build_reply(&f, battery_node.node_id, payload, 2);
    sc_bus_send(batter_fd, &f);
    last_telemetry = now;
  }

  // Read incoming (non-blocking)
  bus_packet_t pkt;
  if (sc_bus_receive(batter_fd, &pkt) == 0) {
    if (battery_node.rx_callback) {
      battery_node.rx_callback(&battery_node, &pkt.frame);
    }
  }
}
