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

void sensor_rx(spacecan_bus_node_t *node, spacecan_frame_t *frame) {
  (void)node;
  (void)frame;
}

void sensor_tx(sensor_node_t *sensor) {
  spacecan_frame_t frame;

  uint8_t payload[2];
  payload[0] = 0x10; // Telemetry ID
  payload[1] = sensor->counter++;

  if (sensor->counter > 10){
    sensor->counter = 0;
  }

  sc_build_reply(&frame, sensor->base.node_id, payload, 2);
  sc_bus_transmit(sensor->bus, &sensor->base, &frame);
  sc_bus_send(fd_server, &frame);
}

void motor_tx(motor_node_t *motor) {
  spacecan_frame_t frame;

  uint8_t payload[2];
  payload[0] = 0x10; // Telemetry ID

  if (motor->speed < 20){
    payload[1] = motor->speed++;
  }

  sc_build_reply(&frame, motor->base.node_id, payload, 2);
  sc_bus_transmit(motor->bus, &motor->base, &frame);
  sc_bus_send(fd_server, &frame);
}

void motor_rx(spacecan_bus_node_t *node, spacecan_frame_t *frame) {
  motor_node_t *motor = (motor_node_t*)node;
  if ((frame->can_id & CAN_FUNCTION_MASK) == CANID_REQ) {
    if (frame->dlc >= 2) {
      uint8_t cmd = frame->buffer[0];

      if (cmd == 0x01) {
        motor->speed = frame->buffer[1];
        printf("[Motor] Set speed to %d\n", motor->speed);
      }
    }
  }
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
  
  sensor_node_t sensor;
  motor_node_t motor;

  spacecan_bus_node_t sensor_node;
  spacecan_bus_node_t motor_node;
  spacecan_bus_node_t logger_node;

  sensor_node.node_id = 0x01;
  sensor_node.rx_callback = sensor_rx;
  sensor_node.is_controller = (sensor_node.node_id == 0);
  sensor_node.last_heartbeat_ms = 0;
  sensor_node.heartbeat_interval_ms = SC_HEARTBEAT_INTERVAL;
  sensor_node.state = SC_STATE_INIT;

  motor_node.node_id = 0x02;
  motor_node.rx_callback = motor_rx;
  motor_node.is_controller = (motor_node.node_id == 0);
  motor_node.last_heartbeat_ms = 0;
  motor_node.heartbeat_interval_ms = SC_HEARTBEAT_INTERVAL;
  motor_node.state = SC_STATE_INIT;

  logger_node.node_id = 0x00;
  logger_node.rx_callback = can_logger;

  sc_bus_add_node(&bus, &logger_node);
  sc_bus_add_node(&bus, &sensor_node);
  sc_bus_add_node(&bus, &motor_node);

  sensor.base = sensor_node;
  sensor.bus = &bus;
  sensor.counter = 0;

  motor.base = motor_node;
  motor.bus = &bus;
  motor.speed = 0;

  while (1){
    sc_bus_poll();
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    const uint32_t ts_now = ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    
    sc_bus_node_send_heartbeat(fd_server, &sensor_node, ts_now);
    sc_bus_node_send_heartbeat(fd_server, &motor_node, ts_now);
    
    sc_bus_node_send_sync(fd_server, &sensor_node, ts_now);
    sensor_tx(&sensor);
    
    sc_bus_node_send_sync(fd_server, &motor_node, ts_now);
    motor_tx(&motor);
    
    sleep(1);
  }

  return 0;
}
