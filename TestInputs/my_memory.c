#include <stdio.h>
#include <stdlib.h>

#define FIRST_FIT 0
#define BEST_FIT 1
#define WORST_FIT 2
#define BUDDY_SYSTEM 3

// Data Structures
//  Doubly Linked list to keep track of memory
typedef struct MemoryNode MemoryNode;
struct MemoryNode {
    MemoryNode *prev;
    MemoryNode *next;
    int start_ptr;
    int size;
    int used;
};

typedef struct MemoryList MemoryList;
struct MemoryList {
    MemoryNode *head;
    MemoryNode *tail;
    int size;
};

// Function prototypes
void listTailInsert(MemoryList*, MemoryNode*);
void listHeadInsert(MemoryList*, MemoryNode*);
void listDelete(MemoryList*, MemoryNode*);
void printList(MemoryList*, int);
void split(MemoryList*, MemoryNode*);

// Globals
MemoryList *freeMemoryNodes;

int MALLOC_TYPE = 0;
int MEM_SIZE = 0;
void* MEM_START;

void setup(int malloc_type, int mem_size, void* start_of_memory) {
    printf("[SETUP] malloc_type=%d, mem_size=%d, start_of_memory=%p\n", malloc_type, mem_size, start_of_memory);
    MALLOC_TYPE = malloc_type;
    MEM_SIZE = mem_size;
    MEM_START = start_of_memory;

    // All allocation policies will allocate one node of size mem_size at address 0
    freeMemoryNodes = malloc(sizeof(MemoryList));
    MemoryNode *newNode = malloc(sizeof(MemoryNode));
    newNode->size = mem_size;
    newNode->start_ptr = 0;
    newNode->used = 0;
    listTailInsert(freeMemoryNodes, newNode);
    printList(freeMemoryNodes, 0);
}

