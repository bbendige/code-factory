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

typedef struct entry_ {
    uint32_t id; //must be unique, 0 means empty TCAM entry
    uint32_t prio; //0 means the highest priority

} entry_t;


#endif
