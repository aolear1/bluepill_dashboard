/*
 * utils.h
 *
 *  Created on: Nov 15, 2025
 *      Author: aiden
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum utilStatus {UTIL_SUCCESS = 0, UTIL_NULL_ERROR = 1, UTIL_INVALID_INPUT = 2};

/**
 * @brief Calculates a checksum for an inputed number of bytes
 *
 *
 * @param start 	A pointer to the start of the array of bytes to checksum.
 * @param size		Number of bytes to checksum
 * @param checksum	Pointer to address to store checksum
 *
 * @return 			utilStatus enum describing success or issue
 */
enum utilStatus calculateChecksum(uint8_t *start, uint32_t size, uint8_t *checksum);

#ifdef __cplusplus
}
#endif

#endif /* INC_UTILS_H_ */
