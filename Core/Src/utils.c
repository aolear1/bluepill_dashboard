/*
 * utils.c
 *
 *  Created on: Nov 15, 2025
 *      Author: aiden
 */

#include "utils.h"


enum utilStatus calculateChecksum(uint8_t *start, uint32_t size, uint8_t *checksum) {

	if (start == 0 || checksum == 0) return UTIL_NULL_ERROR;

	if (size <= 0) return UTIL_INVALID_INPUT;

	uint8_t *current = start;

	uint8_t checksum_tmp = 0;
	for (uint8_t i = 0; i < size; i++) {
		checksum_tmp ^= *current;
		current++;
	}

	*checksum = checksum_tmp;
	return UTIL_SUCCESS;
}

enum utilStatus processPacket(BMS_Params_t *src, BMS_Params_t *dest, uint32_t package_size) {
	//First check if the src matches its checksum
	if (src->header != configBMS_HEADER) return UTIL_FAILED;
	uint8_t *start = (uint8_t *)src + 1;
	uint8_t actual_checksum;
	enum utilStatus status = calculateChecksum(start, package_size, &actual_checksum);
	if (status != UTIL_SUCCESS) return status;

	if (actual_checksum != src->checksum) return UTIL_FAILED;

	uint8_t *src_byte = (uint8_t *)src;
	uint8_t *dest_byte = (uint8_t *)dest;
	for (int i = 0; i < sizeof(BMS_Params_t); i++) *(dest_byte++) = *(src_byte++);
	return UTIL_SUCCESS;

}


