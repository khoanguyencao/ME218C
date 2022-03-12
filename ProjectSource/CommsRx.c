// consequtively 5 NAKs for team Tx msg, or 5 CCAs for broadcast Tx -> Sets flag IM_OUT, posts 
// If IM_OUT flag is on, 
#include "Comms.h"
#include "CommsTx.h"
#include "CommsRx.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PackDataHandler.h"
#include <string.h>

#define STATUS_MSGCNT 7    // 7 bytes per packet per receipt
#define TEAM_MSGCNT 19     // 19 bytes per packet tx'ed/ rx'ed
#define BROADCAST_MSGCNT 20    // 20 bytes per packet tx'ed/ rx'ed

//static void rxstoreBuf(uint8_t msg); // reads msg into char buf
//static void rxCpyData();  //sorts type of data and call sub-helpers to populate packet struct
//static void configStatus();  // sub-helper to send events of ES_DROPOUT / ES_REENTRY
//static void switchTeam(uint8_t cnt);    // stores rx'ed packet into team packet
//static void switchBroadcast(uint8_t cnt);   // stores rx'ed packet into broadcast packet

static uint8_t rxBuf[21];
static int RXTYPE = IS_TEAMDATA;
static uint8_t rxCount = 0;   //stores current byte num+1 receiving
static uint8_t MSGSIZE = 0;     // stores the num of bytes in each packet
static uint8_t whichTeam = NA; 
//extern SelfState_t SELF;
//extern bool OPAMP_ACTIVE;
//extern bool TVS_ACTIVE;
//extern bool ZENER_ACTIVE;

// Helper function to read into buffer
void rxstoreBuf(uint8_t msg) {
    if (((rxCount == 0) || (rxCount == 19) || (rxCount == 20)) && (msg == 0x7E)) {
        if (rxCount > 0) {  // if EOT of a packet
            rxCpyData();
        }
        memset(&rxBuf, 0, sizeof(rxBuf));
        rxBuf[0] = msg;
        rxCount = 1;  //set flag denoting start of valid receive
        return;
    } else {
        rxBuf[rxCount] = msg;
        rxCount++;
    }
}


// Helper function to sort type of data and call sub-helpers to populate packet struct
static void rxCpyData() {
    //printf("%u\n ", rxBuf[2]);
    uint8_t cnt = 0;
    if (rxBuf[0] == 0x7E) {
        if (rxBuf[2] == 15) {
            RXTYPE = IS_TEAMDATA;
            MSGSIZE = TEAM_MSGCNT;
            cnt++;
        } else if (rxBuf[2] == 16) {
            RXTYPE = IS_BROADCASTDATA;
            MSGSIZE = BROADCAST_MSGCNT;
            cnt++;
        }  
    }

    if (RXTYPE == IS_TEAMDATA) {
        for (uint8_t i = 1; i <= MSGSIZE; i++) {
            switchTeam(i);
        }
        ES_Event_t ReceiveEvent;
        ReceiveEvent.EventType = ES_RECEIVE;
        ReceiveEvent.EventParam = ISBUDDY;
        PostComms(ReceiveEvent);
    } else if (RXTYPE == IS_BROADCASTDATA) {
        for (uint8_t i = 1; i <= MSGSIZE; i++) {
            switchBroadcast(i);
        }
        ES_Event_t ReceiveEvent;
        ReceiveEvent.EventType = ES_RECEIVE;
        ReceiveEvent.EventParam = whichTeam;
        PostComms(ReceiveEvent);
    } 
} 

