/*
 * mutex_utils.c
 *
 *  Created on: 18/11/2016
 *      Author: csic
 */

#include "mutex_utils.h"
XMutex Mutex_instance;	/* Mutex instance */

int mutex_utils_init(void)
{
	u16 MutexDeviceID = MUTEX_DEVICE_ID;
	XMutex_Config *ConfigPtr;
	XStatus Status;

	/*
	 * Lookup configuration data in the device configuration table.
	 * Use this configuration info down below when initializing this
	 * driver instance.
	 */
	ConfigPtr = XMutex_LookupConfig(MutexDeviceID);
	if (ConfigPtr == (XMutex_Config *)NULL) {
		return XST_FAILURE;
	}

	/*
	 * Perform the rest of the initialization.
	 */
	Status = XMutex_CfgInitialize(&Mutex_instance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
// This function performs a blocking lock but making some wait cycles, with the objetive of not saturate AXI-lite bus
int mutex_utils_blocking_LOCK(u8 mutex_ID)
{

	while(XMutex_Trylock(&Mutex_instance, mutex_ID)) usleep(USEC_SLEEP_FOR_MUTEX);

	return 0;
}

int mutex_utils_UNLOCK(u8 mutex_ID)
{
	return XMutex_Unlock(&Mutex_instance, mutex_ID);
}

