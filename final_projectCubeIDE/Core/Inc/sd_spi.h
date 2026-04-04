/*
 * sd_spi.h
 *
 *  Created on: Mar 30, 2026
 *      Author: arnol
 */

#ifndef INC_SD_SPI_H_
#define INC_SD_SPI_H_

#include "project.h"

#define MAX_SD_OPERATING_FREQ	25000000
#define SD_TX_TIMEOUT			10
#define SD_RX_TIMEOUT			1000

#define DATA_TOKEN_LEN			1
#define CRC16_SUFFIX_LEN		2

#define CMD_PCKT_LEN	6		/* Length in Bytes */

#define R1_PCKT_LEN		1		/* Length in Bytes */
#define R2_PCKT_LEN		2		/* Length in Bytes */
#define R3_PCKT_LEN		5		/* Length in Bytes */
#define R7_PCKT_LEN		5		/* Length in Bytes */

/* Useful Command Indexes */
#define CMD0			0x00U	/* Resets the SD Memory Card - R1 */
#define CMD8			0x08U	/* Sends SD Memory Card interface conditions - R7 */

#define CMD9			0x09U	/* Send CSD - R1 */
#define CMD10			0x0AU	/* Send CID - R1 */

#define CMD12			0x0CU	/* Stop transmission in multiple read block operation - R1b */

#define CMD13			0x0DU	/* Send status register - R2 */

#define CMD16			0x10U	/* Set Block Length - R1 */
#define CMD17			0x11U	/* Read Single Block - R1 */
#define CMD18			0x12U	/* Read Multiple Block - R1 */

#define CMD55			0x37U	/* Defines to card next cmd is application specific cmd - R1 */

#define CMD58			0x3AU	/* Read OCR Register - R3 */

#define ACMD41			0x29U	/* Sends host capacity support info and activate's card init process - R1 */

#define DUMMY_BYTE		0xFFU
#define CHECK_PATTERN	0xAAUL	/* Used to check validity of MCU to SD card communication */

#define SINGLE_RW_DATA_TOKEN		0xFEU
#define MULTIPLE_READ_DATA_TOKEN	0xFEU
#define MULTIPLE_WRITE_DATA_TOKEN	0xFCU

/* Command Field Masks */
#define CMD8_VCA_MASK_Pos			8U
#define CMD8_VCA_MASK				(0x0F << CMD8_VCA_MASK_Pos)

/* Response Field Masks */
#define R1_IDLE_STATE_MASK_Pos		0U
#define R1_IDLE_STATE_MASK			(0x01U << R1_IDLE_STATE_MASK_Pos)

#define R1_ILLEGAL_STATE_MASK_Pos	2U
#define R1_ILLEGAL_STATE_MASK		(0x01U << R1_ILLEGAL_STATE_MASK_Pos)

typedef enum{
	SD_RESET = 0,	/* Mode of operation not yet SPI Mode (Default: SD Mode) */
	SD_IDLE,		/* In SPI Mode */
	SD_READY,		/* Successfully initialized */
	SD_BUSY,
	SD_TIMEOUT,
	SD_VOLTAGE_MISMATCH,
	SD_NO_CARD,
	SD_ERROR
}SD_STATUS;

typedef enum{
	CS_LOW = 0,
	CS_HIGH
}CS_State;

typedef struct{
	/* FATFS Necessary Registers */
	uint32_t OCR;		/* Operating Conditions Register */

	uint64_t CID_H;		/* Card IDentification Register upper 64-bits */
	uint64_t CID_L;		/* Card IDentification Register lower 64-bits */

	uint64_t CSD_H;		/* Card-Specific Data Register upper 64-bits */
	uint64_t CSD_L;		/* Card-Specific Data Register lower 64-bits */

	/* Optional Registers */
	uint16_t RCA;		/* Relative Card Address register */

	uint16_t DSR;		/* Driver Stage Register */

	uint64_t SCR;		/* SD CARD Configuration Register */
}SD_TypeDef;

typedef struct{
	SD_TypeDef 	*Instance;

	SD_STATUS 	Status;

	uint8_t		*SD_Buffer;
}SD_HandleTypeDef;