//
//static void configStatus() {
//    if (SELF == ZENER) {
//        if (rxBuf[5] == 2) {     // CCA failure
//            nakCntZENER++;
//        } else if ((rxBuf[5] == 1) && (TransmitNum == 5))  {  // NO ACK OPAMP
//            nakCntOPAMP++;
//            
//        } else if ((rxBuf[5] == 1) && (TransmitNum == 6))  {  // NO ACK TVS
//            nakCntTVS++;
//        } else if ((!rxBuf[5]) && (TransmitNum == 5)){  // ACK OPAMP
//            nakCntZENER = 0; 
//            nakCntOPAMP = 0;
//        } else if ((!rxBuf[5]) && (TransmitNum == 6)){  // ACK TVS
//            nakCntZENER = 0; 
//            nakCntTVS = 0;
//        }
//    } else if (SELF == OPAMP) {
//        if (rxBuf[5] == 2) {     // CCA failure
//            nakCntOPAMP++;
//        } else if ((rxBuf[5] == 1) && (TransmitNum == 1))  {  // NO ACK TVS
//            nakCntTVS++;
//        } else if ((rxBuf[5] == 1) && (TransmitNum == 2))  {  // NO ACK ZENER
//            nakCntZENER++;
//        } else if ((!rxBuf[5]) && (TransmitNum == 1)){  // ACK TVS
//            nakCntTVS = 0; 
//            nakCntOPAMP = 0;
//        } else if ((!rxBuf[5]) && (TransmitNum == 2)){  // ACK ZENER
//            nakCntZENER = 0; 
//            nakCntOPAMP = 0;
//        }
//    } else if (SELF == TVS) {
//        printf("b%u ", rxBuf[5]);
//        if (rxBuf[5] == 2) {     // CCA failure
//            printf("C ");
//            nakCntTVS++;
//        } else if ((rxBuf[5] == 1) && (TransmitNum == 3))  {  // NO ACK OPAMP
//            nakCntOPAMP++;
//        } else if ((rxBuf[5] == 1) && (TransmitNum == 4))  {  // NO ACK ZENER
//            nakCntZENER++;
//        } else if ((!rxBuf[5]) && (TransmitNum == 3)){  // ACK OPAMP
//            nakCntTVS = 0; 
//            nakCntOPAMP = 0;
//        } else if ((!rxBuf[5]) && (TransmitNum == 4)){  // ACK ZENER
//            nakCntTVS = 0; 
//            nakCntZENER = 0;
//        }
//    }
//      
//    switch(SELF) {
//        case ZENER:
//            if ((!ZENER_ACTIVE) && ((rxBuf[5]==0) ||(rxBuf[5]==1))) {
//                ackCntZENER ++;
//            } else if ((!OPAMP_ACTIVE) && (!rxBuf[5])) {
//                ackCntOPAMP ++;
//            } else if ((!TVS_ACTIVE) && (!rxBuf[5])) {
//                ackCntTVS ++;
//            }
//            break;
//        case TVS:
//            if ((!TVS_ACTIVE) && ((rxBuf[5]==0) ||(rxBuf[5]==1))) {
//                ackCntTVS ++;
//            } else if ((!OPAMP_ACTIVE) && (!rxBuf[5])) {
//                ackCntOPAMP ++;
//            } else if ((!ZENER_ACTIVE) && (!rxBuf[5])) {
//                ackCntZENER ++;
//            } 
//            break;    
//        case OPAMP:
//            if ((!TVS_ACTIVE) && (!rxBuf[5])) {
//                ackCntTVS ++;
//            } else if ((!OPAMP_ACTIVE) && ((rxBuf[5]==0) ||(rxBuf[5]==1))) {
//                ackCntOPAMP ++;
//            } else if ((!ZENER_ACTIVE) && (!rxBuf[5])) {
//                ackCntZENER ++;
//            } 
//            break; 
//        default:
//            break;
//    }
//    
//    switch(SELF) {
//        case ZENER: 
//            if (nakCntZENER == 5) {
//                // self dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_SELF_DROPOUT;
//                PostComms(StatusEvent);
//            } else if (nakCntTVS == 5) {
//                // tvs dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_DROPOUT;
//                StatusEvent.EventParam = 2;
//                PostComms(StatusEvent);
//            } else if (nakCntOPAMP == 5) {
//                // opamp dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_DROPOUT;
//                StatusEvent.EventParam = 1;
//                PostComms(StatusEvent);
//            } else if (ackCntZENER == 5) {
//                // self back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_SELF_REENTRY;
//                StatusEvent.EventParam = 3;
//                PostComms(StatusEvent);
//            } else if (ackCntOPAMP == 5) {
//                // opamp back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_REENTRY;
//                StatusEvent.EventParam = 1;
//                PostComms(StatusEvent);
//            } else if (ackCntTVS == 5) {
//                // TVS back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_REENTRY;
//                StatusEvent.EventParam = 2;
//                PostComms(StatusEvent);
//            } 
//        break;
//        
//        case TVS: 
//            if (nakCntTVS == 5) {
//                // self dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_SELF_DROPOUT;
//                PostComms(StatusEvent);
//                printf("drop\n");
//            } else if (nakCntZENER == 5) {
//                // zener dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_DROPOUT;
//                StatusEvent.EventParam = 3;
//                PostComms(StatusEvent);
//            } else if (nakCntOPAMP == 5) {
//                // opamp dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_DROPOUT;
//                StatusEvent.EventParam = 1;
//                PostComms(StatusEvent);
//            } else if (ackCntTVS == 5) {
//                // self back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_SELF_REENTRY;
//                PostComms(StatusEvent);
//            } else if (ackCntOPAMP == 5) {
//                // opamp back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_REENTRY;
//                StatusEvent.EventParam = 1;
//                PostComms(StatusEvent);
//            } else if (ackCntZENER == 5) {
//                // zener back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_REENTRY;
//                StatusEvent.EventParam = 3;
//                PostComms(StatusEvent);
//            } 
//        break;
//        
//        case OPAMP: 
//            if (nakCntOPAMP == 5) {
//                // self dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_SELF_DROPOUT;
//                PostComms(StatusEvent);
//            } else if (nakCntTVS == 5) {
//                // tvs dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_DROPOUT;
//                StatusEvent.EventParam = 2;
//                PostComms(StatusEvent);
//            } else if (nakCntZENER == 5) {
//                // zener dropout
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_DROPOUT;
//                StatusEvent.EventParam = 3;
//                PostComms(StatusEvent);
//            } else if (ackCntOPAMP == 5) {
//                // self back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_SELF_REENTRY;
//                PostComms(StatusEvent);
//            } else if (ackCntZENER == 5) {
//                // zener back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_REENTRY;
//                StatusEvent.EventParam = 3;
//                PostComms(StatusEvent);
//            } else if (ackCntTVS == 5) {
//                // TVS back
//                ES_Event_t StatusEvent;
//                StatusEvent.EventType = ES_REENTRY;
//                StatusEvent.EventParam = 2;
//                PostComms(StatusEvent);
//            } 
//        break;
//        
//        default: 
//            break;
//    }
//    
//}

