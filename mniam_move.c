#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <math.h>
#include "mniam_move.h"

float CalculatePath(const position* object_1, const position* object_2) {
	return ((object_2->x - object_1->x)*(object_2->x - object_1->x)) + ((object_2->y - object_1->y)*(object_2->y - object_1->y));
}

uint8_t IsAlive(const AMCOM_ObjectState* ObjectList, const position* object, size_t size){
    for(size_t i = 0; i < size; i++){
        if(ObjectList[i].x == object->x && ObjectList[i].y == object->y && ObjectList[i].hp != 0){
            return 1;
        }
    }
    return 0;
}

AMCOM_ObjectState FindPosition(const AMCOM_ObjectState* ObjectList, position* Object, uint8_t ObjectNumber, uint8_t ObjectType, size_t size) {
	AMCOM_ObjectState FoundObject;

    for (size_t i = 0; i < size; i++) {
		if(ObjectList[i].objectType == ObjectType && ObjectList[i].objectNo == ObjectNumber) {
            FoundObject = ObjectList[i]; 
			Object->x = ObjectList[i].x;
			Object->y = ObjectList[i].y;
			break;
		}
	}
	return FoundObject;
}

AMCOM_ObjectState FindClosestObjectOfType(const AMCOM_ObjectState* ObjectList, const position* StartPositon, position* Object, uint8_t ObjectType, size_t size, uint8_t SecondClosestObject) {
	AMCOM_ObjectState ClosestObject;
    float path = 0;
	float shortest_path = 0;
    float SecondPath = 0;
    position candidate;

	for (size_t i = 0; i < size; i++) {
		if(ObjectList[i].objectType == ObjectType) {
            candidate.x = ObjectList[i].x;
            candidate.y = ObjectList[i].y;
			path = CalculatePath(StartPositon, &candidate);
			if(shortest_path > path || shortest_path == 0) {
				Object->x = ObjectList[i].x;
				Object->y = ObjectList[i].y;
                ClosestObject = ObjectList[i];
				shortest_path = path;
			}
        }
	}

    if(SecondClosestObject == 1){
        for (size_t i = 0; i < size; i++) {
            if(ObjectList[i].objectType == ObjectType) {
                candidate.x = ObjectList[i].x;
                candidate.y = ObjectList[i].y;
                path = CalculatePath(StartPositon, &candidate);
                if(shortest_path != path && (SecondPath > path || SecondPath == 0)) {
                    Object->x = ObjectList[i].x;
                    Object->y = ObjectList[i].y;
                    ClosestObject = ObjectList[i];
                    SecondPath = path;
                }
            }
        }  
    }

	return ClosestObject;
}

float CalculateAngle(const position* StartPosition, const position* EndPosition){
    float angle = 0;
    float deltaX = EndPosition->x - StartPosition->x;
    float deltaY = EndPosition->y - StartPosition->y;
    angle = atan2(deltaY, deltaX);
    if(angle < 0){
        angle += (2 * M_PI);
    }
    return angle;
}

float obstacle(const AMCOM_ObjectState* ObstacleList, const position* StartPosition , const position* EndPosition, const AMCOM_ObjectState* Player, uint8_t diameter, size_t size){
    float Path = 0;
    float PlayerRadius;
    float ObstacleRadius;
    position StartEnd;
    position StartCenter;
    float StartEndSquaredLength = 0;
    float t = 0;
    position ClosestPoint;
    float MinDistanceToCenter;
    position CurrentObstacle;
    float CombinedRadius;
    float overlap;
    float chord;

    Path = CalculatePath(StartPosition, EndPosition);
    PlayerRadius = (25.0 + Player->hp)/2.0;
    ObstacleRadius = diameter/2.0;
    StartEnd.x = EndPosition->x - StartPosition->x;
    StartEnd.y = EndPosition->y - StartPosition->y;
    CombinedRadius = ObstacleRadius + PlayerRadius;
    

    for(size_t i = 0; i < size; i++){
        CurrentObstacle.x = ObstacleList[i].x;
        CurrentObstacle.y = ObstacleList[i].y;
        StartCenter.x = ObstacleList[i].x - StartPosition->x;
        StartCenter.y = ObstacleList[i].y - StartPosition->y;

        StartEndSquaredLength = (StartEnd.x * StartEnd.x) + (StartEnd.y * StartEnd.y);
        t = ((StartEnd.x*StartCenter.x) + (StartEnd.y * StartCenter.y))/StartEndSquaredLength;

        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;

        ClosestPoint.x = StartPosition->x + StartEnd.x * t;
        ClosestPoint.x = StartPosition->y + StartEnd.y * t;

        MinDistanceToCenter = CalculatePath(&CurrentObstacle, &ClosestPoint);

        if(MinDistanceToCenter < CombinedRadius){
            chord = 2.0 * sqrtf(CombinedRadius * CombinedRadius - MinDistanceToCenter * MinDistanceToCenter);
        }

        overlap += chord;

        return overlap/Path;
    }
}

void CalculateAlternativePaths(const AMCOM_ObjectState* List, const position* StartPosition, position* EndPosition, size_t size, const AMCOM_ObjectState* GlueList, const AMCOM_ObjectState* Player, size_t GlueListSize){
    position FirstObject;
    position SecondObject;
    position current;
    position prev;
    float FirstPath = 0;
    float SecondPath = 0;
    float multiplier = 0;

    FindClosestObjectOfType(List, StartPosition, &FirstObject, 1, size, 0);
    FindClosestObjectOfType(List, StartPosition, &SecondObject, 1, size, 1);

    prev = *StartPosition;
    current = FirstObject;
    for(size_t i = 0; i < 3; i++){
        multiplier = obstacle(GlueList, &prev, &current, Player, 200, GlueListSize);
        FirstPath += (1.0 + multiplier * 20) + CalculatePath(&prev, &current);
        prev = current;
        FindClosestObjectOfType(List, &prev, &current, 1, size, 0);
    }

    prev = *StartPosition;
    current = SecondObject;
    for(size_t i = 0; i < 3; i++){
        SecondPath += CalculatePath(&prev, &current);
        prev = current;
        FindClosestObjectOfType(List, &prev, &current, 1, size, 0);
    }

    if(FirstPath < SecondPath){
        *EndPosition = FirstObject;
    }
    
    *EndPosition = SecondObject;
}