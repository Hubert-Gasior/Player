/*
 * mniam_move.h
 *
 *  Created on: Jun 16, 2025
 *      Author: Hubert
 */

#ifndef INC_MNIAM_MOVE_H_
#define INC_MNIAM_MOVE_H_

#include "amcom_packets.h"

/** Structure describing object position. */
typedef struct {
    float x;
    float y;
}position;

/** Enumerate describing object id. */
typedef enum {
    PLAYER = 0,
    TRANSISTOR = 1,
    SPARK = 2,
    GLUE = 3,
    DEFAULT = 4

}ObjectType;

/** Enumerate describing obstacle diameters. */
typedef enum {
    SPARK_DIAMETER = 25,
    GLUE_DIAMETER = 200
}ObjectDiameter;


/**
 * Calculates squared distance between two objects.
 *
 * @param object_1 pointer to a position structure of first object
 * @param object_2 pointer to a position structure of second object
 * @return squared distance between two objects
*/
float CalculatePath(const position* object_1, const position* object_2);

/**
 * Calculates angle between two objects.
 *
 * @param StartPosition pointer to a position structure of starting position
 * @param EndPOsition pointer to a position structure of the objective position
 * @return angle in radians between starting and ending point
*/
float CalculateAngle(const position* StartPosition, const position* EndPosition);

/**
 * Checks if the object at designated position is alive.
 *
 * @param List pointer to a List of object where the object might be found
 * @param object pointer to a position structure of the designated position
 * @param size size of the List
 * @return 1 if the object is alive and 0 when the object is not
*/
uint8_t IsAlive(const AMCOM_ObjectState* List, const position* object, size_t size);

/**
 * Searches for the position of designated object
 *
 * @param List pointer to a list of objects where the position might be found
 * @param Object pointer to a position structure where the found position will be stored
 * @param ObjectNo number of the searched object
 * @param ObjectType number of the searched object type
 * @param size of the List
 * @return AMCOM_ObjectState structure of the found object
*/
AMCOM_ObjectState FindPosition(const AMCOM_ObjectState* List, position* Object, const uint8_t ObjectNo, uint8_t ObjectType, size_t size);

/**
 * Determine the closest or second closest object of chosen type
 *
 * @param List pointer to a list of object where the closest object might be found
 * @param StartPosition pointer to a position structure of the position for which the object is searched
 * @param object pointer to a position structure where the found object position will be stored
 * @param ObjectType number of the searched object type
 * @param size size of the List
 * @param SeconClosestObject if 1 the function returns and stores the position of the second closest object of chosen type, when 0 returns and stores the position of the first closest object
 * @return AMCOM_ObjectState structure of the found first or second closest object depending on SeconClosestObject parameter
*/
AMCOM_ObjectState FindClosestObjectOfType(const AMCOM_ObjectState* List, const position* StartPositon, position* Object, uint8_t ObjectType, size_t size, uint8_t SecondClosestObject);

/**
 * Chooses the next step based on calculated alternative paths
 *
 * @param List Pointer to list of objects from which the path is calculated
 * @param StartPosition pointer to a position structure of the object which is the starting point of the path
 * @param EndPosiotin pointer to a position structure where the found step will be stored
 * @param size size of the List
*/
void CalculateAlternativePaths(const AMCOM_ObjectState* List, const position* StartPosition, position* EndPosition, size_t size);


/**
 * Calculates the ratio between the distance to the obstacle and the length of the combined radiuses of the player and an obstacle
 *
 * @param ObstacleList pointer to a List where the obstacles are stored
 * @param Player pointer to a AMCOM_ObjectState structure of the player
 * @param diameter length of the obstacles diameter
 * @param size size of the list
 * @return ratio of the distance to an obstacle and the combined radiuses
*/
float AvoidObstacle(const AMCOM_ObjectState* ObstacleList, const AMCOM_ObjectState* Player, uint8_t diameter, size_t size);

/**
 * Searches for the best target among players
 *
 * @param ObjectList list of the objects from which the target is chosen
 * @param Player pointer to a AMCOM_ObjectState structure of the player
 * @param Target pointer to a position structure where the target position will be stored
 * @param size size of the list
 * @return AMCOM_ObjectState structure of the target
*/
AMCOM_ObjectState FindBestTarget(const AMCOM_ObjectState* ObjectList, const AMCOM_ObjectState* Player, position* Target, size_t size);

#endif /* INC_MNIAM_MOVE_H_ */
