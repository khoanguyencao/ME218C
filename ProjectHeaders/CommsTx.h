// Function Prototypes
// Team Transmit
struct TeamTxPacket teamCreatePacket(struct TeamData ToSend, uint16_t destAddr);
uint8_t teamChecksum(struct TeamTxPacket PacketToSend);
void teamUARTWrite(struct TeamTxPacket PacketToSend);
uint8_t teamMessage(uint8_t packetNumber, struct TeamTxPacket PacketToSend);
// Broadcast Transmit
struct BroadcastTxPacket broadcastCreatePacket(struct BroadcastData ToSend);
uint8_t broadcastChecksum(struct BroadcastTxPacket PacketToSend);
void broadcastUARTWrite(struct BroadcastTxPacket PacketToSend);
uint8_t broadcastMessage(uint8_t packetNumber, struct BroadcastTxPacket PacketToSend);