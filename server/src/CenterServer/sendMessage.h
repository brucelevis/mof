#pragma once

class INetPacket;
struct skynet_message;

void sendNetPacket(int session, INetPacket* packet);
void sendNetData(int session, void* data, int len);
void sendMessageToCenter(skynet_message* msg);
void sendMessageToGate(skynet_message* msg);
void sendMessageToLog(skynet_message* msg);