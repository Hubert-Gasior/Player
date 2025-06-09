#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
///////////////////////////////
#include <math.h>
#include "mniam_update.h"
#include "mniam_move.h"
///////////////////////////////

#include "amcom.h"
#include "amcom_packets.h"

typedef enum {
    PLAYER = 0,
    TRANSISTOR = 1,
    SPARK = 2,
    GLUE = 3,
    DEFAULT = 4

}ObjectType;

typedef enum {
    SPARK_DIAMETER = 25,
    GLUE_DIAMETER = 200
}ObjectDiameter;

//static AMCOM_ObjectState SparkList[5] = {{5, 0, 0, 0.0f, 0.0f}};

static AMCOM_ObjectState TransistorList[50];
static AMCOM_ObjectState SparkList[5];
static AMCOM_ObjectState GlueList[5];
static AMCOM_ObjectState PlayerList[8];
static const size_t TransistorListSize = sizeof(TransistorList)/sizeof(AMCOM_ObjectState);
static const size_t SparkListSize = sizeof(SparkList)/sizeof(AMCOM_ObjectState);
static const size_t GlueListSize = sizeof(GlueList)/sizeof(AMCOM_ObjectState);
static const size_t PlayerListSize = sizeof(PlayerList)/sizeof(AMCOM_ObjectState);
static uint8_t PlayerNumber = 0;
static position Transistor;
static AMCOM_ObjectState Player;

