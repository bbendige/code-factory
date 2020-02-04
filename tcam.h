#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tcam_defs.h"
#ifndef __TCAM_H__
#define __TCAM_H__

/*
 * Description :
 *     This is the southbound API which is invoked by the tcam_insert() and
 *     NB API to initialize the HW TCAM
 * Arguments:
 * hw_tcam - hw tcam
 * size - size of the tcam
 */

void hw_tcam_init() ;

/* Description
 *   This is the southbound API which implements the HW programming.
 *   This function is called by the tcam_insert() and tcam_remove() NB API.
 *  Arguments
 *  hw_tcam - hardware tcam
 *  ent - contains the id and priority of the entry to be programmed in tcam
 *  position - position where the entry has to be programmed
 * Return: TCAM_ERR_SUCCESS or appropriate error code.
 */

tcam_err_t tcam_program(entry_t *hw_tcam, entry_t *ent, uint32_t position);
uint64_t tcam_get_hw_access_cnt();
#endif
