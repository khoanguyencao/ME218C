/* 
* A file containing struct definitions and helper functions for printing transmitted/ 
* received data / packet. 
* 
* Created: BW, 5/15/21
*/
#include <PackDataHandler.h>
#include <stdio.h>
#include <string.h>

void printTeamData(struct TeamData data) { 
    printf("status: %#x\n",data.status);		
	printf("hull: %u\n", data.hull);
	printf("beam_dx_lsb: %#x\n", data.beam_dx_lsb);
	printf("beam_dx_msb: %#x\n", data.beam_dx_msb);
	printf("beam_dy_lsb: %#x\n", data.beam_dy_lsb);
	printf("beam_dy_msb: %#x\n", data.beam_dy_msb);
	printf("weaponPower: %#x\n", data.weaponPower);
	printf("navPower: %#x\n", data.navPower);
	printf("weaponCap: %#x\n", data.weaponCap);
	printf("shield: %#x\n", data.shield);
//	printf("status: %u\n",data.status);		
//	printf("hull: %u\n", data.hull);
//	printf("beam_dx_lsb: %hd\n", data.beam_dx_lsb);
//	printf("beam_dx_msb: %hd\n", data.beam_dx_msb);
//	printf("beam_dy_lsb: %hd\n", data.beam_dy_lsb);
//	printf("beam_dy_msb: %hd\n", data.beam_dy_msb);
//	printf("weaponPower: %u\n", data.weaponPower);
//	printf("navPower: %u\n", data.navPower);
//	printf("weaponCap: %u\n", data.weaponCap);
//	printf("shield: %u\n", data.shield);
}

void printBroadcastData(struct BroadcastData data) {
	printf("status: %u\n", data.status);
	printf("hull: %u\n", data.hull);
	printf("ship_x_lsb: %u\n", data.ship_x_lsb);
	printf("ship_x_msb: %u\n", data.ship_x_msb);
	printf("ship_y_lsb: %u\n", data.ship_y_lsb);
	printf("ship_y_msb: %u\n", data.ship_y_msb);
	printf("beam_dx_lsb: %hd\n", data.beam_dx_lsb);
	printf("beam_dx_msb: %hd\n", data.beam_dx_msb);
	printf("beam_dy_lsb: %hd\n", data.beam_dy_lsb);
	printf("beam_dy_msb: %hd\n", data.beam_dy_msb);
	printf("shield: %u\n", data.shield);
}

/* Functional Helper Functions for printing and clearing team, broadcast data & packet */
void printTeamRxPacket(struct TeamRxPacket data) {
    printf("startDelim: %#x\n",data.startDelim);
    printf("length: %u\n",data.length); 		
    printf("api: %u\n",data.api);
    printf("sourceAddr: %#x\n",data.sourceAddr); 
    printf("RSSI: %u\n",data.RSSI);
    printf("options: %u\n",data.options);	
    printTeamData(data.LocalRxPacket);
    printf("checkSum: %u\n",data.checkSum);
    printf("\n");

}

void printTeamTxPacket(struct TeamTxPacket data) {
    printf("startDelim: %#x\n",data.startDelim);
    printf("length: %u\n",data.length); 		
    printf("api: %u\n",data.api);
    printf("frameID: %u\n",data.frameID); 
    printf("destAddr: %#x\n",data.destAddr);
    printf("options: %u\n",data.options);	
    printTeamData(data.LocalTxPacket);
    printf("checkSum: %u\n",data.checkSum);
    printf("\n");
}

void printBroadcastRxPacket(struct BroadcastRxPacket data) {
    printf("startDelim: %#x\n",data.startDelim);
    printf("length: %u\n",data.length); 		
    printf("api: %u\n",data.api);
    printf("sourceAddr: %#x\n",data.sourceAddr); 
    printf("RSSI: %u\n",data.RSSI);
    printf("options: %u\n",data.options);	
    printBroadcastData(data.GlobalRxPacket);
    printf("checkSum: %u\n",data.checkSum);
    printf("\n");
}

void printBroadcastTxPacket(struct BroadcastTxPacket data) {
    printf("startDelim: %#x\n",data.startDelim);
    printf("length: %u\n",data.length); 		
    printf("api: %u\n",data.api);
    printf("frameID: %u\n",data.frameID); 
    printf("destAddr: %#x\n",data.destAddr);
    printf("options: %u\n",data.options);	
    printBroadcastData(data.GlobalTxPacket);
    printf("checkSum: %u\n",data.checkSum);
    printf("\n");
}