void *my_malloc(int size) {
    printf("[MY_MALLOC] size=%d\n", size);
    int fullSize = size + 4;
    switch (MALLOC_TYPE) {
        // For first fit, find the first hole that can fit requested size
        case FIRST_FIT: {
            MemoryNode *current = freeMemoryNodes->head;
            while (current != NULL) {
                // Need to check for size + 4, because we need to include the 4 byte header
                if (current->size >= fullSize && current->used == 0) {
                    // Check for exact fit
                    if (current->size == fullSize) {
                        current->used = 1;
                        return (char *) MEM_START + current->start_ptr + 4;
                    } else {
                        // If there is still memory remaining in the free node then
                        // Split the free node and add remaining memory as a node to the tail
                        // ex. request 2:
                        //      [===10===] -> [x2x][===8===] (=: free memory, x: used memory)
                        MemoryNode *remainingNode = malloc(sizeof(MemoryNode));
                        remainingNode->size = current->size - fullSize;
                        remainingNode->start_ptr = current->start_ptr + fullSize;
                        remainingNode->used = 0;
                        listTailInsert(freeMemoryNodes, remainingNode);

                        // Allocate the full size to the found node
                        current->size = fullSize;
                        current->used = 1;
                        printList(freeMemoryNodes, 0);
                        printf("\t[RETURNING my_malloc] %d\n", (int)(current->start_ptr + 4));

                        // C doesn't allow pointer arithmetic on (void *), so cast to (char *)
                        return (char *)MEM_START + current->start_ptr + 4;
                    }
                }
                current = current->next;
            }
            break;
        }
        case BEST_FIT: {
            MemoryNode *best = NULL;
            int hole = MEM_SIZE + 1;

            MemoryNode *current = freeMemoryNodes->head;

            while (current != NULL) {

                int temp_hole = current->size - fullSize;

                // Check for perfect fit
                if (temp_hole == 0 && current->used == 0) {
                    current->used = 1;
                    return (char *) MEM_START + current->start_ptr + 4;
                } else if ((temp_hole > 0) && (temp_hole < hole) && current->used == 0) {
                    hole = temp_hole;
                    best = current;
                }
                current = current->next;
            }
            if (best != NULL) {
                // If there is still memory remaining in the free node then
                // Split the free node and add remaining memory as a node to the tail
                MemoryNode *remainingNode = malloc(sizeof(MemoryNode));
                remainingNode->size = best->size - fullSize;
                remainingNode->start_ptr = best->start_ptr + fullSize;
                remainingNode->used = 0;
                listTailInsert(freeMemoryNodes, remainingNode);
                //Allocate the full size to the found node
                best->size = fullSize;
                best->used = 1;
                printList(freeMemoryNodes, 0);
                printf("\t[RETURNING my_malloc] %d\n", (int)(best->start_ptr + 4));
                // C doesn't allow pointer arithmetic on (void *), so cast to (char *)
                return (char *)MEM_START + best->start_ptr + 4;
            }
            break;
        }
        case WORST_FIT: {
            MemoryNode *worst = NULL;
            int hole = -1;

            MemoryNode *current = freeMemoryNodes->head;

            while (current != NULL) {

                int temp_hole = current->size - (size + 4);
                if((temp_hole > hole) && current->used == 0){

                    hole = temp_hole;
                    worst = current;
                }
                current = current->next;
            }
            if (worst != NULL){
                // If there is still memory remaining in the free node then
                // Split the free node and add remaining memory as a node to the tail
                MemoryNode *remainingNode = malloc(sizeof(MemoryNode));
                remainingNode->size = worst->size - fullSize;
                remainingNode->start_ptr = worst->start_ptr + fullSize;
                remainingNode->used = 0;
                listTailInsert(freeMemoryNodes, remainingNode);
                //Allocate the full size to the found node
                worst->size = fullSize;
                worst->used = 1;
                printList(freeMemoryNodes, 0);
                printf("\t[RETURNING my_malloc] %d\n", (int)(worst->start_ptr + 4));
                // C doesn't allow pointer arithmetic on (void *), so cast to (char *)
                return (char *)MEM_START + worst->start_ptr + 4;
            }
            break;
        }

        case BUDDY_SYSTEM: {
            // Default to an error
            void *rtrVal = (void*) -1;

            // Find smallest base 2 size that can fulfill the request
            // Once we find the smallest size, try to find a free node
            // with that exact size.
            int buddySize = 0;
            int power = 0;
            while (fullSize > buddySize) {
                buddySize = 1 << power;
                power++;
            }
            printf("\t[FIND BUDDY SIZE] buddySize=%d\n", buddySize);

            // Find free node that can fit buddy size exactly
            MemoryNode *freeNode = freeMemoryNodes->head;
            while (freeNode != NULL) {
                if (freeNode->size == buddySize && freeNode->used == 0) {
                    break;
                }
                freeNode = freeNode->next;
            }
            if (freeNode != NULL) {
                printf("\t[FOUND FREE NODE] size=%d, start=%d\n", freeNode->size, (int)freeNode->start_ptr);
                freeNode->used = 1;
                printList(freeMemoryNodes, 0);
                rtrVal = (char *)MEM_START + freeNode->start_ptr + 4;

                return rtrVal;
            } else {
                printf("\t[DID NOT FIND FREE NODE]\n");
                // Need to find a free node to split
                MemoryNode *currentNode = freeMemoryNodes->head;
                MemoryNode *smallestNode = NULL;
                int smallestSize = freeMemoryNodes->head->size;

                // Set initial smallest start_ptr to the head's start_ptr if list only contains 1 node,
                //      set initial value to 2nd node otherwise.
                // If smallest start_ptr is initially set to the head everytime, then when node 1 has
                // the same size as node 2, but node 1 is used, node 2's start_ptr is not <= 0 (start_ptr of node 1)
                // so smallestStartPtr is never set to node 2's start_ptr
                int smallestStartPtr = (freeMemoryNodes->size == 1) ? freeMemoryNodes->head->start_ptr : freeMemoryNodes->head->next->start_ptr;
                while (currentNode != NULL) {
                    if (currentNode->used == 0 && currentNode->size <= smallestSize && currentNode->start_ptr <= smallestStartPtr ) {
                        smallestNode = currentNode;
                        smallestSize = currentNode->size;
                        smallestStartPtr = currentNode->start_ptr;
                    }
                    currentNode = currentNode->next;
                }

                if (smallestNode != NULL) {
                    printf("\t[FOUND SMALLEST FREE NODE] size=%d, start=%d\n", smallestNode->size, (int)smallestNode->start_ptr);
                    printList(freeMemoryNodes, 0);

                    // Keep splitting smallest node until we create a node of size buddySize.
                    int currentSplitSize = smallestNode->size;
                    while (currentSplitSize != buddySize) {
                        currentSplitSize -= currentSplitSize / 2;
                        printf("\t\t[SPLITTING] currentSplitSize=%d\n", currentSplitSize);
                        MemoryNode *newSplit = malloc(sizeof(MemoryNode));
                        newSplit->start_ptr = smallestNode->start_ptr + currentSplitSize;
                        newSplit->size = currentSplitSize;
                        newSplit->used = 0;
                        smallestNode->size = currentSplitSize;

                        if (smallestNode == freeMemoryNodes->tail) {
                            printf("\t\t[SPLITTIG TAIL]\n");
                            newSplit->next = NULL;
                            newSplit->prev = smallestNode;
                            smallestNode->next = newSplit;
                            freeMemoryNodes->tail = newSplit;
                        } else {
                            printf("\t\t[NOT SPLITTING TAIL]\n");
                            newSplit->next = smallestNode->next;
                            newSplit->prev = smallestNode;
                            smallestNode->next = newSplit;
                        }

                        freeMemoryNodes->size++;
                        rtrVal = (char *)MEM_START + smallestNode->start_ptr + 4;

                        if (smallestNode->size == buddySize) {
                            smallestNode->used = 1;
                            break;
                        }
                    }
                    printList(freeMemoryNodes, 0);

                    printf("\t[RETURNING my_malloc] %d\n", (int)(freeMemoryNodes->head->start_ptr + 4));

                    return rtrVal;

                } else {
                    printf("\t[DID NOT FIND SMALLEST FREE NODE]\n");
                    // Check if tail can be split
                    if (freeMemoryNodes->tail->used == 0 && freeMemoryNodes->tail->size >= buddySize) {
                        printf("\t[CAN SPLIT TAIL]\n");
                        int currentSplitSize = freeMemoryNodes->tail->size;
                        printf("\t");
                        int test = 0;
                        while (currentSplitSize != buddySize) {
                            currentSplitSize -= currentSplitSize / 2;
                            printf("\t\t[SPLITTING] currentSplitSize=%d\n", currentSplitSize);

                            freeMemoryNodes->tail->size = currentSplitSize;
                            if (freeMemoryNodes->tail->size == buddySize) {
                                freeMemoryNodes->tail->used = 1;
                            }
                            MemoryNode *newSplit = malloc(sizeof(MemoryNode));
                            newSplit->start_ptr = freeMemoryNodes->tail->start_ptr + currentSplitSize;
                            newSplit->size = currentSplitSize;
                            newSplit->used = 0;

                            listTailInsert(freeMemoryNodes, newSplit);
                            printList(freeMemoryNodes, 0);
                            test = newSplit->prev->start_ptr + 4;
                            rtrVal = (char *) MEM_START + newSplit->prev->start_ptr + 4;
                        }

                        printf("\t[RETURNING my_malloc] %d\n", test);

                    }

                    return rtrVal;
                }
            }
            break;
        }
    }
}

