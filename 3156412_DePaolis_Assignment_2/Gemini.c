#include <stdio.h>    /* Standard input/output library for file operations and printing */
#include <stdlib.h>   /* Standard library for dynamic memory allocation (malloc, realloc, free) */
#include <string.h>   /* String library for string copying, comparison, and manipulation */

/* Structure representing an Email record */
typedef struct {
    char sender[50];    /* Stores the category of the sender (e.g., "Boss", "Subordinate") */
    char subject[100];  /* Stores the subject line of the email */
    char date[11];      /* Stores the email date string in format "MM-DD-YYYY" */
    int priority_score; /* Numeric score computed based on sender category (higher = higher priority) */
    int date_score;     /* Numeric score computed based on date for secondary comparison */
} Email;

/* Structure representing a dynamic, list-based MaxHeap priority queue */
typedef struct {
    Email *array;    /* Dynamic array storing the email elements of the heap */
    int capacity;    /* Current maximum allocation capacity of the heap array */
    int size;        /* Current number of elements stored in the heap */
} MaxHeap;

/* Forward declarations of functions */
MaxHeap* create_heap(int initial_capacity);
int get_sender_priority(const char *sender);
int parse_date_to_score(const char *date_str);
int compare_emails(Email e1, Email e2);
void swap_emails(Email *a, Email *b);
void heapify_up(MaxHeap *heap, int index);
void heapify_down(MaxHeap *heap, int index);
void insert_email(MaxHeap *heap, Email email);
Email peek_email(MaxHeap *heap, int *success);
Email extract_max(MaxHeap *heap, int *success);
void free_heap(MaxHeap *heap);
void trim_whitespace(char *str);

/* Main entry point of the program */
int main() {
    char filename[100];                  /* Buffer to hold the input file name from user */
    FILE *file;                         /* File pointer to handle reading the command file */
    char line[256];                     /* Buffer to read line by line from the input file */
    MaxHeap *heap = create_heap(10);   /* Initialize heap queue with an initial capacity of 10 */

    /* Prompt user to enter the command file name */
    printf("Enter the command file name: ");
    if (scanf("%99s", filename) != 1) { /* Read file name safely from standard input */
        printf("Error reading filename input.\n"); /* Handle input reading failure */
        free_heap(heap);                /* Free memory before exit */
        return 1;                       /* Return error code 1 */
    }

    file = fopen(filename, "r");        /* Open the specified file in read mode */
    if (file == NULL) {                 /* Check if the file failed to open (edge case: missing file) */
        printf("Error: Could not open file %s\n", filename); /* Display error message */
        free_heap(heap);                /* Free allocated memory */
        return 1;                       /* Return error code 1 */
    }

    /* Loop to read command lines from the file line-by-line */
    while (fgets(line, sizeof(line), file) != NULL) {
        trim_whitespace(line);           /* Remove trailing newlines or whitespace from line */
        if (strlen(line) == 0) continue; /* Skip empty lines (edge case: blank lines in file) */

        /* Check if line starts with the "EMAIL" command */
        if (strncmp(line, "EMAIL ", 6) == 0) {
            char *data = line + 6;       /* Shift pointer past "EMAIL " prefix */
            char *sender, *subject, *date;/* Pointers for comma-delimited tokens */

            sender = strtok(data, ",");  /* Tokenize string up to first comma for sender */
            subject = strtok(NULL, ","); /* Tokenize string up to second comma for subject */
            date = strtok(NULL, ",");    /* Tokenize string up to end/comma for date */

            /* Validate that all three required fields exist (edge case: malformed input) */
            if (sender && subject && date) {
                Email new_email;         /* Declare a temporary Email struct */
                trim_whitespace(sender); /* Clean up extra space around sender token */
                trim_whitespace(subject);/* Clean up extra space around subject token */
                trim_whitespace(date);   /* Clean up extra space around date token */

                /* Copy parsed values into the email structure fields safely */
                strncpy(new_email.sender, sender, sizeof(new_email.sender) - 1);
                new_email.sender[sizeof(new_email.sender) - 1] = '\0'; /* Ensure null-termination */

                strncpy(new_email.subject, subject, sizeof(new_email.subject) - 1);
                new_email.subject[sizeof(new_email.subject) - 1] = '\0'; /* Ensure null-termination */

                strncpy(new_email.date, date, sizeof(new_email.date) - 1);
                new_email.date[sizeof(new_email.date) - 1] = '\0'; /* Ensure null-termination */

                /* Calculate priority score based on sender string */
                new_email.priority_score = get_sender_priority(new_email.sender);
                /* Calculate chronological date score based on date string */
                new_email.date_score = parse_date_to_score(new_email.date);

                insert_email(heap, new_email); /* Insert the constructed email into the priority heap */
            }
        }
        /* Check if line is the "NEXT" command */
        else if (strcmp(line, "NEXT") == 0) {
            int success = 0;             /* Flag to check if peek operation succeeded */
            Email top = peek_email(heap, &success); /* Fetch highest priority email without removing */
            if (success) {               /* If heap was not empty */
                printf("Next email:\n"); /* Output formatted header */
                printf("Sender: %s\n", top.sender);   /* Print sender category */
                printf("Subject: %s\n", top.subject); /* Print subject line */
                printf("Date: %s\n", top.date);       /* Print date */
            } else {                     /* Handle empty heap edge case */
                printf("No emails in inbox.\n");
            }
        }
        /* Check if line is the "READ" command */
        else if (strcmp(line, "READ") == 0) {
            int success = 0;             /* Flag to check if extraction succeeded */
            extract_max(heap, &success); /* Remove highest priority email (CEO dealt with it) */
            if (!success) {              /* Handle edge case where heap is already empty */
                printf("No emails to read.\n");
            }
        }
        /* Check if line is the "COUNT" command */
        else if (strcmp(line, "COUNT") == 0) {
            /* Display current unread count based on current heap size */
            printf("There are %d emails to be read.\n", heap->size);
        }
    }

    fclose(file);                        /* Close the input command file safely */
    free_heap(heap);                     /* Free all dynamically allocated memory for the heap */
    return 0;                            /* Program completed successfully */
}

