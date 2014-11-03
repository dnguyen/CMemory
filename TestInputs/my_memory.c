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
void listDelete(MemoryList*, MemoryNode*);
void printList(MemoryList*, int);

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

    switch (MALLOC_TYPE) {
        case FIRST_FIT:
            // Create a hole that is the size of the entire memory initially
            freeMemoryNodes = malloc(sizeof(MemoryList));
            MemoryNode *newNode = malloc(sizeof(MemoryNode));
            newNode->size = mem_size;
            newNode->start_ptr = 0;
            newNode->used = 0;
            listTailInsert(freeMemoryNodes, newNode);
            printList(freeMemoryNodes, 0);
            break;

        case BEST_FIT: break;
        case WORST_FIT: break;
        case BUDDY_SYSTEM: break;
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
                    // Look at adjacent nodes to see if there are holes we can merge.
                    if (current->next->used == 0 || current->prev->used == 0) {
                        // TODO
                    } else {
                        current->used = 0;
                    }
                    return;
                }
                current = current->next;
            }
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