/*
 * sd_spi.c
 *
 *  Created on: Mar 30, 2026
 *      Author: arnol
 */

#include "sd_spi.h"

static uint8_t tx_byte_stream[10];		/* Up to 10-byte byte stream */
static uint8_t rx_byte_stream[516];		/* Up to 516-byte byte stream */

static uint8_t rx_temp_buff[1];
static uint8_t tx_dummy_byte[1] = {DUMMY_BYTE};

static SD_TypeDef SD_1;
SD_HandleTypeDef micro_sd_handle;

void packet_to_byte_stream(uint8_t *stream, uint64_t packet, uint8_t size){
	int8_t count = size-1;

	uint8_t byte_pos = 0;

	while(count >= 0){	/* Should leave once we hit -1. Hopefully 0xFF will be determined to be less than 0x00 */
		stream[count] = (uint8_t)((packet & ((uint64_t)(0xFF) << byte_pos)) >> byte_pos);
		byte_pos += 8;

		count--;
	}
}

void toggle_sd_cs_pin(CS_State cs_state){
	switch(cs_state){
		case CS_LOW:
			HAL_GPIO_WritePin(MicroSD_NSS_GPIO_Port, MicroSD_NSS_Pin, GPIO_PIN_RESET);
			break;
		case CS_HIGH:
			HAL_GPIO_WritePin(MicroSD_NSS_GPIO_Port, MicroSD_NSS_Pin, GPIO_PIN_SET);
			break;
	}
}

void send_cmd_receive_response(uint8_t command, uint32_t argument, uint8_t CRC7, uint8_t response_len){
	HAL_StatusTypeDef status;
	uint64_t cmd_packet = 0x0000000000000000;	/* Will only be using 48 bits here */

	WRITE_CMD_PCKT(cmd_packet, command, argument, CRC7);

	packet_to_byte_stream(tx_byte_stream, cmd_packet, CMD_PCKT_LEN);

	/* Drive SD chip select low */
	toggle_sd_cs_pin(CS_LOW);

	/* Loop until transmission is successful OR times out */
	do{
		status = HAL_SPI_Transmit(&hspi4, tx_byte_stream, CMD_PCKT_LEN, SD_TX_TIMEOUT);
	}while((status != HAL_OK) && (status != HAL_TIMEOUT));
//	print_msg("  TX status: %u\r\n", status);

	/* Get first response byte: wait until != 0xFF */
	do{
		status = HAL_SPI_TransmitReceive(&hspi4, tx_dummy_byte, rx_temp_buff, 1, SD_RX_TIMEOUT);
	}while((status != HAL_TIMEOUT) && (rx_temp_buff[0] == 0xFFU));

	rx_byte_stream[0] = rx_temp_buff[0];
//	print_msg("  RX[0] status: %u, data: 0x%02X\r\n", status, rx_byte_stream[0]);

	/* Remaining bytes: read directly */
	for(int i = 1; i < response_len; i++){
		status = HAL_SPI_TransmitReceive(&hspi4, tx_dummy_byte, rx_temp_buff, 1, SD_RX_TIMEOUT);
		rx_byte_stream[i] = rx_temp_buff[0];

//		print_msg("  RX[%d] status: %u, data: 0x%02X\r\n", i, status, rx_byte_stream[i]);

		if(status == HAL_TIMEOUT){
			break;
		}
	}

	/* Drive SD chip select high */
	toggle_sd_cs_pin(CS_HIGH);

}

