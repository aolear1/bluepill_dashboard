/*
 * bmsParams.c
 *
 *  Created on: Nov 2, 2025
 *      Author: aiden
 */
#include "main.h"
#include "bmsParams.h"
#include "utils.h"

BMS_Params_t bmsParams = {
    .cell_overvoltage = configBMS_CELL_OVERVOLTAGE,
    .cell_undervoltage = configBMS_CELL_UNDERVOLTAGE,
    .debug_mode = configBMS_DEBUG_MODE,
	.header = configBMS_HEADER
};

void updateBMSChecksum()
{
	uint8_t *struct_val = (uint8_t*)&bmsParams;
	struct_val++;
	uint8_t checksum;

	calculateChecksum(struct_val, sizeof(bmsParams)-configBMS_CHECKSUM_PASSES, &checksum);

	bmsParams.checksum = checksum;
}


