/*
 * mniam_update.h
 *
 *  Created on: Jun 16, 2025
 *      Author: Hubert
 */

#ifndef INC_MNIAM_UPDATE_H_
#define INC_MNIAM_UPDATE_H_

#include "amcom_packets.h"

/**
 * Saves object to chosen List
 *
 * @param List pointer to the list where the object will be stored
 * @param object pointer to the AMCOM_ObjectState structure to the object to be stored
 * #param size size of the List
*/
void SaveObject(AMCOM_ObjectState* List, const AMCOM_ObjectState* object, size_t size);

#endif /* INC_MNIAM_UPDATE_H_ */
