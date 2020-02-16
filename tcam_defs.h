/********************************************************************
 *
 *      File:   tcam_defs.h
 *      Name:   Basavaraj Bendigeri
 *
 *       Description:
 *  This header file contains the structure definitions used in the TCAM
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

#ifndef __TCAM_DEFS_H__
#define __TCAM_DEFS_H__

typedef enum _tcam_error_t_ {
    TCAM_ERR_SUCCESS = 0,
    TCAM_ERR_FULL = 1,
    TCAM_ERR_MEM_ALLOC_FAIL,
    TCAM_ERR_TCAM_FULL,
    TCAM_ERR_NULL_CACHE,
    TCAM_ERR_EINVAL,
    TCAM_ERR_INVALID_PRIO
} tcam_err_t;

#define    TCAM_CELL_STATE_EMPTY 0
#define    TCAM_CELL_STATE_BUSY  1

#define TCAM_MAX_ENTRIES 2048
typedef unsigned char bool;
#define TRUE 1
#define FALSE 0

/* This structure would be used to represent an entry in the TCAM cache and
 * hw_tcam tables. It has 2 fields :
 * id - Id of the entry
 * prio - priority of the entry . This will also define the insertion order
 * for the entry
 */
typedef struct entry_ {
    uint32_t id; //must be unique, 0 means empty TCAM entry
    uint32_t prio; //0 means the highest priority

} entry_t;

enum tcam_entry_shift_policy_t {
    TCAM_ENTRY_SHIFT_NO_SHIFT = 0,
    TCAM_ENTRY_SHIFT_UP = 1,
    TCAM_ENTRY_SHIFT_DOWN = 2,
    TCAM_ENTRY_SHIFT_UP_DOWN = 3
};

#endif