void send_cmd_receive_data(uint8_t command, uint32_t argument, uint8_t CRC7, uint32_t data_len){
	HAL_StatusTypeDef status;
	uint64_t cmd_packet = 0x0000000000000000;	/* Will only be using 48 bits here */

	WRITE_CMD_PCKT(cmd_packet, command, argument, CRC7);

	packet_to_byte_stream(tx_byte_stream, cmd_packet, CMD_PCKT_LEN);

	/* Drive SD chip select low */
	toggle_sd_cs_pin(CS_LOW);

	/* Loop until transmission is successful OR times out */
	do{
		status = HAL_SPI_Transmit(&hspi4, tx_byte_stream, CMD_PCKT_LEN, SD_TX_TIMEOUT);
	}while((status != HAL_OK) && (status != HAL_TIMEOUT));
//	print_msg("  TX status: %u\r\n", status);

	/* Get first response byte: wait until != 0xFF */
	do{
		status = HAL_SPI_TransmitReceive(&hspi4, tx_dummy_byte, rx_temp_buff, 1, SD_RX_TIMEOUT);
	}while((status != HAL_TIMEOUT) && (rx_temp_buff[0] == 0xFFU));

	rx_byte_stream[0] = rx_temp_buff[0];
//	print_msg("  RX[0] status: %u, data: 0x%02X\r\n", status, rx_byte_stream[0]);

	/* Get data token: wait until != 0xFF */
	do{
		status = HAL_SPI_TransmitReceive(&hspi4, tx_dummy_byte, rx_temp_buff, 1, SD_RX_TIMEOUT);
	}while((status != HAL_TIMEOUT) && (rx_temp_buff[0] == 0xFFU));

	rx_byte_stream[1] = rx_temp_buff[0];

	/* Remaining bytes: read directly */
	for(uint32_t i = 2; i < (data_len + CRC16_SUFFIX_LEN); i++){
		status = HAL_SPI_TransmitReceive(&hspi4, tx_dummy_byte, rx_temp_buff, 1, SD_RX_TIMEOUT);
		rx_byte_stream[i] = rx_temp_buff[0];

//		print_msg("  RX[%d] status: %u, data: 0x%02X\r\n", i, status, rx_byte_stream[i]);

		if(status == HAL_TIMEOUT){
			break;
		}
	}

	/* Drive SD chip select high */
	toggle_sd_cs_pin(CS_HIGH);
}

void set_sd_spi_pre_init_clk(void){
	/* Set communication speed to 90MHz/256 = 351.6kHz*/
	MODIFY_REG(hspi4.Instance->CR1, SPI_CR1_BR, SPI_BAUDRATEPRESCALER_256);
}

void set_sd_spi_post_init_clk(void){
	/* Set communication speed to 90MHz/4 = 22.5MHz*/
	MODIFY_REG(hspi4.Instance->CR1, SPI_CR1_BR, SPI_BAUDRATEPRESCALER_4);
}

SD_STATUS SD_Mode_Switch(void){
	SD_STATUS ret = SD_RESET;
	HAL_StatusTypeDef status;

	set_sd_spi_pre_init_clk();

	/* Drive SD chip select high */
	toggle_sd_cs_pin(CS_HIGH);

	/* Keep high for at least 74 cycles - Sending dummy byte 10 times counts for 80 cycles */
	memset(tx_byte_stream, DUMMY_BYTE, 10);

	/* Loop until transmission is successful OR times out */
	do{
		status = HAL_SPI_Transmit(&hspi4, tx_byte_stream, 10, SD_TX_TIMEOUT);
	}while((status != HAL_OK) && (status != HAL_TIMEOUT));
//	print_msg("TX status: %u\r\n", status);

	/* Reset card to switch from SD Mode to SPI Mode */
	uint8_t cmd_idx 	= CMD0;
	uint32_t argument 	= 0x00000000;			/* Stuff Bits are ignored */
	uint8_t CRC_7 		= 0x4AU;				/* From the specification */

	send_cmd_receive_response(cmd_idx, argument, CRC_7, R1_PCKT_LEN);

	/* Check CMD0 response to check reset success */
	if(rx_byte_stream[0] > 0x01U){
		print_msg("  <|^|> CMD0 Reset Failed: Error code 0x%02X detected\r\n", rx_byte_stream[0]);
		micro_sd_handle.Instance = &SD_1;
		micro_sd_handle.Status = ret;
		return ret;
	}

	uint8_t state;
	GET_IDLE_STATE(state, rx_byte_stream[0]);
	if(state == 0x00U){
		print_msg("  <|^|> CMD0 Reset Failed: Card claims it is already initialized before reset\r\n");
		ret = SD_ERROR;
		micro_sd_handle.Instance = &SD_1;
		micro_sd_handle.Status = ret;
		return ret;
	}else if(state == 0x01U){
		print_msg("  <|^|> CMD0 Reset Success: Idle state in SPI Mode running initialization process...\r\n");
		ret = SD_IDLE;
		micro_sd_handle.Instance = &SD_1;
		micro_sd_handle.Status = ret;
		return ret;
	}

	micro_sd_handle.Status = ret;

	return ret;	/* Should never reach here */
}

