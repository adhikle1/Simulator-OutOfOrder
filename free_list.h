#ifndef _FREE_LIST_H_
#define _FREE_LIST_H_

#include "apex_macros.h"

int flQueue[PREG_FILE_SIZE]; // Array implementation of Free List queue
int FL_head, FL_tail; // To handle FL Queue
int preg;

int get_free_physical_register(); // Function used to get_free_physical_register from Free List
void put_physicalRegister_inFL(); // Function used to put physical Register in FL 
int checkEmpty_FL(int FL_head); //Check if Free List is empty

#endif