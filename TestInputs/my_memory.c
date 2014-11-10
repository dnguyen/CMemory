#include <stdio.h>
#include <stdlib.h>

#define FIRST_FIT 0
#define BEST_FIT 1
#define WORST_FIT 2
#define BUDDY_SYSTEM 3

const int MAX_MEM_SIZE = 1 << 20;
const int MIN_MALLOC_SIZE = 1 << 10;

// Data Structures
//  Doubly Linked list to keep track of memory
typedef struct MemoryNode MemoryNode;
struct MemoryNode {
    //[size..4byte_header][start...size], totalsize=size+4
    // size will always point to start, when returuning a pointer,
    // just return start + 4
    void* start;
    int* size;
    MemoryNode *prev;
    MemoryNode *next;
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
MemoryNode* createNewNode(void*, int);
MemoryNode* split(MemoryList*, MemoryNode*, int);
void merge(MemoryList*, MemoryNode*);
MemoryNode* findFirstfit(MemoryList*, int);
MemoryNode* findBestFit(MemoryList*, int);
MemoryNode* findWorstFit(MemoryList*, int);

// Globals
MemoryList *memoryNodes;

int MALLOC_TYPE = 0;
int MEM_SIZE = 0;
void* MEM_START;

void setup(int malloc_type, int mem_size, void* start_of_memory) {

    if (mem_size > MAX_MEM_SIZE) {
        printf("Maximum requested memory.");
        return;
    }

    printf("[SETUP] malloc_type=%d, mem_size=%d, start_of_memory=%p\n", malloc_type, mem_size, start_of_memory);

    MALLOC_TYPE = malloc_type;
    MEM_SIZE = mem_size;
    MEM_START = start_of_memory;

    // All allocation policies will allocate one node of size mem_size at address MEM_START
    memoryNodes = malloc(sizeof(MemoryList));
    MemoryNode *newNode = createNewNode(MEM_START, MEM_SIZE);
    listTailInsert(memoryNodes, newNode);
    printList(memoryNodes, 0);

}

void *my_malloc(int size) {

    if (size < MIN_MALLOC_SIZE) {
        return (void*) -1;
    }

    printf("[MY_MALLOC] size=%d\n", size);
    int fullSize = size + 4;
    switch (MALLOC_TYPE) {
        case FIRST_FIT: {
            MemoryNode* freeNode = findFirstfit(memoryNodes, fullSize);
            if (freeNode != NULL) {
                return freeNode->start + 4;
            } else {
                return (void*)-1;
            }
            break;
        }

        case BEST_FIT: {
            MemoryNode* freeNode = findBestFit(memoryNodes, fullSize);
            if (freeNode != NULL) {
                return freeNode->start + 4;
            } else {
                return (void*)-1;
            }

            break;
        }

        case WORST_FIT: {
            MemoryNode* freeNode = findWorstFit(memoryNodes, fullSize);
            if (freeNode != NULL) {
                return freeNode->start + 4;
            } else {
                return (void*)-1;
            }
            break;
        }

        case BUDDY_SYSTEM: {
            printf("[BUDDY SYSTEM MALLOC]\n");
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
            MemoryNode *freeNode = memoryNodes->head;
            while (freeNode != NULL) {
                if (*freeNode->size == buddySize && freeNode->used == 0) {
                    break;
                }
                freeNode = freeNode->next;
            }

            // If a free node of exact size was found, complete the request
            // Else we need to find the smallest possible node that can fulfill the request
            if (freeNode != NULL) {
                freeNode->used = 1;
                printList(memoryNodes, 0);
                rtrVal = freeNode->start + 4;

                return rtrVal;
            } else {
                printf("\t[DID NOT FIND FREE NODE]\n");
                // Need to find a free node to split
                MemoryNode *currentNode = memoryNodes->head;
                MemoryNode *smallestNode = NULL;
                int smallestSize = *memoryNodes->head->size;

                // Set initial smallest start pointer to the head's start pointer if list only contains 1 node,
                //      set initial value to 2nd node otherwise.
                // If smallest start pointer is initially set to the head everytime, then when node 1 has
                // the same size as node 2, but node 1 is used, node 2's start pointer is not <= 0 (start pointer of node 1)
                // so smallestStartPtr is never set to node 2's start pointer
                void* smallestStartPtr = (memoryNodes->size == 1) ? memoryNodes->head->start : memoryNodes->head->next->start;

                // Find the smallest free node with the smallest start pointer
                while (currentNode != NULL) {
                    if (currentNode->used == 0 && *currentNode->size <= smallestSize && currentNode->start <= smallestStartPtr) {
                        smallestNode = currentNode;
                        smallestSize = *currentNode->size;
                        smallestStartPtr = currentNode->start;
                    }
                    currentNode = currentNode->next;
                }

                if (smallestNode != NULL) {
                    printf("\t[FOUND SMALLEST FREE NODE] start=%p size=%d\n",  smallestNode->start, *smallestNode->size);
                    printList(memoryNodes, 0);

                    // Keep splitting smallest node until a node of size buddySize is created.
                    int currentSplitSize = *smallestNode->size;
                    while (currentSplitSize != buddySize) {
                        currentSplitSize -= currentSplitSize / 2;
                        *smallestNode->size = currentSplitSize;
                        MemoryNode *newSplit = createNewNode(smallestNode->start + currentSplitSize, currentSplitSize);

                        // Splitting the tail has to be handled differently
                        if (smallestNode == memoryNodes->tail) {
                            newSplit->next = NULL;
                            newSplit->prev = smallestNode;
                            smallestNode->next = newSplit;
                            memoryNodes->tail = newSplit;
                        } else {
                            newSplit->next = smallestNode->next;
                            newSplit->prev = smallestNode;
                            smallestNode->next = newSplit;
                        }

                        memoryNodes->size++;

                        if (*smallestNode->size == buddySize) {
                            smallestNode->used = 1;
                            rtrVal = smallestNode->start + 4;
                            break;
                        }
                    }
                    printList(memoryNodes, 0);

                    return rtrVal;

                } else {
                    // Check if tail can be split
                    if (memoryNodes->tail->used == 0 && *memoryNodes->tail->size >= buddySize) {
                        printf("\t[CAN SPLIT TAIL]\n");
                        int currentSplitSize = *memoryNodes->tail->size;

                        while (currentSplitSize != buddySize) {
                            currentSplitSize -= currentSplitSize / 2;
                            printf("\t\t[SPLITTING] currentSplitSize=%d\n", currentSplitSize);

                            *memoryNodes->tail->size = currentSplitSize;
                            if (*memoryNodes->tail->size == buddySize) {
                                memoryNodes->tail->used = 1;
                            }
                            MemoryNode *newSplit = createNewNode(memoryNodes->tail->start + currentSplitSize, currentSplitSize);
                            listTailInsert(memoryNodes, newSplit);
                            printList(memoryNodes, 0);

                            rtrVal = newSplit->prev->start + 4;
                        }
                    } else {
                        rtrVal = (void*) -1;
                    }

                    return rtrVal;
                }
            }
            break;
        }
    }
}

void my_free(void *ptr) {
    // Check for invalid pointers
    // Any frees should always be within our given memory space: [MEM_START..MEM_START+MAX_MEM_SIZE]
    if (ptr == NULL || ptr < MEM_START || ptr > MEM_START + MAX_MEM_SIZE) {
        printf("[Trying to free an invalid pointer]\n");
        return;
    }

    void* start = (ptr - 4);
    int size = *((int*)(ptr-4));
    printf("[MY_FREE] ptr=%p sizeaddr=%p size=%d\n", ptr, ptr-4, size);

    switch (MALLOC_TYPE) {
        case FIRST_FIT:
        case BEST_FIT:
        case WORST_FIT: {
            // If there is only 1 hole in the list and is being used, free it.
            if (memoryNodes->size <= 1) {
                if (memoryNodes->head->used == 1) {
                    memoryNodes->head->used = 0;
                }
            } else {
                // Merge any adjacent free nodes.
                MemoryNode *current = memoryNodes->head;
                while (current != NULL) {
                    if (current->start == start && current->used == 1) {
                        printf("\t[FOUND NODE] start=%p size=%d\n", start, size);
                        merge(memoryNodes, current);
                        printList(memoryNodes, 0);

                        return;
                    }
                    current = current->next;
                }
            }
            break;
        }

        case BUDDY_SYSTEM: {

            // Find the memory location in the list that needs to be freed
            MemoryNode *currentNode = memoryNodes->head;
            MemoryNode *nodeToFree = NULL;
            while (currentNode != NULL) {
                if (currentNode->start == start && currentNode->used == 1) {
                    nodeToFree = currentNode;
                    break;
                }
                currentNode = currentNode->next;
            }

            if (nodeToFree != NULL) {
                nodeToFree->used = 0;
                printList(memoryNodes, 0);
                // Iterate through the list to find nodes that can be merged
                // Look at the right node, if it has the same size and is unused
                // then it can be merged into the current node. Everytime we merge,
                // go back to the beginning of the list and check again
                currentNode = memoryNodes->head;
                while (currentNode != NULL) {
                    if (currentNode->next != NULL && *currentNode->next->size == *currentNode->size && currentNode->next->used == 0 && currentNode->used == 0) {
                        *currentNode->size += *currentNode->next->size;
                        currentNode->next = currentNode->next->next;

                        printList(memoryNodes, 0);
                        currentNode = memoryNodes->head;
                        memoryNodes->size--;
                     } else {
                         currentNode = currentNode->next;
                     }
                }
            }

            break;
        }
    }

}

MemoryNode* findFirstfit(MemoryList* list, int size) {
    MemoryNode *current = memoryNodes->head;
    while (current != NULL) {
        int currentSize = *(current->size);
        if (currentSize + 4 >= size && current->used == 0) {
            if (currentSize + 4 > size) {
                split(memoryNodes, current, size);
            }

            current->used = 1;
            return current;
        }

        current = current->next;
    }

    return NULL;
}

MemoryNode* findBestFit(MemoryList* list, int size) {
    MemoryNode *best = NULL;
    int hole = MEM_SIZE + 1;

    MemoryNode *current = memoryNodes->head;

    while (current != NULL) {

        int temp_hole = *current->size - size;

        // Check for perfect fit
        if (temp_hole == 0 && current->used == 0) {
            current->used = 1;
            return current;
        } else if ((temp_hole > 0) && (temp_hole < hole) && current->used == 0) {
            hole = temp_hole;
            best = current;
        }
        current = current->next;
    }
    if (best != NULL) {
        split(memoryNodes, best, size);
        best->used = 1;

        return best;
    } else {
        return NULL;
    }
}

MemoryNode* findWorstFit(MemoryList* list, int size) {
    MemoryNode *worst = NULL;
    int hole = -1;

    MemoryNode *current = memoryNodes->head;

    while (current != NULL) {

        int temp_hole = *current->size - size;
        if((temp_hole > hole) && current->used == 0){

            hole = temp_hole;
            worst = current;
        }
        current = current->next;
    }
    if (worst != NULL){
        split(memoryNodes, worst, size);
        worst->used = 1;

        return worst;
    } else {
        return NULL;
    }

}

// Creates and initializes a new memory node
MemoryNode* createNewNode(void* start, int size) {
    MemoryNode *newNode = malloc(sizeof(MemoryNode));
    newNode->start = start;
    newNode->size = start;
    *newNode->size = size;
    newNode->used = 0;

    return newNode;
}

// Splits a node into a given size.
// node's size = size, and a new node is created that is the size of
// node's original size - size
MemoryNode* split(MemoryList* list, MemoryNode* node, int size) {
    MemoryNode *newNode = createNewNode(node->start + size, *node->size - size);
    listTailInsert(memoryNodes, newNode);

    *(node->size) = size;
    node->used = 0;

    return node;
}

// Merge any adjacentfree nodes adjacent to node
void merge(MemoryList* list, MemoryNode* node) {
    // Look at adjacent nodes to see if there are holes we can merge all nodes into node node.
    if (node->next->used == 0 || node->prev->used == 0) {
        // If both nodes are free, merge all 3
        if (node->next->used == 0 && node->next->used == 0) {
            printf("\t\t[MERGING BOTH NODES]\n");

            node->start = node->prev->start;
            node->size = node->prev->start;
            *(node->size) = *(node->prev->size) + *(node->next->size) + *(node->size);

            node->prev = node->prev->prev;
            node->next = node->next->next;
            memoryNodes->size -= 2;
        }
        // If only right node is free
        else if (node->next->used == 0) {
            printf("\t\t[MERGING RIGHT NODE]\n");
            *(node->size) = *(node->next->size) + *(node->size);
            node->next = node->next->next;
            memoryNodes->size--;
        }
        // If only left node is free
        else if (node->prev->used == 0) {
            printf("\t\t[MERGING LEFT NODE]\n");

            node->start = node->prev->start;
            node->size = node->prev->start;
            *node->size = *node->prev->size + *node->size;

            node->prev = node->prev->prev;
            memoryNodes->size--;
        }
        node->used = 0;

    } else {
        node->used = 0;
    }
}

void listTailInsert(MemoryList *list, MemoryNode *node) {
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
        printf("\t\t[node_addr=%p start=%p sizeptr=%p size=%d, used=%d]\n", node, node->start, node->size, *node->size, node->used);
        node = node->next;
    }
    printf("\n");
}