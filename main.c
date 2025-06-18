#include "cmsis_os2.h"
#include "game.h"
#include "amcom.h"
#include "amcom_packets.h"

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <FreeRTOS_IP.h>
#include <FreeRTOS_Sockets.h>

#include "mniam_update.h"
#include "mniam_move.h"

static AMCOM_ObjectState TransistorList[50];
static AMCOM_ObjectState SparkList[4];
static AMCOM_ObjectState GlueList[2];
static AMCOM_ObjectState PlayerList[8];
static const size_t TransistorListSize = sizeof(TransistorList)/sizeof(AMCOM_ObjectState);
static const size_t SparkListSize = sizeof(SparkList)/sizeof(AMCOM_ObjectState);
static const size_t GlueListSize = sizeof(GlueList)/sizeof(AMCOM_ObjectState);
static const size_t PlayerListSize = sizeof(PlayerList)/sizeof(AMCOM_ObjectState);
static uint8_t PlayerNumber;
static position Transistor;
static AMCOM_ObjectState Player;


/**
 * This function will be called each time a valid AMCOM packet is received
 */
void amcomPacketHandler(const AMCOM_Packet* packet, void* userContext) {
	static uint8_t buf[AMCOM_MAX_PACKET_SIZE];  // buffer used to serialize outgoing packets
	size_t toSend = 0;                          // size of the outgoing packet
	Socket_t socket = (Socket_t)userContext;    // socket used for communication with the server

	switch (packet->header.type) {
	    case AMCOM_NO_PACKET:
	        break;
	    case AMCOM_IDENTIFY_REQUEST:
	        printf("Got IDENTIFY.request. Responding with IDENTIFY.response\n");
	        AMCOM_IdentifyResponsePayload identifyResponse;
	        sprintf(identifyResponse.playerName, "Mariano Italiano");
	        toSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &identifyResponse, sizeof(identifyResponse), buf);
	        break;
	    case AMCOM_NEW_GAME_REQUEST:
	        AMCOM_NewGameResponsePayload NewGameResponse;
	        PlayerNumber = packet->payload[0];
	        sprintf(NewGameResponse.helloMessage, "Let's GOOOOOOOO");
	        toSend = AMCOM_Serialize(AMCOM_NEW_GAME_RESPONSE, &NewGameResponse, sizeof(NewGameResponse), buf);
	        break;
	    case AMCOM_OBJECT_UPDATE_REQUEST:
	        size_t x_index;
	        size_t y_index;
	        float Received_x;
	        float Received_y;
	        uint8_t RawReceived_x[4];
	        uint8_t RawReceived_y[4];
	        size_t structSize = sizeof(AMCOM_ObjectState);
	        size_t payloadLength = packet->header.length;
	        ObjectType ReceivedObject = DEFAULT;
	        AMCOM_ObjectState CurrentReceivedObject = {0};


	        for(size_t i = 0; i + structSize <= payloadLength && i < 192; i += structSize){
	            x_index = 0;
	            y_index = 0;
	            Received_x = 0;
	            Received_y = 0;

	            for(size_t j = 0; j < structSize; j++) {
	                if( j == 0) {
	                    CurrentReceivedObject.objectType = packet->payload[i+j];
	                } else if (j == 1) {
	                    CurrentReceivedObject.objectNo = packet->payload[i+j];
	                } else if (j > 1 && j <= 3) {
	                    CurrentReceivedObject.hp <<= 8;
	                    CurrentReceivedObject.hp |= packet->payload[i+j];
	                } else if (j > 3 && j <= 7) {
	                    RawReceived_x[x_index] = packet->payload[i+j];
	                    x_index++;
	                } else {
	                    RawReceived_y[y_index] = packet->payload[i+j];
	                    y_index++;
	                }
	            }

	            memcpy(&Received_x, RawReceived_x, sizeof(RawReceived_x));
	            memcpy(&Received_y, RawReceived_y, sizeof(RawReceived_y));
	            CurrentReceivedObject.x = Received_x;
	            CurrentReceivedObject.y = Received_y;

	            ReceivedObject = CurrentReceivedObject.objectType;

	            switch (ReceivedObject){
	                case PLAYER:
	                    SaveObject(PlayerList, &CurrentReceivedObject, PlayerListSize);
	                    break;
	                case TRANSISTOR:
	                    SaveObject(TransistorList, &CurrentReceivedObject, TransistorListSize);
	                    break;
	                case SPARK:
	                    SaveObject(SparkList, &CurrentReceivedObject, SparkListSize);
	                    break;
	                case GLUE:
	                    SaveObject(GlueList, &CurrentReceivedObject, GlueListSize);
	                    break;
	                default:
	                    ReceivedObject = DEFAULT;
	                    break;
	            }

	        }

	        break;
	    case AMCOM_MOVE_REQUEST:
	        float angle = 0.0;
	        float obstacle = 0.0;
	        position PlayerPosition;
	        position TargetPosition;
	        uint8_t NoMoreFood = 0;


	        Player = FindPosition(PlayerList, &PlayerPosition, PlayerNumber, PLAYER, PlayerListSize);
	        if(!IsAlive(TransistorList, &Transistor, TransistorListSize)){
	            CalculateAlternativePaths(TransistorList, &PlayerPosition, &Transistor, TransistorListSize);
	        }
	        angle =  CalculateAngle(&PlayerPosition, &Transistor);

	        obstacle = AvoidObstacle(SparkList, &Player, SPARK_DIAMETER, SparkListSize);

	        if(obstacle > 0.0){
	           angle += obstacle * 3.14/2;
	        }

	        for(size_t i = 0; i < TransistorListSize; i++){
	            if(0 != TransistorList[i].hp){
	                NoMoreFood = 0;
	                break;
	            }
	            NoMoreFood = 1;
	        }

	        if(NoMoreFood){
	            FindBestTarget(PlayerList, &Player, &TargetPosition, PlayerListSize);
	            angle = CalculateAngle(&PlayerPosition, &TargetPosition);
	        }

	        AMCOM_MoveResponsePayload MoveResponse;
	        MoveResponse.angle = angle;
	        toSend = AMCOM_Serialize(AMCOM_MOVE_RESPONSE, &MoveResponse, sizeof(MoveResponse), buf);
	        break;
	    case AMCOM_GAME_OVER_REQUEST:
	        AMCOM_GameOverResponsePayload GameOverResponse;
	        sprintf(GameOverResponse.endMessage, "Wszystko by sie udalo gdyby nie te wscibskie dzieciaki");
	        toSend = AMCOM_Serialize(AMCOM_GAME_OVER_RESPONSE, &GameOverResponse, sizeof(GameOverResponse), buf);
	        break;
	    }

	// if there is something to send back - do it
	if (toSend > 0) {
		FreeRTOS_send(socket, buf, toSend, 0);
	}
}
