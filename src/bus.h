/* src - bus.h
 *
 * spacecan_lib - By astrobyte 17/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef SPACECAN_LIB_BUS_H
#define SPACECAN_LIB_BUS_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "frame.h"

#define BUS_MAX_NODES 16
#define BUS_MAGIC 0x5343414E

#define SC_STATE_INIT 0x00
#define SC_STATE_OPERATIONAL 0x01
#define SC_STATE_ERROR 0x09

#define SC_HEARTBEAT_INTERVAL (1000)

typedef struct spacecan_bus_node spacecan_bus_node_t;

typedef void (*sc_rx_callback_t)(spacecan_bus_node_t *node, spacecan_frame_t *frame);

typedef struct {
  uint32_t magic;
  uint64_t timestamp_ns;
  spacecan_frame_t frame;
} bus_packet_t;

typedef struct {
  int client_fd;
} spacecan_bus_client_t;

typedef struct {
  size_t node_count;
  spacecan_bus_node_t *nodes[BUS_MAX_NODES];
} spacecan_bus_t;

typedef struct {
  int server_fd;
  spacecan_bus_t *internal_bus;
  spacecan_bus_client_t clients[32];
  size_t client_count;
} spacecan_busd_t;

struct spacecan_bus_node{
  uint8_t node_id;
  uint8_t state;
  uint8_t is_controller;
  uint8_t is_time_master;

  uint32_t heartbeat_interval_ms;
  uint32_t last_heartbeat_ms;

  uint32_t sync_interval_ms;
  uint32_t last_sync_ms;

  sc_rx_callback_t rx_callback;
  void *user_data;
};

int sc_bus_create_server_socket(void);
void sc_bus_accept_clients(void);
int sc_bus_connect(void);
void sc_bus_poll(void);
void sc_bus_broadcast(int sender_fd, const bus_packet_t *pkt);
void sc_bus_init(spacecan_bus_t *bus);
int sc_bus_add_node(spacecan_bus_t *bus, spacecan_bus_node_t *node);
void sc_bus_transmit(const spacecan_bus_t *bus, const spacecan_bus_node_t *sender, spacecan_frame_t *frame);
ssize_t sc_bus_send(int fd, const spacecan_frame_t *frame);
int sc_bus_receive(int fd, bus_packet_t *pkt);

void sc_bus_node_send_heartbeat(int fd, spacecan_bus_node_t *node);
void sc_bus_node_send_sync(int fd, spacecan_bus_node_t *node);

uint32_t sc_get_monotonic_ms(void);
#endif //SPACECAN_LIB_BUS_H
