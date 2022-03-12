
#ifndef COMMSRX_H
#define COMMSRX_H

// Event Definitions
#include "PackDataHandler.h"
#include "ES_Framework.h"
#include "ES_Types.h"

enum { IS_STATUS, IS_TEAMDATA, IS_BROADCASTDATA};

enum{ NA, ISBUDDY, ISTEAM0, ISTEAM1, ISTEAM2, ISTEAM3, ISTEAM4, ISTEAM5, ISTEAM6};

void rxstoreBuf(uint8_t msg); // reads msg into char buf
static void rxCpyData();  //sorts type of data and call sub-helpers to populate packet struct
static void configStatus();  // sub-helper to send events of ES_DROPOUT / ES_REENTRY
static void switchTeam(uint8_t cnt);	// stores rx'ed packet into team packet
static void switchBroadcast(uint8_t cnt); 	// stores rx'ed packet into broadcast packet

struct TeamRxPacket realTeamRxPacket;    // stores real Team data
struct BroadcastRxPacket realB0RxPacket;    // stores real broadcast data from team 1
struct BroadcastRxPacket realB1RxPacket;    // stores real broadcast data from team 1
struct BroadcastRxPacket realB2RxPacket;    // stores real broadcast data from team 1
struct BroadcastRxPacket realB3RxPacket;    // stores real broadcast data from team 1
struct BroadcastRxPacket realB4RxPacket;    // stores real broadcast data from team 1
struct BroadcastRxPacket realB5RxPacket;    // stores real broadcast data from team 1
struct BroadcastRxPacket realB6RxPacket;    // stores real broadcast data from team 1


#endif /* COMMSRX_H */