/* src - sensor.h
 *
 * spacecan_lib - By astrobyte 20/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef SPACECAN_LIB_SENSOR_H
#define SPACECAN_LIB_SENSOR_H

int sc_sensor_begin(void);
void sc_sensor_close(void);
void sc_sensor_worker(void);

#endif //SPACECAN_LIB_SENSOR_H
