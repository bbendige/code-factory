/********************************************************************
 *
 *      File:   tcam_entry_mgr.c
 *      Name:   Basavaraj Bendigeri
 *
 *       Description:
 *  This  file contains the code for the TCAM
 *  Bank handler API 
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
#include "tcam_defs.h"
#include "tcam_entry_mgr.h"
#include "tcam.h"

/*
 * Indicates the number of entries in the  TCAM Bank handler
 */
static int32_t total_tcam_entries ;
// Maximum size of the TCAM bank handler memory
static uint32_t max_tcam_entries ;
// Pointer to the hw_tcam . This used by the tcam_insert()/tcam_remove() to
// program the hw tcam
static entry_t *hw_tcam_local = NULL;

// Maintains the state of the entries in the TCAM bank handler memory
// Each entry can have either of the 2 states :
// TCAM_CELL_STATE_EMPTY or TCAM_CELL_STATE_BUSY
// This will point to an array of size max_tcam_entries and will be updated
// and looked up whenever an entry has to inserted into the TCAM bank
// handler and subsequently hw_tcam
bool *insert_list ;

/*  Description:
 *     This API initializes a TCAM bank handler (TCAM cache ) serving the given
 *     hw_tcam. 
 *     It returns  in case of success and a pointer to the handler
 *
 * Arguments
 *  hw_tcam - address of hardware tcam memory
 *  size    - size of the memory to be allocated 
 *  tcam    - pointer to memory allocated 
 * Return: TCAM_ERR_SUCCESS or appropriate error code.
 */

tcam_err_t tcam_init(entry_t *hw_tcam, uint32_t size, void **tcam)
{
    max_tcam_entries = size;
    *tcam = calloc(size, sizeof(entry_t));
    if(*tcam == NULL)
        return TCAM_ERR_MEM_ALLOC_FAIL;
    hw_tcam_init(hw_tcam, size);
    hw_tcam_local = hw_tcam;
    insert_list = calloc(size, sizeof(bool));
    if(insert_list == NULL)
        return TCAM_ERR_MEM_ALLOC_FAIL;
    return TCAM_ERR_SUCCESS;
}

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

tcam_err_t tcam_insert(void *tcam, entry_t *entries, uint32_t num)
{
    int32_t i , j, top , bottom  ;
    int32_t  insert_pos, shift_pos;
    uint64_t n1 , n2 ;
    entry_t entry;
    entry_t *tcam_cache = (entry_t *) tcam;
    bool shift_cell = FALSE, found = FALSE;
    
    if(tcam_cache == NULL)
        return TCAM_ERR_NULL_CACHE;

    // let's check if there is enough memory in the TCAM Bank handler A.K.A tcam cache to
    // incorporate these entries
    if((total_tcam_entries + num) >= max_tcam_entries)
       return TCAM_ERR_TCAM_FULL;

    printf("Total number of tcam entries before insertion : %d\n",total_tcam_entries);
    memset(insert_list, TCAM_CELL_STATE_EMPTY, max_tcam_entries);
    i = 0;
    top = max_tcam_entries;
    // First entry
    if(total_tcam_entries <= 0) {
        total_tcam_entries  = 1;
        tcam_cache[0] = entries[0];
        i = 1;
        top = 0;
        insert_list[0] = TCAM_CELL_STATE_BUSY;
    } 

    shift_cell = FALSE;
    for(; i < num; i++) {        
        found = FALSE;
        insert_pos = 0;
         for(j = 0; j < max_tcam_entries; j++) {
            if((tcam_cache[j].id != TCAM_CELL_STATE_EMPTY) && (tcam_cache[j].prio >= entries[i].prio)) {
                found = TRUE;
                break;
            }
        }
        
        if(found) {
            // We found a non-empty entry which either has prio >= to that
            // of this element. So we need to check for a empty slot to
            // insert. So first let's check if we can insert at the previous
            // slot else we may need to shift the old entries by one position
            if((j == 0) || ((j > 0) && (tcam_cache[j-1].id != TCAM_CELL_STATE_EMPTY))) {
                shift_cell = TRUE;
                // We have to shift the entries  by one cell to make
                // way for the new entry
                for(shift_pos = j+1 ;
                    ((tcam_cache[shift_pos].id != TCAM_CELL_STATE_EMPTY) && (shift_pos < max_tcam_entries));shift_pos++);
                
                if(shift_pos >= max_tcam_entries)
                    return TCAM_ERR_TCAM_FULL;
                
                memmove(&tcam_cache[j+1], &tcam_cache[j],(shift_pos-j) * sizeof(entry_t));
                insert_pos = j;
            } else
                insert_pos = j-1;
        } else {
            // Not found . We would have to insert the new element at
            // the end of the existing elements
            // Let's find an empty slot . We have to start from the end
            // of the tcam_cache table and move up to find an empty slot
            for(j = max_tcam_entries; (j > 0) && (tcam_cache[j].id == TCAM_CELL_STATE_EMPTY); j--);
            // We found a non-empty entry . So let's point j to the next
            // index/position which can be used for the new  entry
            insert_pos = j+1;
        }
        // Now let's copy the entry at the intended position , i.e "insert_pos"
        tcam_cache[insert_pos] = entries[i];
        // Let's record the position where the insertion was done. This
        // would be needed for programming hw_tcam
        if(insert_pos < top)
            top = insert_pos;
        insert_list[insert_pos] = TCAM_CELL_STATE_BUSY;
        total_tcam_entries++;
    }
    printf("The number of new entries is : %d\n", num);
    //printf("Total number of tcam entries : %d\n",total_tcam_entries);
    n1 = tcam_get_hw_access_cnt();
    /* We have to now program hw_tcam. We first check if any entries were
       shifted to accomodate the new entries. If so, then all the valid values in
       tcam_cache starting from end of tcam_cache to the index pointed to by
       "top" , are programmed in hw_tcam.
       Else the insert_list values contain the individual indices were
       programming has to be done. The valid values in that array will be
       programmed in hw_tcam
    */
       
    if(shift_cell) {
        for(i = max_tcam_entries; i >= top ; i--) {
            if(tcam_cache[i].id != TCAM_CELL_STATE_EMPTY)
                tcam_program(hw_tcam_local, &tcam_cache[i], i);
        }
    } else {
        for(i = 0; i < max_tcam_entries; i++) {
            if((insert_list[i]) && (tcam_cache[i].id != TCAM_CELL_STATE_EMPTY))
                tcam_program(hw_tcam_local, &tcam_cache[i], i);
        }
    }
    n2 = tcam_get_hw_access_cnt();
    printf("The number of programming to hw_tcam for %d entries is %llu\n",num, (n2-n1)); 
    return TCAM_ERR_SUCCESS;
}

