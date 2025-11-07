/*
 * bmsConfig.h
 *
 *  Created on: Oct 31, 2025
 *      Author: Aiden O'Leary
 */

#ifndef INC_BMSPARAMS_H_
#define INC_BMSPARAMS_H_

#define configBMS_HEADER 				0xFF
#define configBMS_CELL_OVERVOLTAGE  	69
#define configBMS_CELL_UNDERVOLTAGE  	42069
#define configBMS_MAX_TEMP  			30
#define configBMS_BALANCE THRESHOLD  	30
#define configBMS_DEBUG_MODE  			21

//this value controls how many values are not included in checksum
// the pointer starts past header, and we dont want to use checksum for our checksum so default is 2
#define configBMS_CHECKSUM_PASSES		2

#pragma pack(push, 1)
typedef struct {
	uint8_t header;
    uint8_t cell_overvoltage;
    uint32_t cell_undervoltage;
    uint8_t debug_mode;
    uint8_t checksum;
} BMS_Params_t;
#pragma pack(pop)

extern BMS_Params_t bmsParams;

void updateBMSChecksum(void);

#endif /* INC_BMSPARAMS_H_ */