void split(MemoryList* list, MemoryNode* node) {

}

void my_free(void *ptr) {

    int start = (char*) ptr - (char*) MEM_START - 4;

    printf("[MY_FREE] ptr=%d\n", start);
    switch (MALLOC_TYPE) {
        case FIRST_FIT:
        case BEST_FIT:
        case WORST_FIT: {
            MemoryNode *current = freeMemoryNodes->head;
            while (current != NULL) {
                if (current->start_ptr == start && current->used == 1) {
                    printf("\t[FOUND NODE] start=%d size=%d\n", start, current->size);
                    // Look at adjacent nodes to see if there are holes we can merge all nodes into current node.
                    if (current->next->used == 0 || current->prev->used == 0) {
                        // If both nodes are free, merge all 3
                        if (current->next->used == 0 && current->next->used == 0) {
                            printf("\t\t[MERGING BOTH NODES]\n");
                            current->start_ptr = current->prev->start_ptr;
                            current->size = current->prev->size + current->next->size + current->size;
                            current->prev = current->prev->prev;
                            current->next = current->next->next;
                            freeMemoryNodes->size -= 2;
                        }
                        // If only right node is free
                        else if (current->next->used == 0) {
                            printf("\t\t[MERGING RIGHT NODE]\n");
                            current->size = current->next->size + current->size;
                            current->next = current->next->next;
                            freeMemoryNodes->size--;
                        }
                        // If only left node is free
                        else if (current->prev->used == 0) {
                            printf("\t\t[MERGING LEFT NODE]\n");
                            current->start_ptr = current->prev->start_ptr;
                            current->size = current->prev->size + current->size;
                            current->prev = current->prev->prev;
                            freeMemoryNodes->size--;
                        }
                        current->used = 0;

                    } else {
                        current->used = 0;
                    }
                    printList(freeMemoryNodes, 0);

                    return;
                }
                current = current->next;
            }
            break;
        }

        case BUDDY_SYSTEM: {
            // Find the memory location in the list that needs to be freed
            printf("\t[FIND start_ptr] %d\n", start);
            MemoryNode *currentNode = freeMemoryNodes->head;
            MemoryNode *nodeToFree = NULL;
            while (currentNode != NULL) {
                if (currentNode->start_ptr == start && currentNode->used == 1) {
                    nodeToFree = currentNode;
                    break;
                }
                currentNode = currentNode->next;
            }

            if (nodeToFree != NULL) {
                printf("\t[FOUND NODE TO FREE]\n");
                printf("\t\t[start_ptr=%d size=%d used=%d]\n", nodeToFree->start_ptr, nodeToFree->size, nodeToFree->used);
                nodeToFree->used = 0;
                printList(freeMemoryNodes, 0);
                // Iterate through the list to find nodes that can be merged
                // Look at the right node, if it has the same size and is unused
                // then it can be merged into the current node. Everytime we merge,
                // go back to the beginning of the list and check again
                currentNode = freeMemoryNodes->head;
                while (currentNode != NULL) {
                    if (currentNode->next != NULL && currentNode->next->size == currentNode->size && currentNode->next->used == 0 && currentNode->used == 0) {
                        printf("\t[MERGING]\n");
                        printf("\t\t[start_ptr=%d size=%d used=%d]\n", currentNode->start_ptr, currentNode->size, currentNode->used);
                        currentNode->size = currentNode->size + currentNode->next->size;
                        currentNode->next = currentNode->next->next;

                        printList(freeMemoryNodes, 0);
                        currentNode = freeMemoryNodes->head;
                        freeMemoryNodes->size--;
                     } else {
                         currentNode = currentNode->next;
                     }
                }
            }

            printf("[FINISH MY_FREE]\n");
            break;
        }
    }

}

