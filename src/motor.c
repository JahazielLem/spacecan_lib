/* src - motor.c
 *
 * spacecan_lib - By astrobyte 20/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include <stdlib.h>
#include <unistd.h>
#include <spacecan.h>

static spacecan_bus_node_t motor_node;
static int motor_fd;
static uint32_t last_telemetry = 0;
static uint8_t  value = 0;

static void rx_callback(spacecan_bus_node_t *self, spacecan_frame_t *f) {
  if (sc_frame_get_id_rep(f->can_id) != self->node_id) { return; }
  if ((f->can_id & CAN_FUNCTION_MASK) == CANID_REQ) {
    if (f->dlc >= 2) {
      const uint8_t cmd = f->buffer[0];

      if (cmd == 0x01) {
        value = f->buffer[1];
        printf("[Motor Node %02x] Set speed to %d\n", self->node_id, value);
      }
    }
  }
}

int sc_motor_begin(void) {
  motor_fd = sc_bus_connect();
  if (motor_fd < 0){ return -1;}
  motor_node.node_id = 0x04;
  motor_node.state   = SC_STATE_OPERATIONAL;
  motor_node.heartbeat_interval_ms = 1000;
  motor_node.sync_interval_ms      = 200;
  motor_node.last_heartbeat_ms     = 0;
  motor_node.last_sync_ms          = 0;
  motor_node.rx_callback           = rx_callback;
  return 0;
}

void sc_motor_close(void) {
  close(motor_fd);
}

void sc_motor_worker(void){
  const uint32_t now = sc_get_monotonic_ms();
  // Heartbeat & sync
  if (now - motor_node.last_heartbeat_ms >= motor_node.heartbeat_interval_ms) {
    spacecan_frame_t hb;
    sc_build_heartbeat(&hb, motor_node.node_id, motor_node.state);
    sc_bus_send(motor_fd, &hb);
    motor_node.last_heartbeat_ms = now;
  }
  if (now - motor_node.last_sync_ms >= motor_node.sync_interval_ms) {
    spacecan_frame_t s;
    sc_build_sync(&s, motor_node.node_id);
    sc_bus_send(motor_fd, &s);
    motor_node.last_sync_ms = now;
  }

  // Fake telemetry every ~800 ms
  if (now - last_telemetry >= 800) {
    uint8_t payload[2];
    payload[0] = 0x10; // Telemetry ID
    payload[1] = value++;
    if (value > 20){
      value = 0;
    }
    spacecan_frame_t f;
    sc_build_reply(&f, motor_node.node_id, payload, 2);
    sc_bus_send(motor_fd, &f);
    last_telemetry = now;
  }

  // Read incoming (non-blocking)
  bus_packet_t pkt;
  if (sc_bus_receive(motor_fd, &pkt) == 0) {
    if (motor_node.rx_callback) {
      motor_node.rx_callback(&motor_node, &pkt.frame);
    }
  }

}
