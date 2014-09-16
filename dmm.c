#include <stdio.h> //needed for size_t
#include <unistd.h> //needed for sbrk
#include <assert.h> //For asserts
#include <limits.h> // For INT_MAX
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */

typedef struct metadata {
       /* size_t is the return type of the sizeof operator. Since the size of
 	* an object depends on the architecture and its implementation, size_t
	* is used to represent the maximum size of any object in the particular
 	* implementation.
	* size contains the size of the data object or the amount of free
 	* bytes
	*/
	size_t size;
	struct metadata* next;
	struct metadata* prev; //What's the use of prev pointer?
} metadata_t;

void coalesce(metadata_t* freedBlock);

/* freelist maintains all the blocks which are not in use; freelist is kept
 * always sorted to improve the efficiency of coalescing
 */

static metadata_t* freelist = NULL;

// dmalloc currently does first fit, want to change it to best fit later to improve performance
void* dmalloc(size_t numbytes) {
	if(freelist == NULL) { 			//Initialize through sbrk call first time
		if(!dmalloc_init())
			return NULL;
	}

	assert(numbytes > 0);

	/* Your code goes here */
	// print_freelist();
	if(numbytes == 0) {
		return NULL;
	}
	// Align the requested bytes so it's a multiple of 8 I believe
	int alignedNumbytes = ALIGN(numbytes);
	metadata_t *freelist_head = freelist;
	// Iterate through freelist following a first fit approach
	while(freelist_head != NULL) {
		metadata_t *allocatedBlock = freelist_head;
		// Check if there is enough room for the allocated block
		if(freelist_head->size >= alignedNumbytes) {
			// split if there's enough room for another header
			if(freelist_head->size - alignedNumbytes >= (size_t) METADATA_T_ALIGNED) {
				metadata_t* leftoverHeader = (void *) freelist_head + alignedNumbytes + METADATA_T_ALIGNED;
				leftoverHeader->size = freelist_head->size - alignedNumbytes - METADATA_T_ALIGNED;
				// Set leftoverHeader's pointers to the previous and next block if both are not null
				if(freelist_head->next != NULL && freelist_head->prev != NULL) {
					leftoverHeader->next = freelist_head->next;
					leftoverHeader->prev = freelist_head->prev;
					(leftoverHeader->next)->prev = leftoverHeader;
					(leftoverHeader->prev)->next = leftoverHeader;
				}
				// Set leftoverHeader's next pointer and set prev pointer to null
				else if(freelist_head->next != NULL && freelist_head->prev == NULL) {
					leftoverHeader->next = freelist_head->next;
					leftoverHeader->prev = NULL;
					(leftoverHeader->next)->prev = leftoverHeader;
					freelist = leftoverHeader;
				}
				// Set leftoverHeader's prev pointer and set next pointer to null
				else if(freelist_head->next == NULL && freelist_head->prev != NULL) {
					leftoverHeader->next = NULL;
					leftoverHeader->prev = freelist_head->prev;
					(leftoverHeader->prev)->next = leftoverHeader;
				}
				// Set both pointers to null
				else {
					leftoverHeader->next = NULL;
					leftoverHeader->prev = NULL;
					freelist = leftoverHeader;
				}
				// Set allocatedBlock's size equal to the requested size and return
				allocatedBlock->size = alignedNumbytes;
				// Returns pointer to the allocatedBlock, immediately after the header
				return (void *) allocatedBlock + METADATA_T_ALIGNED;
			}
			// allocate whole block because there isn't enough space for another header
			else {
				// If block is in the middle of two other blocks, remove it
				if(freelist_head->next != NULL && freelist_head->prev != NULL) {
					(freelist_head->next)->prev = freelist_head->prev;
					(freelist_head->prev)->next = freelist_head->next;
				}
				// If block is first in freelist, remove it
				else if(freelist_head->next != NULL && freelist_head->prev == NULL) {
					(freelist_head->next)->prev = NULL;
					freelist = freelist_head->next;
				}
				// If block is at the end in freelist, remove it
				else if(freelist_head->next == NULL && freelist_head->prev != NULL) {
					(freelist_head->prev)->next = NULL;
				}
				// Set allocatedBlock's size equal to the entire block
				allocatedBlock->size = freelist_head->size;
				// Returns pointer to the allocatedBlock, immediately after the header
				return (void *) allocatedBlock + METADATA_T_ALIGNED;
			}
			break;
		}
		freelist_head = freelist_head->next;
	}
	// Nothing left in the freelist can accomodate the request
	return NULL;
}

