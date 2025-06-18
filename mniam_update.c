/*
 * mniam_update.c
 *
 *  Created on: Jun 16, 2025
 *      Author: Hubert
 */

#include <stdint.h>
#include <stdio.h>

#include "mniam_update.h"

void SaveObject(AMCOM_ObjectState* List, const AMCOM_ObjectState* object, size_t size){
    uint8_t updated = 0;

    for(size_t j = 0; j < size; j++) {
                if(List[j].objectType == object->objectType && List[j].objectNo == object->objectNo ) {		//condition checking if the object is already saved in the list
                    if(object->hp == 0){																	//condition determining whether object is dead or should be updated
                        List[j].objectType = 5;
                        List[j].hp = 0;
                    } else {
                        List[j] = *object;
                    }
                    updated = 1;
                    break;
                }
            }

            if(updated == 0) {
                for(size_t j = 0; j < size; j++){
                    if(List[j].hp == 0) {
                        List[j] = *object;
                        break;
                    }
                }
            }
}