/*  Description:
 *       This function prints the content of the TCAM Bank handler (TCAM cache).
 *       This function accepts "tcam"  as argument and prints the contents.
 * Arguments
 *  tcam - in memory tcam cache
 * Return: none
 */

void print_tcam_cache(void *tcam) {
    int j = 0;
    entry_t *tcam_cache = (entry_t *) tcam;
    for(j = 0; j < max_tcam_entries; j++) {
        if(tcam_cache[j].id != TCAM_CELL_STATE_EMPTY)
            printf("Index : %d -> Id : %d , Priority : %d\n",
                   j,tcam_cache[j].id, tcam_cache[j].prio);
    }

}

/*  Description:
 *       Delete a TCAM entry in the "tcam_cache" table as well as "hw_tcam".
 *       This function accepts "tcam" and the id of the entry to be deleted
 *       as arguments and deletes an entry with that id in both the
 *       "tcam_cache" as well "hw_tcam" tables. The deletion is done by
 *       setting the id field for that entry to 0.  
 *
 * Arguments
 *  tcam - in memory tcam cache
 *  id   - id of the entry to be deleted 
 * Return: TCAM_ERR_SUCCESS or appropriate error code.
 */
tcam_err_t tcam_remove(void *tcam, uint32_t id) {
    uint32_t i , position = 0;
    bool found = FALSE;
    entry_t *tcam_cache = (entry_t *) tcam;

    if(tcam_cache == NULL)
        return TCAM_ERR_NULL_CACHE;
    
    for(i = 0; i < max_tcam_entries ; i++) {
        if(tcam_cache[i].id == id) {
            found = TRUE;            
            break;
        }
    }
    if(!found)
        return TCAM_ERR_EINVAL;

    position = i;
    //printf("The entry has to be deleted in hw_tcam at position %d\n", position);
    tcam_cache[position].id = 0;
    if(tcam_program(hw_tcam_local, &tcam_cache[position], position) == TCAM_ERR_SUCCESS) {
        tcam_cache[position].id = TCAM_CELL_STATE_EMPTY;
        tcam_cache[position].prio = 0;
        total_tcam_entries--;
    }
    return TCAM_ERR_SUCCESS;
}

/*  Description:
 *  Helper function to free up the in-memory "tcam_cache" . This is used in
 *  case the caller wants to free up the "tcam_cache" memory
 * Arguments
 *  tcam - in memory tcam cache
 */

void tcam_cache_destroy(void *tcam)
{
    if(tcam != NULL)
        free(tcam);
    tcam = NULL;
    max_tcam_entries = 0;
    hw_tcam_local = NULL;
    if(insert_list != NULL)
        free(insert_list);
    insert_list = NULL;
    total_tcam_entries = 0;
}
