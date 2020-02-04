/********************************************************************
 *
 *      File:   tcam_mgr_main.c
 *      Name:   Basavaraj Bendigeri
 *
 *       Description:
 *  This  file contains the UT code for testing the TCAM Bank Handler code
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
#include "tcam_entry_mgr.h"
static entry_t hw_tcam[TCAM_MAX_ENTRIES];
//static uint64_t hw_access;

void print_hw_tcam();
int test_tcam_program();

typedef int (*ut_ptr_t)();

int delete_tcam_entries(void *tcam, int *entries, int num)
{
    int i = 0;
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;
    
    for(i = 0; i < num; i++) {
        if((ret_val = tcam_remove(tcam, entries[i])) != TCAM_ERR_SUCCESS) {
            printf("tcam_remove failed : %d \n", ret_val);
            return FALSE;
        }
    }
    return TRUE;
}

int test_null_tcam_insert()
{
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;
    void *tcam = NULL;
    entry_t entry[3];
    int result = TRUE;
    
    entry[0].prio = 100;
    entry[0].id = 1;
    entry[1].prio = 300;
    entry[1].id = 2;

    entry[2].prio = 200;
    entry[2].id = 3;
    printf("Test to check for NULL TCAM table\n");
    ret_val = tcam_insert(tcam, entry, 3);
    if(ret_val == TCAM_ERR_NULL_CACHE)
        printf("Test case passed\n");
    else {
        printf("Test case failed\n");
        result = FALSE;
    }
    return result;
}

int test_null_tcam_remove()
{
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;
    void *tcam = NULL;
    entry_t entry[3];
    int result = TRUE;
    
    printf("Test to check for deletion from empty TCAM table\n");
    ret_val = tcam_remove(tcam, 3);
    if(ret_val == TCAM_ERR_NULL_CACHE)
        printf("Test case passed\n");
    else {
        result = FALSE;
        printf("Test case failed\n");
    }
    return result;
}

int test_invalid_id_tcam_remove()
{
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;
    void *tcam = NULL;
    entry_t entry[3];
    int result = TRUE;
    
    printf("Test to check for deletion of invalid entry in TCAM table\n");
    if((ret_val = tcam_init(hw_tcam, TCAM_MAX_ENTRIES, &tcam)) != TCAM_ERR_SUCCESS) {
        printf("tcam_init error\n");
        
    }
    entry[0].prio = 100;
    entry[0].id = 1;
    entry[1].prio = 300;
    entry[1].id = 2;

    entry[2].prio = 200;
    entry[2].id = 3;
    if((ret_val = tcam_insert(tcam, entry, 3)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        printf("Testcase failed\n");
        return FALSE;
    }

    ret_val = tcam_remove(tcam, 10);
    if(ret_val == TCAM_ERR_EINVAL) {
        printf("Invalid entry\n");
        printf("Test case passed\n");
    } else {
        printf("Test case failed\n");
        result = FALSE;
    }
    tcam_cache_destroy(tcam);
    return result;
}


int test_full_tcam()
{
    entry_t entries1[5] = {{1, 300}, {2,100}, {3,500}, {4,400},{5,600}};
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;
    void *tcam = NULL;
    entry_t entry[3];
    int result = TRUE, i, prio, id;

    printf("Test case to check for FULL TCAM Table\n");
    if((ret_val = tcam_init(hw_tcam, TCAM_MAX_ENTRIES, &tcam)) != TCAM_ERR_SUCCESS) {
        printf("tcam_init error\n");
        exit(1);
    }
    for(i = 0, prio = 20, id = 1; i < 2045; i++, id +=10, prio +=10) {
        entry[0].prio = prio;
        entry[0].id = id;
        if((ret_val = tcam_insert(tcam, entry, 1)) != TCAM_ERR_SUCCESS) {
            printf("tcam_insert failed : %d \n", ret_val);
            return FALSE;
        }
    }

    if((ret_val = tcam_insert(tcam, entry, 5)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        result = TRUE;
    } else
        result = FALSE;
    tcam_cache_destroy(tcam);
    return result;
}

int test_tcam_insert_1()
{
    entry_t entries1[5] = {{1, 300}, {2,100}, {3,500}, {4,400},{5,600}};
    entry_t entries2[5] = {{6, 1000}, {7,200}, {8,500}, {9,400},{10,600}};
    int delete_entries[3] = {1,2,4};
    void *tcam = NULL;
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;
    int num = 3;
    printf("Test case to check for insert of values into TCAM \n");
    if((ret_val = tcam_init(hw_tcam, TCAM_MAX_ENTRIES, &tcam)) != TCAM_ERR_SUCCESS) {
        printf("tcam_init error\n");
        return FALSE;
    }
    
    if((ret_val = tcam_insert(tcam, entries1, 5)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    print_hw_tcam();
    num = sizeof(delete_entries)/sizeof(int);
    printf("Deleting %d entries from TCAM \n",num);
    delete_tcam_entries(tcam, delete_entries, num);
    printf("Deleted entries in TCAM\n");
    if((ret_val = tcam_insert(tcam, entries2, 5)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    print_hw_tcam();
    tcam_cache_destroy(tcam);
    
    return TRUE;
}

int test_tcam_insert_2()
{
    entry_t entries1[5] = {{1, 300}, {2,100}, {3,500}, {4,400},{5,600}};
    entry_t entries2[5] = {{1, 300}, {2,200}, {3,700}, {4,800},{5,900}};
    void *tcam = NULL;
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;

    printf("Test case to check for insert of values into TCAM \n");
    if((ret_val = tcam_init(hw_tcam, TCAM_MAX_ENTRIES, &tcam)) != TCAM_ERR_SUCCESS) {
        printf("tcam_init error\n");
        return FALSE;
    }

    if((ret_val = tcam_insert(tcam, entries1, 5)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    print_hw_tcam();


    if((ret_val = tcam_remove(tcam, 1)) != TCAM_ERR_SUCCESS) {
        printf("tcam_remove failed : %d \n", ret_val);
        return FALSE;
    }

    if((ret_val = tcam_remove(tcam, 2)) != TCAM_ERR_SUCCESS) {
        printf("tcam_remove failed : %d \n", ret_val);
        return FALSE;
    }
    
    if((ret_val = tcam_remove(tcam, 5)) != TCAM_ERR_SUCCESS) {
        printf("tcam_remove failed : %d \n", ret_val);
        return FALSE;
    }

    print_hw_tcam();
    if((ret_val = tcam_insert(tcam, entries2, 5)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    print_hw_tcam();
    tcam_cache_destroy(tcam);
    
    return TRUE;
}

int test_tcam_insert_3()
{
    entry_t entries1[10] = {{1, 100}, {2,50}, {3,550}, {4,250},{5,650},{6,800}, {7,100}, {8,450}, {9,350},{10,950}};
    entry_t entries2[3] = {{21, 1000}, {22,150}, {23,1500}};
    int delete_entries[3] = {7,8,1};
    void *tcam = NULL;
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;

    printf("%s : Test case to check for insert of values into TCAM \n", __FUNCTION__);
    if((ret_val = tcam_init(hw_tcam, TCAM_MAX_ENTRIES, &tcam)) != TCAM_ERR_SUCCESS) {
        printf("tcam_init error\n");
        return FALSE;
    }

    if((ret_val = tcam_insert(tcam, entries1, 10)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    print_hw_tcam();
    delete_tcam_entries(tcam, delete_entries, 3);
    
    //print_hw_tcam();
    
    if((ret_val = tcam_insert(tcam, entries2, 3)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    print_hw_tcam();
    tcam_cache_destroy(tcam);
    
    return TRUE;
}

int test_tcam_insert_4()
{
    entry_t entries1[10] = {{1, 100}, {2,50}, {3,550}, {4,250},{5,650},{6,800}, {7,100}, {8,450}, {9,350},{10,950}};
    entry_t entries2[3] = {{21, 200}, {22,150}, {23,400}};

    void *tcam = NULL;
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;

    printf("Test case to check for insert of values into TCAM \n");
    if((ret_val = tcam_init(hw_tcam, TCAM_MAX_ENTRIES, &tcam)) != TCAM_ERR_SUCCESS) {
        printf("tcam_init error\n");
        return FALSE;
    }

    if((ret_val = tcam_insert(tcam, entries1, 10)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    print_hw_tcam();

    if((ret_val = tcam_remove(tcam, 7)) != TCAM_ERR_SUCCESS) {
        printf("tcam_remove failed : %d \n", ret_val);
        return FALSE;
    }

    if((ret_val = tcam_remove(tcam, 1)) != TCAM_ERR_SUCCESS) {
        printf("tcam_remove failed : %d \n", ret_val);
        return FALSE;
    }

    if((ret_val = tcam_remove(tcam, 8)) != TCAM_ERR_SUCCESS) {
        printf("tcam_remove failed : %d \n", ret_val);
        return FALSE;
    }
    
    print_hw_tcam();
    
    if((ret_val = tcam_insert(tcam, entries2, 3)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    print_hw_tcam();
    tcam_cache_destroy(tcam);
    
    return TRUE;
}

int test_tcam_program()
{
    tcam_err_t ret_val = TCAM_ERR_SUCCESS;
    void *tcam = NULL;
    entry_t entry[10];
    int32_t id = 0;
    if((ret_val = tcam_init(hw_tcam, TCAM_MAX_ENTRIES, &tcam)) != TCAM_ERR_SUCCESS) {
        printf("tcam_init error\n");
        return FALSE;
    }
    entry[0].prio = 100;
    entry[0].id = 1;
    entry[1].prio = 300;
    entry[1].id = 2;

    entry[2].prio = 200;
    entry[2].id = 3;

    entry[3].prio = 400;
    entry[3].id = 4;
    entry[4].prio = 500;
    entry[4].id = 5;

    entry[5].prio = 600;
    entry[5].id = 6;

    entry[6].prio = 700;
    entry[6].id = 7;

    entry[7].prio = 800;
    entry[7].id = 8;
    
    if((ret_val = tcam_insert(tcam, entry, 8)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    //print_tcam_cache(tcam);
    print_hw_tcam();
    id = 3;
    if((ret_val = tcam_remove(tcam, id)) != TCAM_ERR_SUCCESS) {
        printf("tcam_remove failed : %d \n", ret_val);
        return FALSE;
    }
    //print_tcam_cache(tcam);
    print_hw_tcam();
    id = 6;
    if((ret_val = tcam_remove(tcam, id)) != TCAM_ERR_SUCCESS) {
        printf("tcam_remove failed : %d \n", ret_val);
        return FALSE;
    }
    printf("TCAM cache after deleting id : %d\n", id);
    //print_tcam_cache(tcam);
    print_hw_tcam();
    
    entry[0].prio = 150;
    entry[0].id = 10;

    entry[1].prio = 550;
    entry[1].id = 11;

    if((ret_val = tcam_insert(tcam, entry, 2)) != TCAM_ERR_SUCCESS) {
        printf("tcam_insert failed : %d \n", ret_val);
        return FALSE;
    }
    
    //print_tcam_cache(tcam);
    print_hw_tcam();
    tcam_cache_destroy(tcam);
    return TRUE;
}

void print_hw_tcam()
{
    int i ;
    
    for(i = 0; i < TCAM_MAX_ENTRIES; i++) {
        if(hw_tcam[i].id > 0) {
            printf("hw_tcam index : %d , Id : %d, Priority : %d\n",i,hw_tcam[i].id, hw_tcam[i].prio);
        }
    }
}

int main()
{
    ut_ptr_t ut_fn[9] ={test_full_tcam,test_tcam_insert_1, test_null_tcam_insert, test_null_tcam_remove,
                        test_invalid_id_tcam_remove,test_tcam_insert_2, test_tcam_insert_3,
                        test_tcam_insert_4, test_tcam_program};
    int result = 1, total_tests = 0;
    int fail_count = 0, pass_count = 0, i;
    total_tests = 9;
    for(i = 0;i < total_tests; i++) {
        printf("\n Test Case %d\n",i);
        result = (*ut_fn[i])();
        if(!result)
            fail_count++;
        else
            pass_count++;
        
    }
    printf("Total tests : %d , Passed tests : %d , Failed tests : %d\n",
           total_tests, pass_count, fail_count);
}
