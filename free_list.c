#include <stdio.h>
#include <stdlib.h>

#include "free_list.h"


void put_physicalRegister_inFL(int preg)
{
    if ((FL_head == FL_tail + 1) || (FL_head == 0 && FL_tail == PREG_FILE_SIZE - 1))
        printf("Queue Overflow\n");
    else
    {
        if (FL_head == - 1)
        {
            FL_head = 0;
        }

        //printf("Enter the Physical Register to be get_free_physical_registered in the Free List: ");
        //scanf("%d", &preg);
        FL_tail = (FL_tail+1) % PREG_FILE_SIZE;
        flQueue[FL_tail] = preg;
        //printf("FL_tail at %d", FL_tail);
    }
}

int get_free_physical_register()
{
    int preg;
    if (checkEmpty_FL(FL_head)) 
    {
        printf("No physical registers available\n");
        return -1;
    }
    else
    {
        preg = flQueue[FL_head];
        if (FL_head == FL_tail) 
        {
            FL_head = FL_tail = -1;
        } 
        else 
        {
            //printf("Physical Register issued P%d\n", preg);
            FL_head = (FL_head + 1) % PREG_FILE_SIZE;
            //printf("FL_head at %d\n", FL_head);
        }
    }
    return preg;
}

int checkEmpty_FL(int FL_head)
{
    if (FL_head == -1) 
    {
        return 1;
    }
  return 0;
}