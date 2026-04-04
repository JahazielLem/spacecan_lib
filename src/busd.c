/* src - busd.c
 *
 * spacecan_lib - By astrobyte 20/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#include <spacecan.h>
#include <stdlib.h>
#include <signal.h>

static volatile sig_atomic_t running = 1;

static void sigint_handler(int sig) {
  (void)sig;
  running = 0;
}

int sc_busd_init(void) {
  signal(SIGINT, sigint_handler);
  signal(SIGPIPE, SIG_IGN);

  spacecan_bus_t dummy_bus = {0};

  sc_bus_init(&dummy_bus);

  if (sc_bus_create_server_socket() < 0) { return 1;}

  return 0;
}