void listTailInsert(MemoryList *list, MemoryNode *node) {
    printf("\t[INSERT NODE] [s=%d, ptr=%d]\n", node->size, (int)node->start_ptr);
    node->next = NULL;
    if (list->head == NULL) {
        node->prev = NULL;
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }

    list->size++;
}

void listHeadInsert(MemoryList *list, MemoryNode *node) {
    printf("\t[INSERT NODE] [s=%d, ptr=%d]\n", node->size, (int)node->start_ptr);
    node->next = NULL;
    node->prev = NULL;
    if (list->head == NULL) {
        node->prev = NULL;
        list->head = node;
        list->tail = node;
    } else {
        list->head->prev = node;
        node->next = list->head;
        list->head = node;
    }

    list->size++;
}

void listDelete(MemoryList *list, MemoryNode *node) {
    printf("\t[DELETING NODE] p=%d s=%d n=%d\n", (node->prev != NULL) ? node->prev->size : -1, node->size, (node->next != NULL) ? node->next->size : -1);
    if (node == list->head) {
        node->next->prev = NULL;
        list->head = node->next;
    } else if (node == list->tail) {
        node->prev->next = NULL;
        list->tail = node->prev;
    } else {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }

    free(node);
    list->size--;
}

void printList(MemoryList *list, int showNextPrev) {
    MemoryNode *node = list->head;
    printf("\t[LIST s=%d] \n", list->size);
    while (node != NULL) {
        if (showNextPrev == 1) {
            printf("\t\t[p=%d, %d, n=%d]\n", (node->prev != NULL) ? node->prev->size : -1, node->size, (node->next != NULL) ? node->next->size : -1);
        } else {
            printf("\t\t[size=%d, ptr=%d, used=%d]\n", node->size, node->start_ptr, node->used);
        }
        node = node->next;
    }
    printf("\n");
}