/* src - node.h
 *
 * spacecan_lib - By astrobyte 17/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef SPACECAN_LIB_NODE_H
#define SPACECAN_LIB_NODE_H

#include <stdint.h>
#include <string.h>


typedef struct {
  uint8_t node_id;
  uint8_t is_controller;
  uint32_t heartbeat_interval_ms;
  uint32_t last_heartbeat_ms;
  uint8_t state;
} spacecan_node_t;

void sc_node_init(spacecan_node_t *node, uint8_t node_id);
#endif //SPACECAN_LIB_NODE_H