static void switchTeam(uint8_t cnt) {
    struct TeamRxPacket tempRxPacket;
    //printf("%u ", rxBuf[10]);
    switch(cnt) {
        
        case 1: 
            tempRxPacket.startDelim = rxBuf[cnt-1]; 
            break;
        case 2:
            tempRxPacket.length = rxBuf[cnt-1];  // MSB
            break;  
        case 3:
            tempRxPacket.length = ((tempRxPacket.length << 8) | rxBuf[cnt-1]); // blend LSB w MSB
            break;
        case 4:
            tempRxPacket.api = rxBuf[cnt-1];
            break;     
        case 5:
            tempRxPacket.sourceAddr = rxBuf[cnt-1];
            break; 
        case 6:
            tempRxPacket.sourceAddr = ((tempRxPacket.sourceAddr << 8) | rxBuf[cnt-1]);
            break;
        // Case 7 & 8, rssi & options are ignored in rx
        case 9:   // TODO: Broadcast reading logic to be implemented later
            tempRxPacket.LocalRxPacket.status = rxBuf[cnt-1];
            //printf("B)%x ",rxBuf[cnt-1]);
            break; 
        case 10:
            tempRxPacket.LocalRxPacket.hull = rxBuf[cnt-1];
            break; 
        case 11:
            tempRxPacket.LocalRxPacket.beam_dx_lsb = rxBuf[cnt-1];
            break; 
        case 12:
            tempRxPacket.LocalRxPacket.beam_dx_msb = rxBuf[cnt-1];
            break; 
        case 13:
            tempRxPacket.LocalRxPacket.beam_dy_lsb = rxBuf[cnt-1];
            break; 
        case 14:
            tempRxPacket.LocalRxPacket.beam_dy_msb = rxBuf[cnt-1];
            break; 
        case 15:
            tempRxPacket.LocalRxPacket.weaponPower = rxBuf[cnt-1];
            break; 
        case 16:
            tempRxPacket.LocalRxPacket.navPower = rxBuf[cnt-1];
            break; 
        case 17:
            tempRxPacket.LocalRxPacket.weaponCap = rxBuf[cnt-1];
            break;
        case 18:
            tempRxPacket.LocalRxPacket.shield = rxBuf[cnt-1];
            break;
        case 19:
            tempRxPacket.checkSum = rxBuf[cnt-1];
            memcpy(&realTeamRxPacket, &tempRxPacket, sizeof(struct TeamRxPacket));
            break;  
        default: 
            break;
    }
}


