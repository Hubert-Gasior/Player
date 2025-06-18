/*
 * mniam_move.c
 *
 *  Created on: Jun 16, 2025
 *      Author: Hubert
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <math.h>
#include "mniam_move.h"

float CalculatePath(const position* object_1, const position* object_2) {
	return ((object_2->x - object_1->x)*(object_2->x - object_1->x)) + ((object_2->y - object_1->y)*(object_2->y - object_1->y));
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

uint8_t IsAlive(const AMCOM_ObjectState* ObjectList, const position* object, size_t size){
    for(size_t i = 0; i < size; i++){
        if(ObjectList[i].x == object->x && ObjectList[i].y == object->y && ObjectList[i].hp != 0){
            return 1;
        }
    }
    return 0;
}

AMCOM_ObjectState FindPosition(const AMCOM_ObjectState* ObjectList, position* Object, uint8_t ObjectNo, uint8_t ObjectType, size_t size) {
	AMCOM_ObjectState FoundObject;
    FoundObject.hp = 0;

    for (size_t i = 0; i < size; i++) {
		if(ObjectList[i].objectType == ObjectType && ObjectList[i].objectNo == ObjectNo) {
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

void CalculateAlternativePaths(const AMCOM_ObjectState* List, const position* StartPosition, position* EndPosition, size_t size){
    position FirstObject;
    position SecondObject;
    position current;
    position prev;
    float FirstPath = 0;
    float SecondPath = 0;

    FindClosestObjectOfType(List, StartPosition, &FirstObject, TRANSISTOR, size, 0);
    FindClosestObjectOfType(List, StartPosition, &SecondObject, TRANSISTOR, size, 1);

    prev = *StartPosition;
    current = FirstObject;
    for(size_t i = 0; i < 3; i++){														//loop calculating sum of the distance for 3 next steps starting from first closest transistor
        FirstPath += CalculatePath(&prev, &current);
        prev = current;
        FindClosestObjectOfType(List, &prev, &current, TRANSISTOR, size, 0);
    }

    prev = *StartPosition;
    current = SecondObject;
    for(size_t i = 0; i < 3; i++){														//loop calculating sum of the distance for 3 next steps starting from second closest transistor
        SecondPath += CalculatePath(&prev, &current);
        prev = current;
        FindClosestObjectOfType(List, &prev, &current, TRANSISTOR, size, 0);
    }

    if(FirstPath < SecondPath){
        *EndPosition = FirstObject;
    }

    *EndPosition = SecondObject;
}

float AvoidObstacle(const AMCOM_ObjectState* ObstacleList, const AMCOM_ObjectState* Player, uint8_t diameter, size_t size){
    float PlayerRadius = (25.0 + Player->hp)/2.0;
    float ObstacleRadius = diameter/2.0;
    float CombinedRadius = PlayerRadius + ObstacleRadius + 50;		//added 50 for safety
    float DistanceToObstacle = 0.0;
    position ObstaclePosition;
    position PlayerPosition;
    float ratio = 0.0;

    PlayerPosition.x = Player->x;
    PlayerPosition.y = Player->y;

    for(size_t i = 0; i < size; i++){
        if(ObstacleList[i].hp == 0){
            continue;
        }
        ObstaclePosition.x = ObstacleList[i].x;
        ObstaclePosition.y = ObstacleList[i].y;
        DistanceToObstacle = sqrt(CalculatePath(&PlayerPosition, &ObstaclePosition));
        if(DistanceToObstacle > CombinedRadius){
            continue;
        }

            ratio += DistanceToObstacle/CombinedRadius;
    }
    return ratio;
}

AMCOM_ObjectState FindBestTarget(const AMCOM_ObjectState* ObjectList, const AMCOM_ObjectState* Player, position* Target, size_t size){
    AMCOM_ObjectState FoundTarget;
    position TargetPosition;
    AMCOM_ObjectState CandidateTarget;
    position PlayerPosition;
    float path = 0;
    float shortestPath = 0;

    FindPosition(ObjectList, &PlayerPosition, Player->objectNo, PLAYER, size);

    for(size_t i = 0; i < size; i++){
        if(i == Player->objectNo){
            continue;
        }

        CandidateTarget = FindPosition(ObjectList, &TargetPosition, i, PLAYER, size);
        if(0 == CandidateTarget.hp){
            continue;
        }

        path = CalculatePath(&PlayerPosition, &TargetPosition);
        if(CandidateTarget.hp < Player->hp && (shortestPath > path || 0 == shortestPath)){
            shortestPath = path;
            *Target = TargetPosition;
            FoundTarget = CandidateTarget;
        }
    }

    return FoundTarget;
}
