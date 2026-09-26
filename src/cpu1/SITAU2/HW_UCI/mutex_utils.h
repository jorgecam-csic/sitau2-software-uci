/*
 * mutex_utils.h
 *
 *  Created on: 18/11/2016
 *      Author: csic
 */

#ifndef MUTEX_UTILS_H_
#define MUTEX_UTILS_H_

#include "xmutex.h"
#include "sleep.h"

#define MUTEX_DEVICE_ID		XPAR_MUTEX_0_IF_0_DEVICE_ID

extern XMutex Mutex_instance;	/* Mutex instance */
#define USEC_SLEEP_FOR_MUTEX	1
int mutex_utils_init(void);
int mutex_utils_blocking_LOCK(u8 mutex_ID);
int mutex_utils_UNLOCK(u8 mutex_ID);

#endif /* MUTEX_UTILS_H_ */
