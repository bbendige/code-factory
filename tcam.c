#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tcam_defs.h"
#include "tcam.h"

static uint64_t hw_access ;

void hw_tcam_init(entry_t *hw_tcam, uint32_t size) {
     memset(hw_tcam,0,  sizeof(entry_t)*size);
    hw_access = 0;
}

/* Description  
 *   This is the southbound API which implements the HW programming. This function is called by the tcam_insert() and tcam_remove() functions. 
 *  Arguments 
 *  tcam - hardware tcam
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

uint64_t tcam_get_hw_access_cnt()
{
    return hw_access;
}