SD_STATUS SD_Initialize(void){
	SD_STATUS ret = SD_IDLE;

	/* Set communication speed to 90MHz/256 = 351.6kHz */
	set_sd_spi_pre_init_clk();

	/* Request SD Memory Card interface conditions */
	uint8_t cmd_idx 	= CMD8;
	uint32_t argument 	= (uint32_t)((0x00UL << 12U) | (0x01UL << 8) | (CHECK_PATTERN));
	uint8_t CRC_7 		= 0x43U;							/* From the specification */

	send_cmd_receive_response(cmd_idx, argument, CRC_7, R7_PCKT_LEN);

	/* Validate check pattern echo from SD Card */
	uint16_t attempts = 0;
	while((rx_byte_stream[4] != CHECK_PATTERN) && (attempts < 100)){
		/* Request SD Memory Card interface conditions again */
		send_cmd_receive_response(cmd_idx, argument, CRC_7, R7_PCKT_LEN);

		attempts++;
	}

	if(!(attempts < 100)){
		print_msg("  <|^|> CMD8 SEND_IF_COND Failed: Mismatch in check pattern.\r\n        "
						"Expected 0xAA...\r\n        "
						"Received 0x%02X\r\n", rx_byte_stream[4]);
		micro_sd_handle.Status = ret;
		return ret;
	}

	/* Return if other errors other than illegal error are present in response */
	uint8_t state;
//	IS_ILLEGAL_CMD(state, rx_byte_stream[0]);
	if((rx_byte_stream[0] != 0x05U) && (rx_byte_stream[0] != 0x01U)){
		print_msg("  <|^|> CMD8 SEND_IF_COND Failed: Error code 0x%02X detected.\r\n", rx_byte_stream[0]);
		micro_sd_handle.Status = ret;
		return ret;
	}

	/* Begin going through SPI Mode Initialization Flow */
	if(rx_byte_stream[0] == 0x05U){	/* Illegal Command */
		/* Read OCR Register */
		cmd_idx 	= CMD58;
		argument 	= 0x00000000;			/* Stuff Bits are ignored */
		CRC_7 		= 0x01U;				/* Dummy CRC7 bytes */

		send_cmd_receive_response(cmd_idx, argument, CRC_7, R3_PCKT_LEN);

		/* Check if card supports 3.2V-3.4V voltage range */
		uint8_t temp = rx_byte_stream[2];	/* OCR[23:16] - Bits 20 or 21 <--> Bits 4 or 5 of rx_byte_stream[2] */
		if( ((temp & 0x10U) == 0x00U) && ((temp & 0x20U) == 0X00U )){
			print_msg("  <|^|> CMD58 SEND_IF_COND Failed: Ver1.X SD Card does not support requested voltage.\r\n");
			ret = SD_VOLTAGE_MISMATCH;
			micro_sd_handle.Status = ret;
			return ret;
		}

		/* Check card type using application specific command: ACMD41
		 *
		 * 	Need to send CMD55 first
		 * */
		cmd_idx 	= CMD55;
		argument 	= 0x00000000;			/* Stuff Bits are ignored */
		CRC_7 		= 0x01U;				/* Dummy CRC7 bytes */

		send_cmd_receive_response(cmd_idx, argument, CRC_7, R1_PCKT_LEN);

		GET_IDLE_STATE(state, rx_byte_stream[0]);
		if(state != 0x01U){
			print_msg("  <|^|> CMD55 Reset Failed: Error code 0x%02X detected.\r\n", rx_byte_stream[0]);
			micro_sd_handle.Status = ret;
			return ret;
		}

		/* Now we send ACMD41 */
		cmd_idx 	= ACMD41;
		argument 	= (0x00U << 30);		/* Ask if Host Capacity Support is Standard Capacity */
		CRC_7 		= 0x01U;				/* Dummy CRC7 bytes */

		send_cmd_receive_response(cmd_idx, argument, CRC_7, R1_PCKT_LEN);

		GET_IDLE_STATE(state, rx_byte_stream[0]);

		/* Repeatedly issue ACMD41 until idle state bit is set to 0*/
		attempts = 0;
		while((state != 0x00U) && (attempts < 1000)){
			send_cmd_receive_response(CMD55, 0x00000000, CRC_7, R1_PCKT_LEN);

			send_cmd_receive_response(cmd_idx, argument, CRC_7, R1_PCKT_LEN);

			GET_IDLE_STATE(state, rx_byte_stream[0]);

			attempts++;
		}

		if(!(attempts < 1000)){
			print_msg("  <|^|> ACMD41 Initialization Failed: Not SD Card or no card\r\n");
			ret = SD_NO_CARD;
			micro_sd_handle.Status = ret;
			return ret;
		}

		print_msg("  <|^|> ACMD41 Initialization Success...\r\n");
		print_msg("  	_______________________________________\r\n");
		print_msg("  	| 		Ver1.X Standard Capacity      |\r\n");
		print_msg("  	| 		 SD Memory Card (SDSC)		  |\r\n");
		print_msg("  	|_____________________________________|\r\n");

		set_sd_spi_post_init_clk();

		ret = SD_READY;

		micro_sd_handle.Status = ret;

		return ret;

	}else if(rx_byte_stream[0] == 0x01U){	/* Responds without illegal command */
		/* Check if card supports 2.7V-3.6V voltage range */
		uint8_t temp = (rx_byte_stream[3] & 0x0FU);			/* R7[11:8] - Extract the lower 4 bits */
		if(temp != 0x01U){
			print_msg("  <|^|> CMD8 SEND_IF_COND Failed: Ver2.00 or later SD Card does not support requested voltage.\r\n");
			ret = SD_VOLTAGE_MISMATCH;
			micro_sd_handle.Status = ret;
			return ret;
		}
		print_msg("  <|^|> CMD8 SEND_IF_COND Success: Ver2.00 or later SD Card supports 2.7V-3.6V voltage range.\r\n");

		/* Read OCR Register */
		cmd_idx 	= CMD58;
		argument 	= 0x00000000;			/* Stuff Bits are ignored */
		CRC_7 		= 0x01U;				/* Dummy CRC7 bytes */

		send_cmd_receive_response(cmd_idx, argument, CRC_7, R3_PCKT_LEN);

		/* Check if card supports 3.2V-3.4V voltage range */
		temp = rx_byte_stream[2];	/* OCR[23:16] - Bits 20 or 21 <--> Bits 4 or 5 of rx_byte_stream[2] */
		if( ((temp & 0x10U) == 0x00U) && ((temp & 0x20U) == 0X00U )){
			print_msg("  <|^|> CMD58 SEND_IF_COND Failed: Ver2.00 or later SD Card does not support requested voltage.\r\n");
			ret = SD_VOLTAGE_MISMATCH;
			micro_sd_handle.Status = ret;
			return ret;
		}
		print_msg("  <|^|> CMD58 SEND_IF_COND Success: Ver2.00 or later SD Card supports 3.2V-3.4V voltage range.\r\n");

		/* Check card type using application specific command: ACMD41
		 *
		 * 	Need to send CMD55 first
		 * */
		cmd_idx 	= CMD55;
		argument 	= 0x00000000;			/* Stuff Bits are ignored */
		CRC_7 		= 0x01U;				/* Dummy CRC7 bytes */

		send_cmd_receive_response(cmd_idx, argument, CRC_7, R1_PCKT_LEN);

		GET_IDLE_STATE(state, rx_byte_stream[0]);
		if(state != 0x01U){
			print_msg("  <|^|> CMD55 Reset Failed: Error code 0x%02X detected.\r\n", rx_byte_stream[0]);
			micro_sd_handle.Status = ret;
			return ret;
		}

		/* Now we send ACMD41 */
		cmd_idx 	= ACMD41;
		argument 	= (0x00U << 30);		/* Ask if Host Capacity Support is Standard Capacity */
		CRC_7 		= 0x01U;				/* Dummy CRC7 bytes */

		send_cmd_receive_response(cmd_idx, argument, CRC_7, R1_PCKT_LEN);

		GET_IDLE_STATE(state, rx_byte_stream[0]);

		/* Repeatedly issue ACMD41 until idle state bit is set to 0*/
		attempts = 0;
		while((state != 0x00U) && (attempts < 100)){
			send_cmd_receive_response(CMD55, 0x00000000, CRC_7, R1_PCKT_LEN);

			send_cmd_receive_response(cmd_idx, argument, CRC_7, R1_PCKT_LEN);

			GET_IDLE_STATE(state, rx_byte_stream[0]);

			attempts++;
		}

		if(!(attempts < 1000)){
			print_msg("  <|^|> ACMD41 Initialization Failed: Not SD Card or no card\r\n");
			ret = SD_NO_CARD;
			micro_sd_handle.Status = ret;
			return ret;
		}

		/* Get CCS from Reading OCR Register again */
		send_cmd_receive_response(CMD58, 0x00000000, CRC_7, R3_PCKT_LEN);

		temp = (rx_byte_stream[1] & (0x03U << 6)) >> 6;	/* OCR[31:24] - Bits 30 & 31 <--> Bits 6 & 7 of rx_byte_stream[1] */

		if(temp == 0x02U){

			print_msg("  <|^|> ACMD41 Initialization Success...\r\n");
			print_msg("  	_______________________________________\r\n");
			print_msg("  	|           Ver2.00 or later          |\r\n");
			print_msg("  	|          Standard Capacity          |\r\n");
			print_msg("  	|        SD Memory Card (SDSC)        |\r\n");
			print_msg("  	|_____________________________________|\r\n");

			set_sd_spi_post_init_clk();

			ret = SD_READY;

			micro_sd_handle.Status = ret;

			return ret;

		}else if(temp == 0x03U){

			print_msg("  <|^|> ACMD41 Initialization Success...\r\n");
			print_msg("  	_______________________________________\r\n");
			print_msg("  	|           Ver2.00 or later          |\r\n");
			print_msg("  	|  High Capacity or Extended Capacity |\r\n");
			print_msg("  	|     SD Memory Card (SDHC/SDXC)      |\r\n");
			print_msg("  	|_____________________________________|\r\n");

			set_sd_spi_post_init_clk();

			ret = SD_READY;

			micro_sd_handle.Status = ret;

			return ret;
		}else{
			print_msg("<|^|> CMD58 Get CCS Failed: Initialized but somehow power sequence is not finished yet\r\n");
			ret = SD_ERROR;
			micro_sd_handle.Status = ret;
			return ret;
		}
	}

	micro_sd_handle.Status = ret;

	return ret;	/* Ideally, should never get here */
}