/* Creates and initializes a dynamic list-based MaxHeap */
MaxHeap* create_heap(int initial_capacity) {
    MaxHeap *heap = (MaxHeap*)malloc(sizeof(MaxHeap)); /* Allocate memory for heap controller structure */
    if (!heap) {                         /* Handle memory allocation failure edge case */
        printf("Memory allocation failure!\n");
        exit(1);
    }
    heap->capacity = initial_capacity;   /* Set initial capacity */
    heap->size = 0;                      /* Set initial element count to zero */
    heap->array = (Email*)malloc(sizeof(Email) * heap->capacity); /* Allocate dynamic list array */
    if (!heap->array) {                  /* Handle memory allocation failure edge case */
        printf("Memory allocation failure!\n");
        free(heap);
        exit(1);
    }
    return heap;                         /* Return pointer to initialized heap */
}

/* Converts sender string category to a numerical priority (higher integer = higher priority) */
int get_sender_priority(const char *sender) {
    if (strcmp(sender, "Boss") == 0) return 5;            /* Top priority */
    if (strcmp(sender, "Subordinate") == 0) return 4;     /* 2nd priority */
    if (strcmp(sender, "Peer") == 0) return 3;            /* 3rd priority */
    if (strcmp(sender, "ImportantPerson") == 0) return 2; /* 4th priority */
    if (strcmp(sender, "OtherPerson") == 0) return 1;     /* Lowest priority */
    return 0;                                             /* Fallback for unknown category */
}

/* Parses date format "MM-DD-YYYY" into a comparable YYYYMMDD integer score */
int parse_date_to_score(const char *date_str) {
    int month = 0, day = 0, year = 0;                     /* Local integer storage for parsed date components */
    if (sscanf(date_str, "%d-%d-%d", &month, &day, &year) == 3) { /* Parse formatted integers from string */
        return (year * 10000) + (month * 100) + day;      /* Combine into YYYYMMDD sortable representation */
    }
    return 0;                                             /* Return 0 on failure to parse */
}

/* Compares two email items: returns >0 if e1 has higher priority than e2, <0 if lower, 0 if equal */
int compare_emails(Email e1, Email e2) {
    if (e1.priority_score != e2.priority_score) {         /* Check if primary sender priorities differ */
        return e1.priority_score - e2.priority_score;     /* Positive if e1 has higher sender rank */
    }
    return e1.date_score - e2.date_score;                 /* Primary ties broken by recency (newest/larger score wins) */
}

/* Swaps two Email elements in memory */
void swap_emails(Email *a, Email *b) {
    Email temp = *a;                                      /* Store element 'a' in temporary storage */
    *a = *b;                                              /* Overwrite element 'a' with element 'b' */
    *b = temp;                                            /* Overwrite element 'b' with temporary storage */
}

