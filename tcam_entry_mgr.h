#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tcam_defs.h"

#ifndef __TCAM_ENTRY_MGR_H__
#define __TCAM_ENTRY_MGR_H__

/* Initialize a TCAM bank handler serving the given hw_tcam,
 * returns 0 in case of success and a pointer to the handler
 */

tcam_err_t tcam_init(entry_t *hw_tcam, uint32_t size, void **tcam);
tcam_err_t tcam_insert(void *tcam, entry_t *entries, uint32_t num);

tcam_err_t tcam_remove(void *tcam, uint32_t id);

/*  Description:                    
 *  Helper function to free up the in-memory "tcam_cache" . This is used in
 *  case the caller wants to free up the "tcam_cache" memory
 * Arguments
 *  tcam - in memory tcam cache
 */

void tcam_cache_destroy(void *tcam);
#endif