#define OCR_2V7_2V8_MASK_Pos	15U
#define OCR_2V7_2V8_MASK		(0x00000001 << OCR_2V7_2V8_MASK_Pos)

#define OCR_2V8_2V9_MASK_Pos	16U
#define OCR_2V8_2V9_MASK		(0x00000001 << OCR_2V8_2V9_MASK_Pos)

#define OCR_2V9_3V0_MASK_Pos	17U
#define OCR_2V9_3V0_MASK		(0x00000001 << OCR_2V9_3V0_MASK_Pos)

#define OCR_3V0_3V1_MASK_Pos	18U
#define OCR_3V0_3V1_MASK		(0x00000001 << OCR_3V0_3V1_MASK_Pos)

#define OCR_3V1_3V2_MASK_Pos	19U
#define OCR_3V1_3V2_MASK		(0x00000001 << OCR_3V1_3V2_MASK_Pos)

#define OCR_3V2_3V3_MASK_Pos	20U
#define OCR_3V2_3V3_MASK		(0x00000001 << OCR_3V2_3V3_MASK_Pos)

#define OCR_3V3_3V4_MASK_Pos	21U
#define OCR_3V3_3V4_MASK		(0x00000001 << OCR_3V3_3V4_MASK_Pos)

#define OCR_3V4_3V5_MASK_Pos	22U
#define OCR_3V4_3V5_MASK		(0x00000001 << OCR_3V4_3V5_MASK_Pos)

#define OCR_3V5_3V6_MASK_Pos	23U
#define OCR_3V5_3V6_MASK		(0x00000001 << OCR_3V5_3V6_MASK_Pos)

#define OCR_CCS_MASK_Pos		30U
#define OCR_CCS_MASK			(0x00000001 << OCR_CCS_MASK_Pos)

#define OCR_POW_STATUS_MASK_Pos	31U
#define OCR_POW_STATUS_MASK		(0x00000001 << OCR_POW_STATUS_MASK_Pos)


#define WRITE_CMD_PCKT(__PACKET__, __INDEX__, __ARGUMENT__, __CRC7__) 									\
		do{																								\
			(__PACKET__) = ((uint64_t)(0x00U) << 47U) | ((uint64_t)(0x01U) << 46U) | ((uint64_t)(__INDEX__) << 40U) | \
			((uint64_t)(__ARGUMENT__) << 8U) | 	((uint8_t)(__CRC7__) << 1U) | (0x01U);					\
		}while(0)

#define GET_IDLE_STATE(__STATE__, __RESPONSE__)															\
		do{																								\
			(__STATE__) = (uint8_t)((__RESPONSE__ & R1_IDLE_STATE_MASK) >>  R1_IDLE_STATE_MASK_Pos);	\
		}while(0)

#define IS_ILLEGAL_CMD(__STATE__, __RESPONSE__)	\
		do{																								\
			(__STATE__) = (uint8_t)((__RESPONSE__ & R1_ILLEGAL_STATE_MASK) >>  R1_ILLEGAL_STATE_MASK_Pos);	\
		}while(0)

extern SD_HandleTypeDef micro_sd_handle;;

/* Private Function Prototypes */
SD_STATUS SD_Mode_Switch(void);
SD_STATUS SD_Initialize(void);
SD_STATUS SD_Read_Single_Block(uint32_t block_address, uint8_t *rbuff, uint32_t block_size);
SD_STATUS SD_Read_Multiple_Block(uint32_t block_address, uint8_t *rbuff, uint32_t block_count);

void packet_to_byte_stream(uint8_t *stream, uint64_t packet, uint8_t size);

void send_cmd_receive_response(uint8_t command, uint32_t argument, uint8_t CRC7, uint8_t response_len);
void send_cmd_receive_data(uint8_t command, uint32_t argument, uint8_t CRC7, uint32_t data_len);

void toggle_sd_cs_pin(CS_State cs_state);
void set_sd_spi_pre_init_clk(void);
void set_sd_spi_post_init_clk(void);

#endif /* INC_SD_SPI_H_ */
