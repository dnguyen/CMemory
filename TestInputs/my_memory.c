#include "assert.h"
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
    //[size..4byte_size_header][start...size], totalsize=size+4
    void* start;
    int* size_;
    int start_ptr;
    int size;
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
MemoryNode* split(MemoryList*, MemoryNode*, int);

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

    // All allocation policies will allocate one node of size mem_size at address 0
    memoryNodes = malloc(sizeof(MemoryList));
    MemoryNode *newNode = malloc(sizeof(MemoryNode));
    newNode->start_ptr = 0;

    newNode->start = MEM_START;
    newNode->size_ = (int *) MEM_START;
    *(newNode->size_) = MEM_SIZE;

    newNode->size = mem_size;
    newNode->used = 0;
    listTailInsert(memoryNodes, newNode);
    printList(memoryNodes, 0);
    int sizePrint = *(newNode->size_);
    printf("[First Node] start=%p size_addr=%p size=%d\n", newNode->start, newNode->size_, sizePrint);
}

void *my_malloc(int size) {

    if (size < MIN_MALLOC_SIZE) {
        return (void*) -1;
    }

    printf("[MY_MALLOC] size=%d\n", size);
    int fullSize = size + 4;
    switch (MALLOC_TYPE) {
        // For first fit, find the first hole that can fit requested size
        case FIRST_FIT: {
            MemoryNode *current = memoryNodes->head;
            while (current != NULL) {
                int currentSize = *(current->size_);

                if (currentSize == fullSize && current->used == 0) {
                    current->used = 1;

                    return current->start + 4;

                } else if (currentSize > fullSize && current->used == 0) {
                    split(memoryNodes, current, fullSize);
                    current->used = 1;

                    return current->start + 4;
                } else {
                    printf("[shouldn't happen]\n");
                }

                current = current->next;
            }
            break;
        }

        case BEST_FIT: {
            MemoryNode *best = NULL;
            int hole = MEM_SIZE + 1;

            MemoryNode *current = memoryNodes->head;

            while (current != NULL) {

                int temp_hole = *current->size_ - fullSize;

                // Check for perfect fit
                if (temp_hole == 0 && current->used == 0) {
                    current->used = 1;
                    return current->start + 4;
                } else if ((temp_hole > 0) && (temp_hole < hole) && current->used == 0) {
                    hole = temp_hole;
                    best = current;
                }
                current = current->next;
            }
            if (best != NULL) {
                split(memoryNodes, best, fullSize);
                best->used = 1;

                return best->start + 4;
            }
            break;
        }
        case WORST_FIT: {
            MemoryNode *worst = NULL;
            int hole = -1;

            MemoryNode *current = memoryNodes->head;

            while (current != NULL) {

                int temp_hole = *current->size_ - (size + 4);
                if((temp_hole > hole) && current->used == 0){

                    hole = temp_hole;
                    worst = current;
                }
                current = current->next;
            }
            if (worst != NULL){
                split(memoryNodes, worst, fullSize);
                worst->used = 1;

                return worst->start + 4;
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
            MemoryNode *freeNode = memoryNodes->head;
            while (freeNode != NULL) {
                if (freeNode->size == buddySize && freeNode->used == 0) {
                    break;
                }
                freeNode = freeNode->next;
            }

            // If a free node of exact size was found, complete the request
            // Else we need to find the smallest possible node that can fulfill the request
            if (freeNode != NULL) {
                printf("\t[FOUND FREE NODE] size=%d, start=%d\n", freeNode->size, (int)freeNode->start_ptr);
                freeNode->used = 1;
                printList(memoryNodes, 0);
                rtrVal = (char *)MEM_START + freeNode->start_ptr + 4;

                return rtrVal;
            } else {
                printf("\t[DID NOT FIND FREE NODE]\n");
                // Need to find a free node to split
                MemoryNode *currentNode = memoryNodes->head;
                MemoryNode *smallestNode = NULL;
                int smallestSize = memoryNodes->head->size;

                // Set initial smallest start_ptr to the head's start_ptr if list only contains 1 node,
                //      set initial value to 2nd node otherwise.
                // If smallest start_ptr is initially set to the head everytime, then when node 1 has
                // the same size as node 2, but node 1 is used, node 2's start_ptr is not <= 0 (start_ptr of node 1)
                // so smallestStartPtr is never set to node 2's start_ptr
                int smallestStartPtr = (memoryNodes->size == 1) ? memoryNodes->head->start_ptr : memoryNodes->head->next->start_ptr;

                // Find the smallest free node with the smallest start_ptr
                while (currentNode != NULL) {
                    if (currentNode == memoryNodes->tail)
                        printf("[Checking if currentNode == tail]\n");
                    if (currentNode->used == 0 && currentNode->size <= smallestSize && currentNode->start_ptr <= smallestStartPtr ) {
                        smallestNode = currentNode;
                        smallestSize = currentNode->size;
                        smallestStartPtr = currentNode->start_ptr;
                    }
                    currentNode = currentNode->next;
                }

                if (smallestNode != NULL) {
                    printf("\t[FOUND SMALLEST FREE NODE] size=%d, start=%d\n", smallestNode->size, (int)smallestNode->start_ptr);
                    printList(memoryNodes, 0);

                    // Keep splitting smallest node until we create a node of size buddySize.
                    int currentSplitSize = smallestNode->size;
                    while (currentSplitSize != buddySize) {
                        currentSplitSize -= currentSplitSize / 2;
                        MemoryNode *newSplit = malloc(sizeof(MemoryNode));
                        newSplit->start_ptr = smallestNode->start_ptr + currentSplitSize;
                        newSplit->size = currentSplitSize;
                        newSplit->used = 0;
                        smallestNode->size = currentSplitSize;

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
                        rtrVal = (char *)MEM_START + smallestNode->start_ptr + 4;

                        if (smallestNode->size == buddySize) {
                            smallestNode->used = 1;
                            break;
                        }
                    }
                    printList(memoryNodes, 0);

                    printf("\t[RETURNING my_malloc] %d\n", (int)(memoryNodes->head->start_ptr + 4));

                    return rtrVal;

                } else {
                    // Check if tail can be split
                    if (memoryNodes->tail->used == 0 && memoryNodes->tail->size >= buddySize) {
                        printf("\t[CAN SPLIT TAIL]\n");
                        int currentSplitSize = memoryNodes->tail->size;

                        while (currentSplitSize != buddySize) {
                            currentSplitSize -= currentSplitSize / 2;
                            printf("\t\t[SPLITTING] currentSplitSize=%d\n", currentSplitSize);

                            memoryNodes->tail->size = currentSplitSize;
                            if (memoryNodes->tail->size == buddySize) {
                                memoryNodes->tail->used = 1;
                            }
                            MemoryNode *newSplit = malloc(sizeof(MemoryNode));
                            newSplit->start_ptr = memoryNodes->tail->start_ptr + currentSplitSize;
                            newSplit->size = currentSplitSize;
                            newSplit->used = 0;

                            listTailInsert(memoryNodes, newSplit);
                            printList(memoryNodes, 0);

                            rtrVal = (char *) MEM_START + newSplit->prev->start_ptr + 4;
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

    void* start = (ptr - 4);
    int size = *((int*)(ptr-4));
    printf("[MY_FREE] ptr=%p size_addr=%p size=%d\n", ptr, ptr-4, size);

    switch (MALLOC_TYPE) {
        case FIRST_FIT:
        case BEST_FIT:
        case WORST_FIT: {
            MemoryNode *current = memoryNodes->head;
            while (current != NULL) {
                if (current->start == start && current->used == 1) {
                    printf("\t[FOUND NODE] start=%d size=%d\n", start, size);
                    // Look at adjacent nodes to see if there are holes we can merge all nodes into current node.
                    if (current->next->used == 0 || current->prev->used == 0) {
                        // If both nodes are free, merge all 3
                        if (current->next->used == 0 && current->next->used == 0) {
                            printf("\t\t[MERGING BOTH NODES]\n");

                            current->start = current->prev->start;
                            current->size_ = current->prev->start;
                            *(current->size_) = *(current->prev->size_) + *(current->next->size_) + *(current->size_);

                            current->prev = current->prev->prev;
                            current->next = current->next->next;
                            memoryNodes->size -= 2;
                        }
                        // If only right node is free
                        else if (current->next->used == 0) {
                            printf("\t\t[MERGING RIGHT NODE]\n");
                            *(current->size_) = *(current->next->size_) + *(current->size_);
                            current->next = current->next->next;
                            memoryNodes->size--;
                        }
                        // If only left node is free
                        else if (current->prev->used == 0) {
                            printf("\t\t[MERGING LEFT NODE]\n");

                            current->start = current->prev->start;
                            current->size_ = current->prev->start;
                            *current->size_ = *current->prev->size_ + *current->size_;

                            current->prev = current->prev->prev;
                            memoryNodes->size--;
                        }
                        current->used = 0;

                    } else {
                        current->used = 0;
                    }
                    printList(memoryNodes, 0);

                    return;
                }
                current = current->next;
            }
            break;
        }

        case BUDDY_SYSTEM: {
            int start = (char*) ptr - (char*) MEM_START - 4;
            // Find the memory location in the list that needs to be freed
            printf("\t[FIND start_ptr] %d\n", start);
            MemoryNode *currentNode = memoryNodes->head;
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
                printList(memoryNodes, 0);
                // Iterate through the list to find nodes that can be merged
                // Look at the right node, if it has the same size and is unused
                // then it can be merged into the current node. Everytime we merge,
                // go back to the beginning of the list and check again
                currentNode = memoryNodes->head;
                while (currentNode != NULL) {
                    if (currentNode->next != NULL && currentNode->next->size == currentNode->size && currentNode->next->used == 0 && currentNode->used == 0) {
                        printf("\t[MERGING]\n");
                        printf("\t\t[start_ptr=%d size=%d used=%d]\n", currentNode->start_ptr, currentNode->size, currentNode->used);
                        currentNode->size = currentNode->size + currentNode->next->size;
                        currentNode->next = currentNode->next->next;

                        printList(memoryNodes, 0);
                        currentNode = memoryNodes->head;
                        memoryNodes->size--;
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

// Splits a node into a given size.
// node's size = size, and a new node is created that is the size of
// node's original size - size
MemoryNode* split(MemoryList* list, MemoryNode* node, int size) {
    MemoryNode *remainingNode = malloc(sizeof(MemoryNode));
    remainingNode->size = node->size - size;
    remainingNode->start_ptr = node->start_ptr + size;

    remainingNode->start = node->start + size;
    remainingNode->size_ = (int *) (node->start + size);
    *(remainingNode->size_) = *node->size_ - size;

    remainingNode->used = 0;
    listTailInsert(memoryNodes, remainingNode);
    printList(memoryNodes, 0);
    node->size = size;
    *(node->size_) = size;
    node->used = 0;

    return node;
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
            if (node->start == NULL || node->size_ == NULL)
                printf("\t\t[size=%d, ptr=%d, used=%d]\n", node->size, node->start_ptr, node->used);
            else
                printf("\t\t[node_addr=%p start=%p size_ptr=%p size=%d, used=%d]\n", node, node->start, node->size_, *node->size_, node->used);
        }
        node = node->next;
    }
    printf("\n");
}