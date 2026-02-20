/* src - node.c
 *
 * spacecan_lib - By astrobyte 17/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include <spacecan.h>

void sc_node_init(spacecan_node_t *node, uint8_t node_id) {
  node->node_id = node_id;
  node->is_controller = (node_id == 0);
  node->last_heartbeat_ms = 0;
  node->heartbeat_interval_ms = SC_HEARTBEAT_INTERVAL;
  node->state = SC_STATE_INIT;
}

