TCAM entry management

This code implements a TCAM Bank handler (A.K.A TCAM cache). Basically it manages the insertion/deletion of entries into a TCAM . 
It exports API to handle insertion / deletion of entries in a TCAM. It simulates the TCAM in an in-memory data structure in RAM 
and also maintains a mirror copy of the TCAM (which is the TCAM cache) also in RAM. Each entry will have an id and a priority value.
Following is the functionality implemented :
1.	The entries in TCAM (and also TCAM Cache) are sorted in ascending order with the priority field as the key. 
2.	All the entries with the same priority are grouped together .
3.	There can be multiple entries with the same priority but different Ids . Always, the latest entries are inserted at the start of the group
4.	The entries with low priority value are inserted at the top of the list. 
5.	Entries can be deleted using the id as key. 
6.	Sometimes , the entries can be shifted as a consequence of insertion


The code maintains 2 tables to implement this functionality :
1.	TCAM
The TCAM here is simulated in RAM by an array of structures , with each element representing one entry in the TCAM . 
Following is the structure of the entry :
typedef struct entry_ {
    uint32_t id; //must be unique, 0 means empty TCAM entry                                                                                                                                                                                                  
    uint32_t prio; //0 means the highest priority                                                                                                                                                                                                            

} entry_t;
The TCAM can incorporate a maximum of 2048 entries. The entries in this table are ordered based on the priority value

2.	TCAM Bank Handler (A.K.A TCAM Cache)
This is also an array of structures which would be a mirror copy of the TCAM. All the operations on the entries 
(deletion/addition/shifting) are first done in this table before replicating the same onto TCAM. 
Similar to the TCAM table , this will also be an array of structures with each entry in this table is represented 
by an element of the type : entry_t
This is required because the writes to TCAM have to be minimized . Once all the input entries are inserted or deleted 
in this table, then the same will be replicated to TCAM.
The size of the this table will be the same as that of  the TCAM memory.  The TCAM Cache has been implemented as an 
array simply for the ease of convenience of translating the indices from this table to TCAM . Other data structures 
like BST, B-Tree , AVL Tree , Linked list were considered for this table but the array was found most suitable 
for the following reasons :
1.	Insertion – BST , B-Tree and AVL tree were better in performance when insertion of an entry but translating that 
entry into an index into the TCAM would prove to be very complicated . The entry has to be inserted  into an empty 
slot in TCAM . There would be no straightforward way of translating an entry location in these data structures to an 
index in TCAM . Whereas an array index can be translated directly. 


2.	Deleting
Similar to insertion , deletion of an entry in BST/AVL / B-Tree would be quicker compared to an array but the same 
concerns w.r.t index translation to TCAM would hold true here also . The index of an entry deleted in the TCAM cache 
can translate to the exact index in TCAM. 



The following API will be implemented :
1.	tcam_init()
This API initializes the TCAM cache. The caller of the API will pass the TCAM and it’s size as paramaters to this API. 
The TCAM cache will be created with the size passed to it by the caller . 

2.	tcam_insert()
This API insert a TCAM entry first in the TCAM cache and then in the TCAM . The API does a linear search for an index 
in the TCAM cache in the following manner :
1.	The entry to be inserted has a priority value less than or equal to the next entry. If an empty index exists such 
that the next entry is non-empty, then that index is selected for insertion of this entry value . In this case, the 
index selected in TCAM for this entry will same as that in TCAM cache. 
2.	If no empty index is found, select the first non-empty entry which is greater  or equal to the entry to be inserted . 
Then that entry and the subsequent entries wil be shifted up by one position so that the new entry is inserted at the 
current index. This means that the entries will be moved from a lower index to a higher index i.e an entry at position ‘i’ 
will be moved to position ‘i+1’, ‘i-1’ to ‘i’ and so on. In this case , the TCAM will be populated with entries from the 
index where the new entry is inserted until the end of the TCAM cache table . However, the method of insertion is such that
entries at the end of the table will be first inserted and then so on until the index of the new entry. This is done so that
existing entries in TCAM will still hold true. So , if an entry is currently at position ‘i’ in TCAM , it would have already 
been moved to ‘i+1’ in TCAM cache. So the writing to TCAM begins from the ‘i+1’ index in TCAM cache. 
This effectively means that  TCAM ‘i+1’th entry will now have the same value as TCAM cache ‘i+1’th entry. Since the writing
to TCAM starts from the end , the existing  entries will not be overwritten. Rather they get shifted upwards by one position
which means they will still be effective .

Once the new entry is inserted into the TCAM cache , the values will be inserted into TCAM.  For a TCAM having ‘m’ entries 
with ‘n’ entries to be inserted , the insertion works as follows :
i.	For case 1 , highlighted above , the n entries will be inserted in O(n)
ii.	For case 2 , highlighted above, the n entries will be inserted in O(n+m)

3.	tcam_remove()
This API deletes an entry for a given priority value. A linear search is done in the TCAM cache table for the first such 
entry which matches the priority value and then marked for deletion. The method of deletion is that the ‘id’ for that entry 
is set to 0. Then the TCAM entry at the same position is also effectively modified . Thus the search is done in the TCAM 
cache for the entry , then the index is retrieved and used to modify the TCAM. 

The following source files contain the code which implements this functionality :
1.	tcam_entry_mgr.c
2.	tcam.c
3.	tcam_mgr_main.c

This code has been compiled on the following linux distributions :
1.	Redhat linux , 2.6 kernel
2.	Amazon Linux , 4.14 kernel 

The gcc compiler has been used to compile this code. It does’nt use any platform specific libraries. Hence the code can be 
compiled and executed on any linux distro . 
Following steps are needed to compile and test
1.	Download the code from github . The code contains the .c and .h files and a Makefile 
2.	Use “gmake” to compile the code. This will generate the binary “tcam_entry_mgr”. 
3.	Execute the “tcam_entry_mgr” which will then invoke the UT testcases which test all the API 
