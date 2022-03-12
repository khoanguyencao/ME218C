#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PackDataHandler.h"
#include "Comms.h"
#include "OPAMP.h"
#include "CommsTx.h"

struct TeamTxPacket teamCreatePacket(struct TeamData ToSend, uint16_t destAddr){
    struct TeamTxPacket PacketToSend;
    PacketToSend.startDelim = 0x7E;
    PacketToSend.length = 15;
    PacketToSend.api = 0x01;
    PacketToSend.frameID = 5;
    PacketToSend.destAddr = destAddr;
    PacketToSend.options = 0;
    PacketToSend.LocalTxPacket = ToSend;
    PacketToSend.checkSum = teamChecksum(PacketToSend);
    return PacketToSend;
}

struct BroadcastTxPacket broadcastCreatePacket(struct BroadcastData ToSend){
    struct BroadcastTxPacket PacketToSend;
    PacketToSend.startDelim = 0x7E;
    PacketToSend.length = 16;
    PacketToSend.api = 0x01;
    PacketToSend.frameID = 5;
    PacketToSend.destAddr = 0xFFFF;
    PacketToSend.options = 0;
    PacketToSend.GlobalTxPacket = ToSend;
    PacketToSend.checkSum = broadcastChecksum(PacketToSend);
    return PacketToSend;
}

void teamUARTWrite(struct TeamTxPacket PacketToSend){
    uint8_t packetLength = PacketToSend.length + 4;
    for (uint8_t i = 0; i < packetLength; i++){
        // wait for the register to empty
        while(U2STAbits.UTXBF)
        {}
        uint8_t msg = teamMessage(i, PacketToSend);
        U2TXREG = msg;
    }
}

void broadcastUARTWrite(struct BroadcastTxPacket PacketToSend){
    uint8_t packetLength = PacketToSend.length + 4;
    for (uint8_t i = 0; i < packetLength; i++){
        // wait for the register to empty
        while(U2STAbits.UTXBF)
        {}
        uint8_t msg = broadcastMessage(i, PacketToSend);
        U2TXREG = msg;
    }
}

uint8_t teamMessage(uint8_t packetNumber, struct TeamTxPacket PacketToSend){
    uint8_t msg;
    switch(packetNumber)
        {
            case 0:
                msg = PacketToSend.startDelim;
            break;
            case 1:
                msg = 0;
            break;    
            case 2:
                msg = PacketToSend.length;
            break;
            case 3:
                msg = PacketToSend.api;
            break;  
            case 4:
                msg = PacketToSend.frameID;
            break;
            case 5:
                msg = PacketToSend.destAddr >> 8;
            break;
            case 6:
                msg = PacketToSend.destAddr & 255;
            break;
            case 7:
                msg = PacketToSend.options;
            break;
            case 8:
                msg = PacketToSend.LocalTxPacket.status;
            break;
            case 9:
                msg = PacketToSend.LocalTxPacket.hull;
            break;  
            case 10:
                msg = PacketToSend.LocalTxPacket.beam_dx_lsb;
            break;
            case 11:
                msg = PacketToSend.LocalTxPacket.beam_dx_msb;
            break;    
            case 12:
                msg = PacketToSend.LocalTxPacket.beam_dy_lsb;
            break;
            case 13:
                msg = PacketToSend.LocalTxPacket.beam_dy_msb;
            break;  
            case 14:
                msg = PacketToSend.LocalTxPacket.weaponPower;
            break;
            case 15:
                msg = PacketToSend.LocalTxPacket.navPower;
            break;
            case 16:
                msg = PacketToSend.LocalTxPacket.weaponCap;
            break;
            case 17:
                msg = PacketToSend.LocalTxPacket.shield;
            break;
            case 18:
                msg = PacketToSend.checkSum;
            break;
            default:
            {}
            break;
        }
    return msg;
}

