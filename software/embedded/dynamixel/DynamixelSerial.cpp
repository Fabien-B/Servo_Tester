/*
 * DynamixelSerial.cpp
 *
 *  Created on: 24 janv. 2019
 *      Author: guilhem
 */

#include "DynamixelSerial.h"
#include <ch.h>
#include <hal.h>
#include <string.h>
#include "stdutil.h"

#define NO_OP asm("NOP")//__asm nop

DynamixelSerial::DynamixelSerial(unsigned int serialNumber): _serialNumber(serialNumber){
	_error = 0;
}

int DynamixelSerial::reset(unsigned char ID){
	sendInstruction(ID, I_Reset, nullptr, 0);
	return readResponse(nullptr);
}

int DynamixelSerial::ping(unsigned char ID){
	sendInstruction(ID, I_Ping, nullptr, 0);
	return readResponse(nullptr);
}

int DynamixelSerial::setID(unsigned char ID, unsigned char newID){
	uint8_t data[2];
	data[0] = R_ServoID;
	data[1] = newID;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setBD(unsigned char ID, long baud){
	unsigned char baud_rate = (2000000/baud) - 1;
	uint8_t data[2];
	data[0] = R_BaudRate;
	data[1] = baud_rate;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::move(unsigned char ID, int position){
	uint8_t data[3];
	data[0] = R_GoalPosition;
	data[1] = position & 0xFF;
	data[2] = (position >> 8) & 0xFF;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	int r = readResponse(nullptr);
	return r;
}

int DynamixelSerial::moveSpeed(unsigned char ID, int position, int speed){
	uint8_t data[5];
	data[0] = R_GoalPosition;
	data[1] = position & 0xFF;
	data[2] = (position >> 8) & 0xFF;
	data[3] = speed & 0xFF;
	data[4] = (speed >> 8) & 0xFF;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setEndless(unsigned char ID, bool status){
	uint8_t data[3];
	if (status){
		data[0] = R_CCW_AngleLimit;
		data[1] = 0;
		data[2] = 0;
	}else{
		turn(ID, RotationDirection::Clockwise, 0);
		data[0] = R_CCW_AngleLimit;
		data[1] = 0xFF;
		data[2] = 0x03;
	}
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::turn(unsigned char ID, RotationDirection direction, int speed){
	uint8_t data[3];
	data[0] = R_MovingSpeed;
	data[1] = speed & 0xFF;
	data[2] = (speed >> 8) & 0xFF;
	data[2] |= ((int)direction) << 2;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::moveRW(unsigned char ID, int position){
	uint8_t data[3];
	data[0] = R_GoalPosition;
	data[1] = position & 0xFF;
	data[2] = (position >> 8) & 0xFF;
	sendInstruction(ID, I_RegWrite, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::moveSpeedRW(unsigned char ID, int position, int speed){
	uint8_t data[5];
	data[0] = R_GoalPosition;
	data[1] = position & 0xFF;
	data[2] = (position >> 8) & 0xFF;
	data[3] = speed & 0xFF;
	data[4] = (speed >> 8) & 0xFF;
	sendInstruction(ID, I_RegWrite, data, sizeof(data));
	return readResponse(nullptr);
}

void DynamixelSerial::action(){
	sendInstruction(0xFE, I_Action, nullptr, 0);
}

int DynamixelSerial::torqueStatus(unsigned char ID, bool status){
	uint8_t data[2];
	data[0] = R_TorqueEnable;
	data[1] = status ? 1 : 0;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::ledStatus(unsigned char ID, bool status){
	uint8_t data[2];
	data[0] = R_LED;
	data[1] = status ? 1 : 0;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::readTemperature(unsigned char ID){
	uint8_t data[2];
	data[0] = R_PresentTemperature;
	data[1] = 1;  // Temperature is only 1 byte
	sendInstruction(ID, I_ReadData, data, sizeof(data));
	int ret = readResponse(data);
	if(ret == 0) {
		return data[0];
	}
	return -1;
}

int DynamixelSerial::readPosition(unsigned char ID){
	uint8_t data[2];
	data[0] = R_PresentPosition;
	data[1] = 2;  // Position is 2 bytes long
	sendInstruction(ID, I_ReadData, data, sizeof(data));
	int ret = readResponse(data);
	if(ret == 0) {
		return (data[1]<<8) | data[0];
	}
	return -1;
}

int DynamixelSerial::readVoltage(unsigned char ID){
	uint8_t data[2];
	data[0] = R_PresentVoltage;
	data[1] = 1;  // Voltage is 1 byte long
	sendInstruction(ID, I_ReadData, data, sizeof(data));
	int ret = readResponse(data);
	if(ret == 0) {
		return data[0];
	}
	return -1;
}

int DynamixelSerial::setTempLimit(unsigned char ID, unsigned char temperature){
	uint8_t data[2];
	data[0] = R_HighestLimitTemperature;
	data[1] = temperature;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setVoltageLimit(unsigned char ID, unsigned char DVoltage, unsigned char UVoltage){
	uint8_t data[3];
	data[0] = R_LowestLimitVoltage;
	data[1] = DVoltage;
	data[2] = UVoltage;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setAngleLimit(unsigned char ID, int CWLimit, int CCWLimit){
	uint8_t data[6];
	data[0] = R_CW_AngleLimit;
	data[1] = CWLimit & 0xFF;
	data[2] = (CWLimit >> 8) & 0xFF;
	data[3] = R_CCW_AngleLimit;
	data[4] = CCWLimit & 0xFF;
	data[5] = (CCWLimit >> 8) & 0xFF;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setMaxTorque(unsigned char ID, int maxTorque){
	uint8_t data[3];
	data[0] = R_MaxTorque;
	data[1] = maxTorque & 0xFF;
	data[2] = (maxTorque >> 8) & 0xFF;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setSRL(unsigned char ID, unsigned char SRL){
	uint8_t data[2];
	data[0] = R_StatusReturnLevel;
	data[1] = SRL;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setRDT(unsigned char ID, unsigned char RDT){
	uint8_t data[2];
	data[0] = R_ReturnDelayTime;
	data[1] = RDT;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setLEDAlarm(unsigned char ID, unsigned char LEDAlarm){
	uint8_t data[2];
	data[0] = R_AlarmLED;
	data[1] = LEDAlarm;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setShutdownAlarm(unsigned char ID, unsigned char SALARM){
	uint8_t data[2];
	data[0] = R_AlarmShutdown;
	data[1] = SALARM;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setCMargin(unsigned char ID, unsigned char CWCMargin, unsigned char CCWCMargin){
	uint8_t data[4];
	data[0] = R_CW_ComplianceMargin;
	data[1] = CWCMargin;
	data[2] = R_CCW_ComplianceMargin;
	data[3] = CCWCMargin;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setCSlope(unsigned char ID, unsigned char CWCSlope, unsigned char CCWCSlope){
	uint8_t data[4];
	data[0] = R_CW_ComplianceSlope;
	data[1] = CWCSlope;
	data[2] = R_CCW_ComplianceSlope;
	data[3] = CCWCSlope;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::setPunch(unsigned char ID, int punch){
	uint8_t data[3];
	data[0] = R_Punch;
	data[1] = punch & 0xFF;
	data[2] = (punch >> 8) & 0xFF;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::moving(unsigned char ID){
	uint8_t data[2];
	data[0] = R_Moving;
	data[1] = 1;  // moving is 1 byte long
	sendInstruction(ID, I_ReadData, data, sizeof(data));
	int ret = readResponse(data);
	if(ret == 0) {
		return data[0];
	}
	return -1;
}

int DynamixelSerial::lockRegister(unsigned char ID){
	uint8_t data[2];
	data[0] = R_Lock;
	data[1] = 1;
	sendInstruction(ID, I_WriteData, data, sizeof(data));
	return readResponse(nullptr);
}

int DynamixelSerial::RWStatus(unsigned char ID){
	uint8_t data[2];
	data[0] = R_RegisteredInstruction;
	data[1] = 1;  // Registered Instruction is 1 byte long
	sendInstruction(ID, I_ReadData, data, sizeof(data));
	int ret = readResponse(data);
	if(ret == 0) {
		return data[0];
	}
	return -1;
}

int DynamixelSerial::readSpeed(unsigned char ID){
	uint8_t data[2];
	data[0] = R_PresentSpeed;
	data[1] = 2;  // Speed is 2 bytes long
	sendInstruction(ID, I_ReadData, data, sizeof(data));
	int ret = readResponse(data);
	if(ret == 0) {
		return (data[1]<<8) | data[0];
	}
	return -1;
}

int DynamixelSerial::readLoad(unsigned char ID){
	uint8_t data[2];
	data[0] = R_PresentLoad;
	data[1] = 2;  // Load is 2 bytes long
	sendInstruction(ID, I_ReadData, data, sizeof(data));
	int ret = readResponse(data);
	if(ret == 0) {
		return (data[1]<<8) | data[0];
	}
	return -1;
}


int DynamixelSerial::sendInstruction(uint8_t ID, uint8_t instruction, uint8_t* params, int paramsLength){
	uint8_t txPacket[SERIAL_BUFFERS_SIZE];
//	// Send instruction
	size_t s=0;
	txPacket[s++] = 0xFF;
	txPacket[s++] = 0xFF; // Start bytes
	txPacket[s++] = ID;
	txPacket[s++] = paramsLength + 2; // Total size = instruction + params + checksum
	txPacket[s++] = instruction;
	for (int i=0; i < paramsLength; i++){
		txPacket[s++] = params[i];
	}
	uint8_t checksum = 0;
	for (size_t i = 2; i<s; i++){
		checksum += txPacket[i];
	}
	txPacket[s++] = ~checksum;

	uint8_t rxPacket[SERIAL_BUFFERS_SIZE];
	// read all input buffer to clean it.
	sdReadTimeout(&SD1, rxPacket, SERIAL_BUFFERS_SIZE, TIME_IMMEDIATE);

	sdWrite(&SD1, txPacket, s);

	//uint8_t rxPacket[100];
	volatile size_t n = sdRead(&SD1, rxPacket, s);

	if(n < s || memcmp(txPacket, rxPacket, s) != 0) {
		//read bytes are not the same as send bytes. Something's wrong. (simultaneous communication ?)
		NO_OP;
		return -1;
	}
	return 0;
}

int DynamixelSerial::readResponse(uint8_t *params){
	uint8_t incoming[MAX_READ_PARAMS_NUMBER];
	uint8_t error = 255;
	uint8_t tmp_checksum = 0;
	uint8_t checksum = 0;
	volatile uint8_t dyn_id = 255;
	(void)dyn_id;
	eStatusReceiveState _receive_state = RECEIVE_IDLE;
	size_t _nb_bytes_to_read = 1;
	size_t _nb_params = 0;

	while(true) {

		if(_nb_bytes_to_read > 0) {
			if(sdReadTimeout(&SD1, incoming, _nb_bytes_to_read, 500) < _nb_bytes_to_read) {
				_receive_state = RECEIVE_IDLE;
				return -1;	//timeout error
			}
			for(size_t i=0; i<_nb_bytes_to_read; i++) {
				tmp_checksum += incoming[i];
			}
		}

		switch(_receive_state) {
		case RECEIVE_IDLE:
			if(incoming[0] != 0xFF) {
				_receive_state = RECEIVE_IDLE;
				NO_OP;
				return -1;	//error, expected 0xFF, got something else
			}
			_nb_bytes_to_read = 1;
			_receive_state = RECEIVE_GOT_FF_1;
			break;
		case RECEIVE_GOT_FF_1:
			if(incoming[0] != 0xFF) {
				_receive_state = RECEIVE_IDLE;
				NO_OP;
				return -1;	//error, expected 0xFF, got something else
			}
			_nb_bytes_to_read = 1;
			tmp_checksum = 0;
			_receive_state = RECEIVE_ID;
			break;
		case RECEIVE_ID:
			dyn_id = incoming[0];
			_nb_bytes_to_read = 1;
			_receive_state = RECEIVE_LENGHT;
			break;
		case RECEIVE_LENGHT:
			_nb_params = incoming[0] - 2;
			_nb_bytes_to_read = 1;
			_receive_state = RECEIVE_ERROR;
			break;
		case RECEIVE_ERROR:
			error = incoming[0];
			_nb_bytes_to_read = _nb_params;
			_receive_state = RECEIVE_PARAMS;
			break;
		case RECEIVE_PARAMS:
			NO_OP;
			if(params != nullptr) {
				memcpy(params, incoming, _nb_params);
			}
			_nb_bytes_to_read = 1;
			checksum = ~tmp_checksum;
			_receive_state = RECEIVE_CHK;
			break;
		case RECEIVE_CHK:
			volatile uint8_t received_chk = incoming[0];
			if(checksum == received_chk) {
				//checksum ok
				NO_OP;
				_error = error;	//update error flag only if com was successful
				return 0;
			} else {
				//checksum wrong
				return -1;
			}
			break;
		}

	}
return 0;
}


