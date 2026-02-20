#include "bus.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>

#define BUS_SOCKET_PATH "/tmp/spacecan.sock"

static spacecan_busd_t server_bus;

int sc_bus_create_server_socket(void) {
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, BUS_SOCKET_PATH);

  fcntl(fd, F_SETFL, O_NONBLOCK);

  unlink(BUS_SOCKET_PATH);

  bind(fd, (struct sockaddr*)&addr, sizeof(addr));
  listen(fd, 5);

  server_bus.client_count = 0;
  server_bus.server_fd = fd;

  return fd;
}

void sc_bus_accept_clients(void) {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(server_bus.server_fd, &readfds);

  struct timeval tv = {0, 100000}; // 100ms

  select(server_bus.server_fd + 1, &readfds, NULL, NULL, &tv);

  if (FD_ISSET(server_bus.server_fd, &readfds)) {
    int client_fd = accept(server_bus.server_fd, NULL, NULL);

    if (client_fd >= 0 && server_bus.client_count < BUS_MAX_NODES) {
      server_bus.clients[server_bus.client_count++].client_fd = client_fd;
      fcntl(client_fd, F_SETFL, O_NONBLOCK);
    }
  }
}

int sc_bus_connect(void) {
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, BUS_SOCKET_PATH);

  // fcntl(fd, F_SETFL, O_NONBLOCK);

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
    return -1;
  }

  return fd;
}

void sc_bus_poll(void){
  fd_set readfds;
  FD_ZERO(&readfds);

  int maxfd = server_bus.server_fd;
  FD_SET(server_bus.server_fd, &readfds);

  for (size_t i = 0; i < server_bus.client_count; i++){
    const int fd = server_bus.clients[i].client_fd;
    FD_SET(fd, &readfds);
    if (fd > maxfd){ maxfd = fd; }
  }

  struct timeval tv = { 0, 100000 };

  int ret = select(maxfd + 1, &readfds, NULL, NULL, &tv);
  if (ret <= 0){ return; }

  if (FD_ISSET(server_bus.server_fd, &readfds)) {
    int client_fd = accept(server_bus.server_fd, NULL, NULL);
    if (client_fd >= 0 && server_bus.client_count < BUS_MAX_NODES) {
      fcntl(client_fd, F_SETFL, O_NONBLOCK);
      server_bus.clients[server_bus.client_count++].client_fd = client_fd;
    }
  }

  for (size_t i = 0; i < server_bus.client_count;) {
    int fd = server_bus.clients[i].client_fd;
    if (FD_ISSET(fd, &readfds)) {
      bus_packet_t pkt;
      ssize_t r = read(fd, &pkt, sizeof(pkt));

      if (r <= 0) {
        close(fd);
        server_bus.clients[i] = server_bus.clients[--server_bus.client_count];
        continue;
      }

      if (pkt.magic == BUS_MAGIC) {
        sc_bus_transmit(server_bus.internal_bus, NULL, &pkt.frame);
        sc_bus_broadcast(fd, &pkt);
      }
    }
    i++;
  }
}

void sc_bus_broadcast(int sender_fd, bus_packet_t *pkt) {
  for (size_t i = 0; i < server_bus.client_count;){
    int fd = server_bus.clients[i].client_fd;

    if (fd != sender_fd) {
      ssize_t w = write(fd, pkt, sizeof(*pkt));

      if (w <= 0){
        close(fd);
        server_bus.clients[i] = server_bus.clients[--server_bus.client_count];
        continue;
      }
    }
    i++;
  }
}

void sc_bus_init(spacecan_bus_t *bus) {
  bus->node_count = 0;
  server_bus.internal_bus = bus;
}

int sc_bus_add_node(spacecan_bus_t *bus, spacecan_bus_node_t *node) {
  if (bus->node_count >= BUS_MAX_NODES) { return -1; }
  bus->nodes[bus->node_count++] = node;
  return 0;
}

void sc_bus_transmit(spacecan_bus_t *bus, spacecan_bus_node_t *sender, spacecan_frame_t *frame){
  for (size_t i = 0; i < bus->node_count; i++){
    spacecan_bus_node_t *node = bus->nodes[i];

    if (node != sender && node->rx_callback){
      node->rx_callback(node, frame);
    }
  }
}

int sc_bus_send(int fd, spacecan_frame_t *frame) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  bus_packet_t pkt;
  pkt.magic = BUS_MAGIC;
  pkt.timestamp_ns = ts.tv_sec * 1000000000ULL + ts.tv_nsec;
  pkt.frame = *frame;

  return write(fd, &pkt, sizeof(pkt));
}

int sc_bus_receive(int fd, bus_packet_t *pkt) {
  ssize_t r = read(fd, pkt, sizeof(*pkt));
  if (r <= 0) { return -1; }

  if (pkt->magic != BUS_MAGIC) { return -2; }
  return 0;
}

void sc_bus_node_send_heartbeat(int fd, spacecan_bus_node_t *node, uint32_t now_ms) {
  if (now_ms - node->last_heartbeat_ms >= node->heartbeat_interval_ms) {
    spacecan_frame_t frame;
    sc_build_heartbeat(&frame, node->node_id, node->state);
    sc_bus_send(fd, &frame);
    node->last_heartbeat_ms = now_ms;
  }
}

void sc_bus_node_send_sync(int fd, spacecan_bus_node_t *node, uint32_t now_ms) {
  if (now_ms - node->last_heartbeat_ms >= node->heartbeat_interval_ms) {
    spacecan_frame_t frame;
    sc_build_sync(&frame, node->node_id);
    sc_bus_send(fd, &frame);
    node->last_heartbeat_ms = now_ms;
  }
}
