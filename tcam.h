#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tcam_defs.h"
#ifndef __TCAM_H__
#define __TCAM_H__

void hw_tcam_init() ;
tcam_err_t tcam_program(entry_t *hw_tcam, entry_t *ent, uint32_t position);
uint64_t tcam_get_hw_access_cnt();
#endif