void amPacketHandler(const AMCOM_Packet* packet, void* userContext) {
    uint8_t buf[AMCOM_MAX_PACKET_SIZE];              // buffer used to serialize outgoing packets
    size_t toSend = 0;                               // size of the outgoing packet
    SOCKET ConnectSocket  = *((SOCKET*)userContext); // socket used for communication with the server

    switch (packet->header.type) {
    case AMCOM_NO_PACKET:
        break;
    case AMCOM_IDENTIFY_REQUEST:
        printf("Got IDENTIFY.request. Responding with IDENTIFY.response\n");
        PlayerNumber = packet->payload[0];
        AMCOM_IdentifyResponsePayload identifyResponse;
        sprintf(identifyResponse.playerName, "mniAM player");
        toSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &identifyResponse, sizeof(identifyResponse), buf);
        break;
    case AMCOM_NEW_GAME_REQUEST:
        AMCOM_NewGameResponsePayload NewGameResponse;
        sprintf(NewGameResponse.helloMessage, "LET'S GOOO");
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
        

        for(size_t i = 0; i + structSize <= payloadLength && i < 192; i += structSize){
            AMCOM_ObjectState CurrentReceivedObject = {0};
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
            printf("%s\n", "OBIEKT!!!!!!!!!!!");
            printf("Type: %d No: %d hp: %d x: %f y: %f\n",CurrentReceivedObject.objectType, CurrentReceivedObject.objectNo, CurrentReceivedObject.hp, CurrentReceivedObject.x, CurrentReceivedObject.y);
            printf("%s\n", "OBIEKT!!!!!!!!!!!");

            //if(CurrentReceivedObject.objectType == 0 && CurrentReceivedObject.objectNo == 0 && CurrentReceivedObject.hp == 0){
            //    continue;
            //}

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
        printf("%s\n", "TRANSISTORS");
        for(size_t i = 0; i < TransistorListSize; i++){
            printf("Type: %d No: %d hp: %d x: %f y: %f\n",TransistorList[i].objectType, TransistorList[i].objectNo, TransistorList[i].hp, TransistorList[i].x, TransistorList[i].y);
        }
        printf("%s\n", "SPARKS");
        for(size_t i = 0; i < SparkListSize; i++){
            printf("Type: %d No: %d hp: %d x: %f y: %f\n",SparkList[i].objectType, SparkList[i].objectNo, SparkList[i].hp, SparkList[i].x, SparkList[i].y);
        }
        printf("%s\n", "GLUE");
        for(size_t i = 0; i < GlueListSize; i++){
            printf("Type: %d No: %d hp: %d x: %f y: %f\n",GlueList[i].objectType, GlueList[i].objectNo, GlueList[i].hp, GlueList[i].x, GlueList[i].y);
        }
        printf("%s\n", "PLAYERS");
        for(size_t i = 0 ; i < PlayerListSize; i++){
            printf("Type: %d No: %d hp: %d x: %f y: %f\n",PlayerList[i].objectType, PlayerList[i].objectNo, PlayerList[i].hp, PlayerList[i].x, PlayerList[i].y);
        }
        break;
    case AMCOM_MOVE_REQUEST:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float angle = 0;
    position PlayerPosition;

    Player = FindPosition(PlayerList, &PlayerPosition, PlayerNumber, PLAYER, PlayerListSize);
    printf("Player\nX: %f Y:%f\n", PlayerPosition.x, PlayerPosition.y);
    //FindClosestObjectOfType(TransistorList, &PlayerPosition, &Transistor, TRANSISTOR, TransistorListSize, 0);
    if(!IsAlive(TransistorList, &Transistor, TransistorListSize)){
        CalculateAlternativePaths(TransistorList, &PlayerPosition, &Transistor, TransistorListSize, GlueList, &Player, GlueListSize);
    }
    printf("Transistor\nX: %f Y:%f\n", Transistor.x, Transistor.y);

    //angle = (1.0f + obstacle(SparkList, &PlayerPosition, &transistor, &Player, 25, SparkListSize)) * CalculateAngle(&PlayerPosition, &transistor);
    angle = CalculateAngle(&PlayerPosition, &Transistor);
    printf("angle: %f\n", angle);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        AMCOM_MoveResponsePayload MoveResponse;
        MoveResponse.angle = angle;
        toSend = AMCOM_Serialize(AMCOM_MOVE_RESPONSE, &MoveResponse, sizeof(MoveResponse), buf);
        break;
    case AMCOM_GAME_OVER_REQUEST:
        AMCOM_GameOverResponsePayload GameOverResponse;
        sprintf(GameOverResponse.endMessage, "Goodbye");
        toSend = AMCOM_Serialize(AMCOM_GAME_OVER_RESPONSE, &GameOverResponse, sizeof(GameOverResponse), buf);
        break;
    }

	// if there is something to send back - do it
	if (toSend > 0) {
		int bytesSent = send(ConnectSocket, (const char*)buf, toSend, 0 );
		if (bytesSent == SOCKET_ERROR) {
			printf("Socket send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			return;
		}
	}
}


#define GAME_SERVER "localhost"
#define GAME_SERVER_PORT "2001"

int main(int argc, char **argv) {
    printf("This is mniAM player. Let's eat some transistors! \n");

    WSADATA wsaData;
    int iResult;

    // Initialize Winsock library (windows sockets)
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Prepare temporary data
    SOCKET ConnectSocket  = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;
    int iSendResult;
    char recvbuf[512];
    int recvbuflen = sizeof(recvbuf);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the game server address and port
    iResult = getaddrinfo(GAME_SERVER, GAME_SERVER_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    printf("Connecting to game server...\n");
    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    // Free some used resources
    freeaddrinfo(result);

    // Check if we connected to the game server
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to the game server!\n");
        WSACleanup();
        return 1;
    } else {
        printf("Connected to game server\n");
    }

    AMCOM_Receiver amReceiver;
    AMCOM_InitReceiver(&amReceiver, amPacketHandler, &ConnectSocket);

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ) {
            AMCOM_Deserialize(&amReceiver, recvbuf, iResult);
        } else if ( iResult == 0 ) {
            printf("Connection closed\n");
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }

    } while( iResult > 0 );

    // No longer need the socket
    closesocket(ConnectSocket);
    // Clean up
    WSACleanup();

    return 0;
}