/* Restores max-heap property upwards starting from given index */
void heapify_up(MaxHeap *heap, int index) {
    while (index > 0) {                                   /* Loop until node reaches root */
        int parent = (index - 1) / 2;                     /* Compute parent index in array representation */
        /* If current node has higher priority than its parent */
        if (compare_emails(heap->array[index], heap->array[parent]) > 0) {
            swap_emails(&heap->array[index], &heap->array[parent]); /* Swap current node with parent */
            index = parent;                               /* Move index up to parent position */
        } else {
            break;                                        /* Heap structure is valid, exit loop */
        }
    }
}

/* Restores max-heap property downwards starting from given index */
void heapify_down(MaxHeap *heap, int index) {
    int max_index = index;                                /* Initialize max index as current index */
    while (1) {
        int left = 2 * index + 1;                         /* Calculate left child index */
        int right = 2 * index + 2;                        /* Calculate right child index */

        /* Check if left child exists and has higher priority than current max */
        if (left < heap->size && compare_emails(heap->array[left], heap->array[max_index]) > 0) {
            max_index = left;
        }

        /* Check if right child exists and has higher priority than current max */
        if (right < heap->size && compare_emails(heap->array[right], heap->array[max_index]) > 0) {
            max_index = right;
        }

        /* If child was larger than current node, swap and keep heapifying down */
        if (index != max_index) {
            swap_emails(&heap->array[index], &heap->array[max_index]);
            index = max_index;                            /* Update current index to point to swapped child */
        } else {
            break;                                        /* Heap structure is valid, exit loop */
        }
    }
}

/* Inserts a new email into the priority queue (MaxHeap) */
void insert_email(MaxHeap *heap, Email email) {
    /* Handle dynamic array resizing if capacity is reached (edge case: list-based array expansion) */
    if (heap->size >= heap->capacity) {
        heap->capacity *= 2;                              /* Double capacity */
        heap->array = (Email*)realloc(heap->array, sizeof(Email) * heap->capacity); /* Reallocate memory */
        if (!heap->array) {                               /* Check for allocation failure */
            printf("Memory allocation failure on expansion!\n");
            exit(1);
        }
    }
    heap->array[heap->size] = email;                      /* Place new item at bottom end of heap */
    heap->size++;                                         /* Increment element count */
    heapify_up(heap, heap->size - 1);                     /* Bubbled up to restore heap order */
}

/* Returns highest priority email without removing it */
Email peek_email(MaxHeap *heap, int *success) {
    if (heap->size == 0) {                                /* Edge case: empty heap peek */
        *success = 0;                                     /* Set failure flag */
        Email empty = {"", "", "", 0, 0};                 /* Return dummy empty structure */
        return empty;
    }
    *success = 1;                                         /* Set success flag */
    return heap->array[0];                                /* Return root element */
}

/* Removes and returns highest priority email from the priority queue */
Email extract_max(MaxHeap *heap, int *success) {
    if (heap->size == 0) {                                /* Edge case: empty heap extraction */
        *success = 0;                                     /* Set failure flag */
        Email empty = {"", "", "", 0, 0};                 /* Return dummy empty structure */
        return empty;
    }
    Email max_item = heap->array[0];                      /* Store max element at root */
    heap->array[0] = heap->array[heap->size - 1];         /* Move last element to root position */
    heap->size--;                                         /* Decrement heap size */
    heapify_down(heap, 0);                                /* Bubble down root to restore heap property */
    *success = 1;                                         /* Set success flag */
    return max_item;                                      /* Return extracted priority item */
}

/* Frees all dynamic memory associated with the heap queue */
void free_heap(MaxHeap *heap) {
    if (heap) {                                           /* Check if heap pointer is non-null */
        if (heap->array) {                                /* Check if internal dynamic list exists */
            free(heap->array);                            /* Free dynamic memory array */
        }
        free(heap);                                       /* Free heap container structure */
    }
}

/* Utility function to trim leading and trailing spaces/newlines from strings */
void trim_whitespace(char *str) {
    char *end;                                            /* Pointer tracking end of string */
    while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n') str++; /* Skip leading whitespace */
    if (*str == 0) return;                                /* Handle empty string */
    end = str + strlen(str) - 1;                          /* Point to last character */
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) end--; /* Trim trailing */
    end[1] = '\0';                                        /* Write null-terminator at end of trimmed string */
}