SD_STATUS SD_Read_Single_Block(uint32_t block_address, uint8_t *rbuff, uint32_t block_size){
	SD_STATUS ret = SD_READY;
	micro_sd_handle.Status = ret;

	/* Set block length to designated value */
	uint8_t cmd_idx 	= CMD16;
	uint32_t argument 	= (block_size > 512) ? 512 : block_size;	/* Cannot be bigger than 512 bytes */
	uint8_t CRC_7 		= 0x01U;									/* From the specification */

	send_cmd_receive_response(cmd_idx, argument, CRC_7, R1_PCKT_LEN);

	if(rx_byte_stream[0] != 0x00U){
		print_msg("<|^|> CMD16 Set Block Length Failed: Error code 0x%02X detected.\r\n", rx_byte_stream[0]);
		ret = SD_ERROR;
		micro_sd_handle.Status = ret;
		return ret;
	}
//	print_msg("<|^|> CMD16 Set Block Length Success...\r\n", rx_byte_stream[0]);

	/* Read block at designated address */
	cmd_idx 	= CMD17;
	argument 	= (block_address > 0xFFFFFFFF) ? 0xFFFFFFFF : block_address;	/* Cannot be bigger than 512 bytes */
	CRC_7 		= 0x01U;							/* From the specification */

	send_cmd_receive_data(cmd_idx, argument, CRC_7, block_size);	/* Need to check block_size here again */

	if(rx_byte_stream[0] != 0x00U){
		print_msg("<|^|> CMD17 Read Single Block Failed: Error code 0x%02X detected.\r\n", rx_byte_stream[0]);
		ret = SD_ERROR;
		micro_sd_handle.Status = ret;
		return ret;
	}

	/* Check if data token was received correctly */
	if(rx_byte_stream[1] != SINGLE_RW_DATA_TOKEN){
		print_msg("  <|^|> CMD17 Read Single Block Failed: Mismatch in data token.\r\n        "
							"Expected 0xFE...\r\n        "
							"Received 0x%02X\r\n", rx_byte_stream[1]);
		ret = SD_ERROR;
		micro_sd_handle.Status = ret;
		return ret;
	}
//	print_msg("<|^|> CMD17 Read Single Block Success...\r\n", rx_byte_stream[0]);

	micro_sd_handle.SD_Buffer = rx_byte_stream + 2;
	memcpy(rbuff, &rx_byte_stream[2], block_size);

//	print_msg("        rbuff[510] = 0x%02X\r\n        rbuff[511] = 0x%02X\r\n", rbuff[510], rbuff[511]);

	ret = SD_READY;

	micro_sd_handle.Status = ret;

	return ret;

}

SD_STATUS SD_Read_Multiple_Block(uint32_t block_address, uint8_t *rbuff, uint32_t block_count);