uint8_t broadcastMessage(uint8_t packetNumber, struct BroadcastTxPacket PacketToSend){
    uint8_t msg;
    switch(packetNumber)
        {
            case 0:
                msg = PacketToSend.startDelim;
            break;
            case 1:
                msg = 0;
            break;    
            case 2:
                msg = PacketToSend.length;
            break;
            case 3:
                msg = PacketToSend.api;
            break;  
            case 4:
                msg = PacketToSend.frameID;
            break;
            case 5:
                msg = PacketToSend.destAddr >> 8;
            break;
            case 6:
                msg = PacketToSend.destAddr & 255;
            break;
            case 7:
                msg = PacketToSend.options;
            break;
            case 8:
                msg = PacketToSend.GlobalTxPacket.status;
            break;
            case 9:
                msg = PacketToSend.GlobalTxPacket.hull;
            break;  
            case 10:
                msg = PacketToSend.GlobalTxPacket.ship_x_lsb;
            break;
            case 11:
                msg = PacketToSend.GlobalTxPacket.ship_x_msb;
            break;    
            case 12:
                msg = PacketToSend.GlobalTxPacket.ship_y_lsb;
            break;
            case 13:
                msg = PacketToSend.GlobalTxPacket.ship_y_msb;
            break;  
            case 14:
                msg = PacketToSend.GlobalTxPacket.beam_dx_lsb;
            break;
            case 15:
                msg = PacketToSend.GlobalTxPacket.beam_dx_msb;
            break;    
            case 16:
                msg = PacketToSend.GlobalTxPacket.beam_dy_lsb;
            break;
            case 17:
                msg = PacketToSend.GlobalTxPacket.beam_dy_msb;
            break;  
            case 18:
                msg = PacketToSend.GlobalTxPacket.shield;
            break;
            case 19:
                msg = PacketToSend.checkSum;
            break;
            default:
            {}
            break;
        }
    return msg;
}

uint8_t teamChecksum(struct TeamTxPacket PacketToSend){
    uint8_t checksum = 0;
    checksum = PacketToSend.api;
    checksum += PacketToSend.frameID;
    checksum += PacketToSend.destAddr >> 8;
    checksum += PacketToSend.destAddr & 255;
    checksum += PacketToSend.options;
    checksum += PacketToSend.LocalTxPacket.status;
    checksum += PacketToSend.LocalTxPacket.hull;
    checksum += (uint8_t)PacketToSend.LocalTxPacket.beam_dx_lsb;
    checksum += (uint8_t)PacketToSend.LocalTxPacket.beam_dx_msb;
    checksum += (uint8_t)PacketToSend.LocalTxPacket.beam_dy_lsb;
    checksum += (uint8_t)PacketToSend.LocalTxPacket.beam_dy_msb;
    checksum += PacketToSend.LocalTxPacket.weaponPower;
    checksum += PacketToSend.LocalTxPacket.navPower;
    checksum += PacketToSend.LocalTxPacket.weaponCap;
    checksum += PacketToSend.LocalTxPacket.shield;
    checksum = 0xFF - checksum; 
    return checksum;
}

uint8_t broadcastChecksum(struct BroadcastTxPacket PacketToSend){
    uint8_t checksum = 0;
    checksum = PacketToSend.api;
    checksum += PacketToSend.frameID;
    checksum += PacketToSend.destAddr >> 8;
    checksum += PacketToSend.destAddr & 255;
    checksum += PacketToSend.options;
    checksum += PacketToSend.GlobalTxPacket.status;
    checksum += PacketToSend.GlobalTxPacket.hull;
    checksum += PacketToSend.GlobalTxPacket.ship_x_lsb;
    checksum += PacketToSend.GlobalTxPacket.ship_x_msb;
    checksum += PacketToSend.GlobalTxPacket.ship_y_lsb;
    checksum += PacketToSend.GlobalTxPacket.ship_y_msb;
    checksum += (uint8_t)PacketToSend.GlobalTxPacket.beam_dx_lsb;
    checksum += (uint8_t)PacketToSend.GlobalTxPacket.beam_dx_msb;
    checksum += (uint8_t)PacketToSend.GlobalTxPacket.beam_dy_lsb;
    checksum += (uint8_t)PacketToSend.GlobalTxPacket.beam_dy_msb;
    checksum += PacketToSend.GlobalTxPacket.shield;
    checksum = 0xFF - checksum; 
    return checksum;
}