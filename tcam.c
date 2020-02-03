/********************************************************************
 *
 *      File:   tcam.c
 *      Name:   Basavaraj Bendigeri
 *
 *       Description:
 *  This  file contains the code for the HW TCAM
 *   API 
 *
 *
 *
 *
 *
 *
 *********************************************************************
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tcam_defs.h"
#include "tcam.h"

static uint64_t hw_access ;

/*
 * Description :
 *     This is the southbound API which is invoked by the tcam_insert() and
 *     NB API to initialize the HW TCAM 
 * Arguments:
 * hw_tcam - hw tcam
 * size - size of the tcam
 */
void hw_tcam_init(entry_t *hw_tcam, uint32_t size) {
     memset(hw_tcam,0,  sizeof(entry_t)*size);
    hw_access = 0;
}

/* Description  
 *   This is the southbound API which implements the HW programming.
 *   This function is called by the tcam_insert() and tcam_remove() NB API. 
 *  Arguments 
 *  hw_tcam - hardware tcam
 *  ent - contains the id and priority of the entry to be programmed in tcam   
 *  position - position where the entry has to be programmed 
 * Return: TCAM_ERR_SUCCESS or appropriate error code. 
 */

tcam_err_t tcam_program(entry_t *hw_tcam, entry_t *ent, uint32_t position) {

    if (position >= TCAM_MAX_ENTRIES)
        return TCAM_ERR_EINVAL;

    memcpy(hw_tcam+position, ent, sizeof(entry_t));
    hw_access++;

    return TCAM_ERR_SUCCESS;
}

/* Description
 *   This is used to get the count for the accesses to the hw tcam . This
 *   would help the caller to check the number of accessed to hw tcam
 *  Arguments
 * Return: The number of accesses to hw tcam 
 */
uint64_t tcam_get_hw_access_cnt()
{
    return hw_access;
}

