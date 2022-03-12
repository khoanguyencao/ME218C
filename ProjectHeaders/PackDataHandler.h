#ifndef PACKDATA_H
#define PACKDATA_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

/* 10 bytes data */
struct TeamData {
    uint8_t status;		
    uint8_t hull;
    int8_t beam_dx_lsb;
    int8_t beam_dx_msb;
    int8_t beam_dy_lsb;
    int8_t beam_dy_msb;
    uint8_t weaponPower;
    uint8_t navPower;
    uint8_t weaponCap;
    uint8_t shield;
};

// Packet to send 
struct TeamTxPacket {
	uint8_t startDelim; 		
	uint16_t length;
	uint8_t api;
	uint8_t frameID; 
	uint16_t destAddr;
	uint8_t options;	
	struct TeamData LocalTxPacket;
	uint8_t checkSum;
};

// Empty packet to populate
struct TeamRxPacket {
	uint8_t startDelim; 		
	uint16_t length;
	uint8_t api; 		
	uint16_t sourceAddr; 
	uint8_t RSSI;
	uint8_t options;	
	struct TeamData LocalRxPacket;  
	uint8_t checkSum;
};



/* 11 bytes data */

struct BroadcastData {
    uint8_t status;		
    uint8_t hull;
    uint8_t ship_x_lsb;
    uint8_t ship_x_msb;
    uint8_t ship_y_lsb;
    uint8_t ship_y_msb;
    int8_t beam_dx_lsb;
    int8_t beam_dx_msb;
    int8_t beam_dy_lsb;
    int8_t beam_dy_msb;
    uint8_t shield;
};

struct BroadcastTxPacket {
	uint8_t startDelim; 		
	uint16_t length;
	uint8_t api;
	uint8_t frameID; 
	uint16_t destAddr;
	uint8_t options;	
	struct BroadcastData GlobalTxPacket;
	uint8_t checkSum;
};


struct BroadcastRxPacket {
	uint8_t startDelim;
	uint16_t length;
	uint8_t api; 		
	uint16_t sourceAddr; 
	uint8_t RSSI;
	uint8_t options;	
	struct BroadcastData GlobalRxPacket;  
	uint8_t checkSum;
};

void printTeamData(struct TeamData data) ;
void printBroadcastData(struct BroadcastData data);
void printTeamRxPacket(struct TeamRxPacket data) ;
void printTeamTxPacket(struct TeamTxPacket data) ;
void printBroadcastRxPacket(struct BroadcastRxPacket data);
void printBroadcastTxPacket(struct BroadcastTxPacket data); 

#endif /* PACKDATA_H */