void dfree(void *ptr) {
	// print_freelist();
	/* Your free and coalescing code goes here */
	metadata_t *freelist_head = freelist;
	metadata_t *freedBlock = (void *) ptr - METADATA_T_ALIGNED;
	// Get the freedBlockAddress to check where block should be inserted
	void* freedBlockAddress = (void *) freedBlock;
	// boolean to check for edge case
	int blockInserted = 0;
	while(freelist_head != NULL) {
		// Address of the current freelist_head
		void* freelist_headAddress = (void *) freelist_head;
		// If the address of the freedblock is less than the address of the current freelist_head, it should be inserted here
		if(freedBlockAddress < freelist_headAddress) {
			// If the block is being inserted into the middle
			if(freelist_head->next != NULL && freelist_head->prev != NULL) {
				freedBlock->next = freelist_head;
				freedBlock->prev = freelist_head->prev;
				(freedBlock->prev)->next = freedBlock;
				(freedBlock->next)->prev = freedBlock;
			}
			// If the block is being inserted at the beginning, have to move freelist
			else if(freelist_head->next != NULL && freelist_head->prev == NULL) {
				freedBlock->next = freelist_head;
				freedBlock->prev = NULL;
				(freedBlock->next)->prev = freedBlock;
				freelist = freedBlock;
			}
			// If the block is being added at the end
			else if(freelist_head->next == NULL && freelist_head->prev != NULL) {
				freedBlock->next = freelist_head;
				freedBlock->prev = freelist_head->prev;
				(freedBlock->prev)->next = freedBlock;
				(freedBlock->next)->prev = freedBlock;
			}
			// If the block is being inserted and there's only one other block, have to move freelist
			else {
				freedBlock->next = freelist_head;
				freedBlock->prev = NULL;
				(freedBlock->next)->prev = freedBlock;
				freelist = freedBlock;
			}
			// Change boolean corresponding to whether or not block was inserted to 1
			blockInserted = 1;
			break;
		}
		freelist_head = freelist_head->next;
	}
	// If block was never added in the while loop, that means it belongs at the end of the freelist
	if(!blockInserted) {
		metadata_t* freelist_head2 = freelist;
		// Iterate through freelist until you get to the last node and insert the block after that
		// Probably a more efficient way to get to the end of the list without a second while like incorporating into first while
		while(freelist_head2 != NULL) {
			if(freelist_head2->next == NULL) {
				freelist_head2->next = freedBlock;
				(freelist_head2->next)->prev = freelist_head2;
				freedBlock->next = NULL;
				break;
			}
			freelist_head2 = freelist_head2->next;
		}
	}
	coalesce(freedBlock);
}

void coalesce(metadata_t* freedBlock) {
	// Checks if there is even a block after freedBlock
	if(freedBlock->next != NULL) {
	void* blockAddress = (void *) freedBlock;
		metadata_t* nextNode = freedBlock->next;
		void* nextAddress = (void *) nextNode;
		// If the next block is right next to the current block, they need to be coalesced
		if(nextAddress == blockAddress + freedBlock->size + METADATA_T_ALIGNED) {
			// Adjust size and pointers accordingly
			freedBlock->size = freedBlock->size + nextNode->size + METADATA_T_ALIGNED;
			if(nextNode->next != NULL) {
				freedBlock->next = nextNode->next;
				(freedBlock->next)->prev = freedBlock;
			}
			else {
				freedBlock->next = NULL;
			}
		}
	}

	// Checks if there is even a block before freedBlock
	if(freedBlock->prev != NULL) {
		void* blockAddress = (void *) freedBlock;
		metadata_t* prevNode = freedBlock->prev;
		void* prevAddress = (void *) prevNode;
		// If the previous block is right behind the current block, they need to be coalesced
		if(prevAddress == blockAddress - prevNode->size - METADATA_T_ALIGNED) {
			// Adjust size and pointers accordingly
			prevNode->size = prevNode->size + freedBlock->size + METADATA_T_ALIGNED;
			if(freedBlock->next != NULL){
				prevNode->next = freedBlock->next;
				(freedBlock->next)->prev= prevNode;
			}
			else {
				prevNode->next = NULL;
			}
		}
	}
}

bool dmalloc_init() {

	/* Two choices:
 	* 1. Append prologue and epilogue blocks to the start and the end of the freelist
 	* 2. Initialize freelist pointers to NULL
 	*
 	* Note: We provide the code for 2. Using 1 will help you to tackle the
 	* corner cases succinctly.
 	*/

	// Didn't use the prologue and epilogue blocks, so I just handled the corner cases
	size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
	freelist = (metadata_t*) sbrk(max_bytes); // returns heap_region, which is initialized to freelist
	/* Q: Why casting is used? i.e., why (void*)-1? */
	if (freelist == (void *)-1)
		return false;
	freelist->next = NULL;
	freelist->prev = NULL;
	freelist->size = max_bytes-METADATA_T_ALIGNED;
	return true;
}

/*Only for debugging purposes; can be turned off through -NDEBUG flag*/
void print_freelist() {
	metadata_t *freelist_head = freelist;
	while(freelist_head != NULL) {
		DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p, HAddress:%p, TAddress:%p\t",freelist_head->size,freelist_head,freelist_head->prev,freelist_head->next, (void *) freelist_head, (void *) freelist_head + METADATA_T_ALIGNED + freelist_head->size);
		freelist_head = freelist_head->next;
	}
	DEBUG("\n");
}
