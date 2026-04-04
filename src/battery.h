/* src - battery.h
 *
 * spacecan_lib - By astrobyte 20/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef SPACECAN_LIB_BATTERY_H
#define SPACECAN_LIB_BATTERY_H

int sc_battery_begin(void);
void sc_battery_close(void);
void sc_battery_worker(void);


#endif //SPACECAN_LIB_BATTERY_H
