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

/* When the tcam_cache cells are shifted up and down, the entire range of indices has to be recorded, 
 * the entries within that range will have to be programmed in hw_tcam. This array is used to
 * record that range .
 */ 
bool *shift_window ;


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

    shift_window = calloc(size, sizeof(bool));
    if(shift_window == NULL)
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
    bool shift_up = FALSE, shift_down = FALSE, found = FALSE;
    int32_t shift_policy = TCAM_ENTRY_SHIFT_NO_SHIFT;
    int32_t shift_start , shift_end;
    
    if(tcam_cache == NULL)
        return TCAM_ERR_NULL_CACHE;

    printf("Total number of tcam entries before insertion : %d\n",total_tcam_entries);
    printf("The number of new entries is : %d\n", num);
    // let's check if there is enough memory in the TCAM Bank handler A.K.A tcam cache to
    // incorporate these entries
    if((total_tcam_entries + num) > max_tcam_entries) {
        printf("The number of entries exceed the maximum number\n");
       return TCAM_ERR_TCAM_FULL;
    }


    memset(insert_list, TCAM_CELL_STATE_EMPTY, max_tcam_entries);
    memset(shift_window, TCAM_CELL_STATE_EMPTY, max_tcam_entries);
        
    i = 0;
    top = max_tcam_entries;
    // First entry
    if(total_tcam_entries <= 0) {
        total_tcam_entries  = 1;
        insert_pos = 0;
        tcam_cache[insert_pos] = entries[0];
        i = 1;
        insert_list[insert_pos] = TCAM_CELL_STATE_BUSY;
        shift_window[insert_pos] = TCAM_CELL_STATE_BUSY;
    } 

    shift_up = shift_down = FALSE;
    shift_start = shift_end = -1;
   
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

                // We have to shift the entries  by one cell to make
                // way for the new entry
                for(shift_pos = j+1 ;
                    ((tcam_cache[shift_pos].id != TCAM_CELL_STATE_EMPTY) && (shift_pos < max_tcam_entries));shift_pos++);
                
                if(shift_pos >= max_tcam_entries) {
                    // We could'nt find an entry to shift down . So let's check if we can find an empty entry to shift upwards
                    insert_pos = (j-1);
                    for(shift_pos = insert_pos; ((tcam_cache[shift_pos].id != TCAM_CELL_STATE_EMPTY) && (shift_pos >= 0));shift_pos--);

                    if(shift_pos <= 0) {
                        printf("ERROR : Could'nt find an empty entry slot  \n");
                        return TCAM_ERR_TCAM_FULL;
                    }
                    memmove(&tcam_cache[shift_pos], &tcam_cache[shift_pos+1],(insert_pos-shift_pos) * sizeof(entry_t));
                    // Indicate that we had to shift up
                    shift_up = TRUE;
                    // since the entries are shifted , we have to record the start and end of the range of entries
                    shift_window[shift_pos] = TCAM_CELL_STATE_BUSY; // let's record the start
                    shift_window[j] = TCAM_CELL_STATE_BUSY;         // record the end
                } else {
                    insert_pos = j;
                    memmove(&tcam_cache[j+1], &tcam_cache[j],(shift_pos-insert_pos) * sizeof(entry_t));
                    // Indicate that we had to shift up
                    shift_down = TRUE;
                    // since the entries are shifted , we have to record the start and end of the range of entries
                    shift_window[j] = TCAM_CELL_STATE_BUSY; // let's record the start
                    shift_window[shift_pos] = TCAM_CELL_STATE_BUSY;         // record the end
                }
            } else  { // empty slot found
                insert_pos = j-1;
                /* Even when entries are not shifted , we have to record the position , since there may be other entries
                 * in the input for which shfiting has to be done. This index is recorded so that it may not be missed 
                 */
                shift_window[insert_pos] = TCAM_CELL_STATE_BUSY;  
            }
        } else {
            /* No valid slot found. There are 3 reasons for this to happen :
             * 1. We could not find an empty slot at all
             * 2. There are empty slots in the middle of the tcam cache but this entry has to be inserted at the 
             *     end of the table.
             * 3. There are empty slots at the end of the tcam cache and this entry has to be inserted at the
             *    first such entry
             *    So we have to first start from the end of the cache and iterate backwards to find an empty slot .
             *    If no empty slot is found, then we have to return an error. 
             *    To check for case 2 mentioned above, let's check if the last entry (for e.g, 2047th) entry is occupied . This means 
             *    we have to shift the entries upwards and insert the new entry at the end 
             *    If the last entry is empty, then we just iterate backwards until we find the first non-empty entry and then 
             *    insert this new entry after that position 
             */

            insert_pos = max_tcam_entries-1;
            if(tcam_cache[insert_pos].id != TCAM_CELL_STATE_EMPTY) {

                for(shift_pos = insert_pos ; (shift_pos >= 0) && (tcam_cache[shift_pos].id != TCAM_CELL_STATE_EMPTY); shift_pos--);

                if(shift_pos < 0 ) {
                    // All entries are full. Not empty slot found  found . Return an error
                    printf("ERROR : Could'nt find an empty slot to shift the entries upwards \n");
                    return TCAM_ERR_TCAM_FULL;
                }
                memmove(&tcam_cache[shift_pos], &tcam_cache[shift_pos+1],(insert_pos-shift_pos) * sizeof(entry_t));
                // since the entries are shifted , we have to record the start and end of the range of entries
                shift_window[shift_pos] = TCAM_CELL_STATE_BUSY; // let's record the start
                shift_window[insert_pos] = TCAM_CELL_STATE_BUSY;         // record the end    
                shift_up = TRUE;
            } else {
                /* We iterate backwards until we hit a non-empty slot. Then we just insert the new entry at 
                 * the (non-empty slot index + 1)
                 */
                for(shift_pos = insert_pos ; (shift_pos >= 0) && (tcam_cache[shift_pos].id == TCAM_CELL_STATE_EMPTY); shift_pos--);
                
                /*  Found an entry or none at all. If found, we insert at the 'j'th index or at the '0'th index.
                 *  There is a small caveat here . If in case the value of j < 0, then that means that all entries 
                 *  are emtpy from end to start of the cache. Should'nt happen but that also will be handled, when we 
                 *  insert at the '0'th index 
                 */
                insert_pos = shift_pos+1;
                /* Even when entries are not shifted , we have to record the position , since there may be other entries
                 * in the input for which shfiting has to be done. This index is recorded so that it may not be missed 
                 */
                shift_window[insert_pos] = TCAM_CELL_STATE_BUSY; 
            }
        }
        // Now let's copy the entry at the intended position , i.e "insert_pos"
        tcam_cache[insert_pos] = entries[i];
        insert_list[insert_pos] = TCAM_CELL_STATE_BUSY;
        total_tcam_entries++;
    }
    /* if the entries were either shifted up or down, then we have to iterate through the shift_window 
     * to record the start and end of that window. This will help when have to program that range of entries
     * in hw_tcam
     */
    if(shift_up || shift_down) {
        for(i = 0; (i < max_tcam_entries) && (shift_window[i] != TCAM_CELL_STATE_BUSY); i++);
        shift_start = i;

        for(i =	max_tcam_entries; (i > 0) && (shift_window[i] != TCAM_CELL_STATE_BUSY); i--);
        shift_end = i;
    }
    shift_policy = TCAM_ENTRY_SHIFT_NO_SHIFT;
    if(shift_up && shift_down) {
        shift_policy = TCAM_ENTRY_SHIFT_UP_DOWN;
        printf("Shift policy = TCAM_ENTRY_SHIFT_UP_DOWN\n");
    }
    else if(shift_up) {
        printf("Shift policy = TCAM_ENTRY_SHIFT_UP\n");
        shift_policy = TCAM_ENTRY_SHIFT_UP;
    }    else if(shift_down) {
        shift_policy = TCAM_ENTRY_SHIFT_DOWN;
        printf("Shift policy = TCAM_ENTRY_SHIFT_DOWN\n");
    }

    //printf("Total number of tcam entries : %d\n",total_tcam_entries);
    n1 = tcam_get_hw_access_cnt();
    
    switch(shift_policy) {
    case TCAM_ENTRY_SHIFT_NO_SHIFT:
        /* No entries were shifted. So we just program the entries at that index
         * in hw_tcam. This will be a proper O(n) solution
         */
        printf("Shift policy = TCAM_ENTRY_NO_SHIFT\n");
        for(i = 0; i < max_tcam_entries; i++) {
            if((insert_list[i]) && (tcam_cache[i].id != TCAM_CELL_STATE_EMPTY))
                tcam_program(hw_tcam_local, &tcam_cache[i], i);
        }
        break;

    case TCAM_ENTRY_SHIFT_UP:
    case TCAM_ENTRY_SHIFT_UP_DOWN:

        printf("Writing entries from %d to %d\n",shift_start, shift_end);
        for(i = shift_start ; i <= shift_end; i++) {
            if(tcam_cache[i].id != TCAM_CELL_STATE_EMPTY)
                tcam_program(hw_tcam_local, &tcam_cache[i], i);
        }
        break;

    case TCAM_ENTRY_SHIFT_DOWN:
        
        for(i = shift_end ; i >= shift_start; i--) {
            if(tcam_cache[i].id != TCAM_CELL_STATE_EMPTY)
                tcam_program(hw_tcam_local, &tcam_cache[i], i);
        }
        break;

    default :
        printf("Invalid \n");
        break;
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
