/********************************************************************
 *
 *      File:   tcam_entry_mgr.h
 *      Name:   Basavaraj Bendigeri
 *
 *       Description:
 *  This header file contains the  declarations for the API implemented  in the TCAM
 *  Bank handler code
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
#include <stdint.h>
#include "tcam_defs.h"

#ifndef __TCAM_ENTRY_MGR_H__
#define __TCAM_ENTRY_MGR_H__

/*  Description:
 *     This API initializes a TCAM bank handler (TCAM cache ) serving the
 *     given
 *     hw_tcam.
 *     It returns  in case of success and a pointer to the handler
 *
 * Arguments
 *  hw_tcam - address of hardware tcam memory
 *  size    - size of the memory to be allocated
 *  tcam    - pointer to memory allocated
 * Return: TCAM_ERR_SUCCESS or appropriate error code.
 */
tcam_err_t tcam_init(entry_t *hw_tcam, uint32_t size, void **tcam);

/*  Description:
 *     This API inserts a batch of entries into the TCAM Bank handler (A.K.A
 *     TCAM cache) referred to by the ‘tcam’ parameter.
 *     The insert is either successful entirely or it fails and nothing
 *     is inserted. Each entry in the batch has a priority and id .
 *     The entries are inserted into the TCAM bank handler in a sorted
 *     manner with  the value of the priority field as a key. The
 *     entries are sorted in ascending order . All entried of the same
 *     priority are grouped together and the recent onces are at the start
 *     of the group. Thus entries with a lower priority value are treated
 *     with higher priority and are inserted at the start of the tcam  and
 *     in each priority group, the most recent ones are at the start of the
 *     group. Thus entries with a lower value and which are recent are
 *     treated with higher priority. Once these entries are inserted into
 *     'tcam' (TCAM bank handler) , they are then programmed in the hardware tcam table. The
 *     'tcam' is represented  by an in-memory data structure "tcam_cache"
 *     and the 'hw_tcam' is represented by a "hw_tcam_local" variables.
 * Arguments
 *  tcam - in memory tcam cache
 *  entries - entries to be inserted
 *  num - number of entries 
 * Return: TCAM_ERR_SUCCESS or appropriate error code.
 */
tcam_err_t tcam_insert(void *tcam, entry_t *entries, uint32_t num);

/*  Description:
 *       Delete a TCAM entry in the "tcam_cache" table as well as "hw_tcam".
 *       This function accepts "tcam" and the id of the entry to be deleted
 *       as arguments and deletes an entry with that id in both the
 *       "tcam_cache" as well "hw_tcam" tables. The deletion is done by
 *       setting the id field for that entry to TCAM_CELL_STATE_EMPTY.  
 *
 * Arguments
 *  tcam - in memory tcam cache
 *  id   - id of the entry to be deleted 
 * Return: TCAM_ERR_SUCCESS or appropriate error code.
 */
tcam_err_t tcam_remove(void *tcam, uint32_t id);

/*  Description:                    
 *  Helper function to free up the in-memory "tcam_cache" . This is used in
 *  case the caller wants to free up the "tcam_cache" memory
 * Arguments
 *  tcam - in memory tcam cache
 */
void tcam_cache_destroy(void *tcam);

/*  Description:
 *       This function prints the content of the TCAM Bank handler (TCAM cache).
 *       This function accepts "tcam"  as argument and prints the contents.
 * Arguments
 *  tcam - in memory tcam cache
 * Return: none
 */
void print_tcam_cache(void *tcam);

#endif

