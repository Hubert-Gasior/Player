#ifndef MNIAM_MOVE_H_
#define MNIAM_MOVE_H

#include "amcom_packets.h"

typedef struct {
    float x;
    float y;
}position;

float CalculatePath(const position* object_1, const position* object_2);

AMCOM_ObjectState FindPosition(const AMCOM_ObjectState* List, position* Object, const uint8_t PlayerNo, uint8_t ObjectType, size_t size);

AMCOM_ObjectState FindClosestObjectOfType(const AMCOM_ObjectState* List, const position* StartPositon, position* Object, uint8_t ObjectType, size_t size, uint8_t SecondClosestObject);

float CalculateAngle(const position* StartPosition, const position* EndPosition);

float obstacle(const AMCOM_ObjectState* ObstacleList, const position* StartPosition , const position* EndPosition, const AMCOM_ObjectState* Player, uint8_t diameter, size_t size);

void CalculateAlternativePaths(const AMCOM_ObjectState* List, const position* StartPosition, position* EndPosition, size_t size, const AMCOM_ObjectState* GlueList, const AMCOM_ObjectState* Player, size_t GlueListSize);

uint8_t IsAlive(const AMCOM_ObjectState* List, const position* object, size_t size);

#endif