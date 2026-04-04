/* src - motor.h
 *
 * spacecan_lib - By astrobyte 20/02/26.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
*/
#ifndef SPACECAN_LIB_MOTOR_H
#define SPACECAN_LIB_MOTOR_H

int sc_motor_begin(void);
void sc_motor_close(void);
void sc_motor_worker(void);

#endif //SPACECAN_LIB_MOTOR_H