static void switchBroadcast(uint8_t cnt) {
    struct BroadcastRxPacket tempRxPacket;
    switch(cnt) {
        case 1: 
            tempRxPacket.startDelim = rxBuf[cnt-1]; 
            break;
        case 2:
            tempRxPacket.length = rxBuf[cnt-1];  // MSB
            break;  
        case 3:
            tempRxPacket.length = ((tempRxPacket.length << 8) | rxBuf[cnt-1]); // blend LSB w MSB
            break;
        case 4:
            tempRxPacket.api = rxBuf[cnt-1];
            break;     
        case 5:
            tempRxPacket.sourceAddr = rxBuf[cnt-1];
            break; 
        case 6:
            tempRxPacket.sourceAddr = ((tempRxPacket.sourceAddr << 8) | rxBuf[cnt-1]);
            break;
        // Case 7 & 8, rssi & options are ignored in rx
        case 9:   // TODO: Broadcast reading logic to be implemented later
            tempRxPacket.GlobalRxPacket.status = rxBuf[cnt-1];
            break; 
        case 10:
            tempRxPacket.GlobalRxPacket.hull = rxBuf[cnt-1];
            break; 
        case 11:
            tempRxPacket.GlobalRxPacket.ship_x_lsb = rxBuf[cnt-1];
            break; 
        case 12:
            tempRxPacket.GlobalRxPacket.ship_x_msb = rxBuf[cnt-1];
            break; 
        case 13:
            tempRxPacket.GlobalRxPacket.ship_y_lsb = rxBuf[cnt-1];
            break; 
        case 14:
            tempRxPacket.GlobalRxPacket.ship_y_msb = rxBuf[cnt-1];
            break; 
        case 15:
            tempRxPacket.GlobalRxPacket.beam_dx_lsb = rxBuf[cnt-1];
            break; 
        case 16:
            tempRxPacket.GlobalRxPacket.beam_dx_msb = rxBuf[cnt-1];
            break; 
        case 17:
            tempRxPacket.GlobalRxPacket.beam_dy_lsb = rxBuf[cnt-1];
            break;
        case 18:
            tempRxPacket.GlobalRxPacket.beam_dy_msb = rxBuf[cnt-1];
            break;
        case 19:
            tempRxPacket.GlobalRxPacket.shield = rxBuf[cnt-1];
            break;
        case 20:
            tempRxPacket.checkSum = rxBuf[cnt-1];
            switch (tempRxPacket.GlobalRxPacket.status >> 5) {
                case 0:
                    memcpy(&realB0RxPacket, &tempRxPacket, sizeof(struct BroadcastRxPacket));
                    whichTeam = ISTEAM0;
                    break;
                case 1:
                    memcpy(&realB1RxPacket, &tempRxPacket, sizeof(struct BroadcastRxPacket));
                    whichTeam = ISTEAM1;
                    break;
                case 2:
                    memcpy(&realB2RxPacket, &tempRxPacket, sizeof(struct BroadcastRxPacket));
                    whichTeam = ISTEAM2;
                    break;
                case 3:
                    memcpy(&realB3RxPacket, &tempRxPacket, sizeof(struct BroadcastRxPacket));
                    whichTeam = ISTEAM3;
                    break;
                case 4:
                    memcpy(&realB4RxPacket, &tempRxPacket, sizeof(struct BroadcastRxPacket));
                    whichTeam = ISTEAM4;
                    break;
                case 5:
                    memcpy(&realB5RxPacket, &tempRxPacket, sizeof(struct BroadcastRxPacket));
                    whichTeam = ISTEAM5;
                    break;
                case 6:
                    memcpy(&realB6RxPacket, &tempRxPacket, sizeof(struct BroadcastRxPacket));
                    whichTeam = ISTEAM6;
                    break;
                default: 
                    break;
            }
            break;  
        default: 
            break;
    }
}
