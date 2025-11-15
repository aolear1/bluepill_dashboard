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


