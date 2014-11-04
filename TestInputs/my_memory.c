#include <stdio.h>
#include <stdlib.h>

#define FIRST_FIT 0
#define BEST_FIT 1
#define WORST_FIT 2
#define BUDDY_SYSTEM 3

// Data Structures
typedef struct MemoryNode MemoryNode;
struct MemoryNode {
    MemoryNode *prev;
    MemoryNode *next;
    int start_ptr;
    int size;
    int used;
};

typedef struct MemoryList MemoryList;
typedef struct MemoryList {
    MemoryNode *head;
    MemoryNode *tail;
    int size;
};

// Function prototypes
void listTailInsert(MemoryList*, MemoryNode*);
void listHeadInsert(MemoryList*, MemoryNode*);
void listDelete(MemoryList*, MemoryNode*);
void printList(MemoryList*, int);
void split(MemoryList*, int , int);

// doubly Linked list to keep track of holes
MemoryList *freeMemoryNodes;

int MALLOC_TYPE = 0;
int MEM_SIZE = 0;
void* MEM_START;

void setup(int malloc_type, int mem_size, void* start_of_memory) {
    printf("[SETUP] malloc_type=%d, mem_size=%d, start_of_memory=%p\n", malloc_type, mem_size, start_of_memory);
    MALLOC_TYPE = malloc_type;
    MEM_SIZE = mem_size;
    MEM_START = start_of_memory;

    MemoryNode *newNode = malloc(sizeof(MemoryNode));

    switch (MALLOC_TYPE) {
        case FIRST_FIT:
            // Create a hole that is the size of the entire memory initially
            freeMemoryNodes = malloc(sizeof(MemoryList));
            newNode->size = mem_size;
            newNode->start_ptr = 0;
            newNode->used = 0;
            listTailInsert(freeMemoryNodes, newNode);
            printList(freeMemoryNodes, 0);
            break;

        case BEST_FIT:
            // Create a hole that is the size of the entire memory initially
            freeMemoryNodes = malloc(sizeof(MemoryList));
            newNode->size = mem_size;
            newNode->start_ptr = 0;
            newNode->used = 0;
            listTailInsert(freeMemoryNodes, newNode);
            printList(freeMemoryNodes, 0);
            break;

        case WORST_FIT:
            // Create a hole that is the size of the entire memory initially
            freeMemoryNodes = malloc(sizeof(MemoryList));
            newNode->size = mem_size;
            newNode->start_ptr = 0;
            newNode->used = 0;
            listTailInsert(freeMemoryNodes, newNode);
            printList(freeMemoryNodes, 0);
            break;

        case BUDDY_SYSTEM:
            freeMemoryNodes = malloc(sizeof(MemoryList));
            newNode->size = mem_size;
            newNode->start_ptr = 0;
            newNode->used = 0;
            listTailInsert(freeMemoryNodes, newNode);
            printList(freeMemoryNodes, 0);
            break;
    }
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
                if(temp_hole == 0 && current->used == 0){
                    current->used = 1;
                    return (char *) MEM_START + current->start_ptr + 4;
                } else if ((temp_hole > 0) && (temp_hole < hole) && current->used == 0){
                    hole = temp_hole;
                    best = current;
                }
                current = current->next;
            }
            if(best != NULL){
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
            if(worst != NULL){
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
            // Find smallest base 2 size that can accomdate the request
            // Once we find the smallest size, try to find a free node
            // with that exact size. If there is no node with that size
            // keep splitting the
            void *rtrVal = (void*)-1;
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
                rtrVal = (char *)MEM_START + freeNode->start_ptr;
                return rtrVal;
            } else {
                printf("\t[DID NOT FIND FREE NODE]\n");
                // Need to find a free node to split
                MemoryNode *currentNode = freeMemoryNodes->head;
                MemoryNode *smallestNode = NULL;
                int smallestSize = freeMemoryNodes->head->size;
                while (currentNode != NULL) {
                    if (currentNode->used == 0 && currentNode->size <= smallestSize) {
                        smallestNode = currentNode;
                        smallestSize = currentNode->size;
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

                        // Check if the node we're splitting is the head
                        if (smallestNode == freeMemoryNodes->head) {
                            printf("\t\t[SPLITTING HEAD OF LIST]\n");
                            MemoryNode *newSplit = malloc(sizeof(MemoryNode));
                            newSplit->start_ptr = smallestNode->start_ptr + currentSplitSize + 4;
                            newSplit->size = currentSplitSize;
                            newSplit->used = 0;
                            newSplit->next = smallestNode->next;
                            if (smallestNode == freeMemoryNodes->tail) {
                                freeMemoryNodes->tail = newSplit;
                            }
                            smallestNode->next = newSplit;
                            smallestNode->size = currentSplitSize;

                            // allocate request to smallest node once we've reached the maximum split
                            if (smallestNode->size == buddySize) {
                                smallestNode->used = 1;
                            }
                            freeMemoryNodes->head = smallestNode;
                            freeMemoryNodes->size++;
                            printList(freeMemoryNodes, 0);

                            rtrVal = (char *)MEM_START + freeMemoryNodes->head->start_ptr +4;
                        } else {
                            printf("\t\t[NOT SPLITTING HEAD]\n");
                            break;
                        }

                    }

                    printf("\t[RETURNING my_malloc] %d\n", (int)(freeMemoryNodes->head->start_ptr + 4));
                    return rtrVal;

                } else {
                    printf("\t[DID NOT FIND SMALLEST FREE NODE]\n");
                    // Check if tail can be split
                    if (freeMemoryNodes->tail->used == 0 && freeMemoryNodes->tail->size >= buddySize) {
                        printf("\t[CAN SPLIT TAIL]\n");
                        int currentSplitSize = freeMemoryNodes->tail->size;
                        printf("\t");
                        while (currentSplitSize != buddySize) {
                            currentSplitSize -= currentSplitSize / 2;
                            printf("\t\t[SPLITTING] currentSplitSize=%d\n", currentSplitSize);

                            freeMemoryNodes->tail->size = currentSplitSize;
                            if (freeMemoryNodes->tail->size == buddySize) {
                                freeMemoryNodes->tail->used = 1;
                            }
                            MemoryNode *newSplit = malloc(sizeof(MemoryNode));
                            newSplit->start_ptr = freeMemoryNodes->tail->start_ptr + currentSplitSize + 4;
                            newSplit->size = currentSplitSize;
                            newSplit->used = 0;

                            listTailInsert(freeMemoryNodes, newSplit);
                            printList(freeMemoryNodes, 0);

                            rtrVal = (char *) MEM_START + freeMemoryNodes->tail->start_ptr + 4;
                        }

                        printf("\t[RETURNING my_malloc] %d\n", (int)(freeMemoryNodes->tail->start_ptr + 4));
                        return rtrVal;
                    } else {
                        printf("\t[CAN'T SPLIT TAIL]\n");
                    }
                }
            }
            break;
        }
    }
}

void my_free(void *ptr) {
    // Calculate int pointer value with some fancy pointer arithmetic...
    int start = (char*) ptr - (char*) MEM_START - 4;
    printf("[MY_FREE] ptr=%lu\n", start);
    printf("\tMEM_START=%lu ptr=%lu\n", MEM_START, ptr);
    switch (MALLOC_TYPE) {
        case FIRST_FIT: {
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
        case BEST_FIT:{
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
    printf("\t[LIST s=%d] ", list->size);
    while (node != NULL) {
        if (showNextPrev == 1) {
            printf(" [p=%d, %d, n=%d] ", (node->prev != NULL) ? node->prev->size : -1, node->size, (node->next != NULL) ? node->next->size : -1);
        } else {
            printf(" [size=%d, ptr=%d, used=%d] ", node->size, (int *)node->start_ptr, node->used);
        }
        node = node->next;
    }
    printf("\n");
}

