/*
Program Name: Email Prioritizer
Description: Prioritizes emails inputted via a text file by putting them in a MaxHeap
             then produces outputs for each command inside the text file.
Inputs: Text file containing email data
Outputs: Outputs what each command does during file reading.
Collaborators: Gemini for whole code
Author: Antonio DePaolis
Creation Date: 9/16/2026
Revision Date: 9/17/2026
*/

#include <stdio.h>    // standard input/output library for file handling and console I/O
#include <stdlib.h>   // Standard library for dynamic memory allocation and general utilities
#include <string.h>   // Standard library for string manipulation functions

// Define the Email structure to hold email details and priority scores
typedef struct {
    char sender[50];    // Stores the sender category of the email (e.g., "Boss", "Peer", etc.)
    char subject[100];  // Stores the subject line of the email
    char date[11];      // Stores the date of the email in "MM-DD-YYYY" format
    int priority_score; // Score that is based on sender for primary comparison
    int date_score;     // Score that is based on date for secondary comparison
} Email; // end of Email structure 

// Define the MaxHeap structure to manage a dynamic array of Email elements
typedef struct {
    Email *array;    // Pointer to the dynamic array that holds Email elements in heap order
    int capacity;    // Maximum number of elements the heap can currently hold
    int size;        // Current number of elements in the heap
} MaxHeap;

// Function prototypes for heap operations and utility functions
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

// Main function to execute the email prioritization program
int main() {
    char filename[100];                 // Buffer to store the name of the command file inputted by the user
    FILE *file;                         // File pointer to handle reading from the specified command file
    char line[256];                     // Buffer to read each line from the command file
    MaxHeap *heap = create_heap(10);    // Create a new MaxHeap with an initial capacity of 10 emails

    // Prompt the user to enter the command file name and read it safely
    printf("Enter the command file name: ");
    if (scanf("%99s", filename) != 1) { // Check if reading the filename input failed 
        printf("Error reading filename input.\n");  // Display error message for failed input
        free_heap(heap); // Free allocated memory for the heap to prevent memory leaks
        return 1; // Return error code 1
    }

    file = fopen(filename, "r");    // Open the specified command file in read mode
    if (file == NULL) { // Check if the file could not be opened 
        printf("Error: Could not open file %s\n", filename); // Display error message for failing to open file
        free_heap(heap); // free memory allocated preventing memory leaks
        return 1;   // Return error code 1
    }

    // Read each line from the command file and process commands
    while (fgets(line, sizeof(line), file) != NULL) {
        trim_whitespace(line);           // Remove trailing newlines or whitespace from line
        if (strlen(line) == 0) continue; // Skip empty lines

        // Check if line starts with "EMAIL " command
        if (strncmp(line, "EMAIL ", 6) == 0) {
            char *data = line + 6;  // Move pointer past "EMAIL " prefix to get the actual email data
            char *sender, *subject, *date; // Declare pointers to hold tokenized email fields

            sender = strtok(data, ",");  // Split string up to first comma for sender
            subject = strtok(NULL, ","); // Split string up to second comma for subject
            date = strtok(NULL, ",");    // Split string up to end/comma for date 

            // Checks if all three fields were successfully parsed before proceeding
            if (sender && subject && date) {
                Email new_email; // Declare a new Email structure to hold the parsed email data
                trim_whitespace(sender); // Clean up extra space around sender token
                trim_whitespace(subject);   // Clean up extra space around subject token
                trim_whitespace(date);  // Clean up extra space around date token

                // Copy parsed strings into the new_email structure while ensuring null-termination 
                strncpy(new_email.sender, sender, sizeof(new_email.sender) - 1);
                new_email.sender[sizeof(new_email.sender) - 1] = '\0'; // Ensure null-termination

                strncpy(new_email.subject, subject, sizeof(new_email.subject) - 1);
                new_email.subject[sizeof(new_email.subject) - 1] = '\0'; // Ensure null-termination

                strncpy(new_email.date, date, sizeof(new_email.date) - 1);
                new_email.date[sizeof(new_email.date) - 1] = '\0'; // Ensure null-termination

                // Calculate sender priority score based on sender string
                new_email.priority_score = get_sender_priority(new_email.sender);

                // Calculate date score by parsing the date string into a comparable integer
                new_email.date_score = parse_date_to_score(new_email.date);
                
                insert_email(heap, new_email); // Insert the newly created email into the MaxHeap priority queue
            }
        }
        
        // Check if line is the "NEXT" command
        else if (strcmp(line, "NEXT") == 0) {
            int success = 0;    // Check if peek operation succeeded
            Email top = peek_email(heap, &success); // Peek at the highest priority email without removing it
            if (success) { // If heap is not empty, display the email details
                printf("Next email:\n"); // Print header for next email
                printf("Sender: %s\n", top.sender);   // Print sender line
                printf("Subject: %s\n", top.subject); // Print subject line
                printf("Date: %s\n", top.date);       // Print date
            } else { // If heap is empty, display message indicating no emails are available
                printf("No emails in inbox.\n");
            }
        }

        // Check if line is the "READ" command
        else if (strcmp(line, "READ") == 0) {
            int success = 0;    // Check if extract operation succeeded
            extract_max(heap, &success); // Remove the highest priority email from the heap indicating that it has been read
            if (!success) { // If heap is empty, display message indicating no emails are available to read
                printf("No emails to read.\n");
            }
        }
        
        // Check if line is the "COUNT" command
        else if (strcmp(line, "COUNT") == 0) {
            // Display emails left in the inbox by printing the current size of the heap
            printf("There are %d emails to be read.\n", heap->size);
        }
    }
    
    fclose(file);   // Close the command file after processing all commands
    free_heap(heap);    // frees all dynamic memory associated with the heap to prevent memory leaks
    return 0;   // Return 0 indicating successful execution
}

// Creates a new MaxHeap structure with specified initial capacity
MaxHeap* create_heap(int initial_capacity) {
    MaxHeap *heap = (MaxHeap*)malloc(sizeof(MaxHeap)); // Allocate memory for the MaxHeap structure
    if (!heap) { // Check if memory allocation failed and displays an error message before exiting the program
        printf("Memory allocation failure!\n");
        exit(1);
    }
    heap->capacity = initial_capacity;  // Set initial capacity for the heap
    heap->size = 0; //
    heap->array = (Email*)malloc(sizeof(Email) * heap->capacity);   // Allocates memory for the internal dynamic array to hold Email elements
    if (!heap->array) { // Checks if memory allocation for the internal array failed and displays an error message before exiting the program
        printf("Memory allocation failure!\n");
        free(heap);
        exit(1);
    }
    return heap;    // Return pointer to the newly created MaxHeap structure
}

// Returns a priority score based on the sender category string
int get_sender_priority(const char *sender) {
    if (strcmp(sender, "Boss") == 0) return 5;              // Highest priority
    if (strcmp(sender, "Subordinate") == 0) return 4;       // 2nd priority
    if (strcmp(sender, "Peer") == 0) return 3;              // 3rd priority  
    if (strcmp(sender, "ImportantPerson") == 0) return 2;   // 4th priority
    if (strcmp(sender, "OtherPerson") == 0) return 1;       // Lowest priority
    return 0;   // Return 0 for unrecognized sender categories
}

// Parses a date string in "MM-DD-YYYY" format and converts it to an integer score for comparison
int parse_date_to_score(const char *date_str) {
    int month = 0, day = 0, year = 0;   // Initialize variables to hold month, day, and year values
    if (sscanf(date_str, "%d-%d-%d", &month, &day, &year) == 3) {   // Use sscanf to parse the date string into month, day, and year integers
        return (year * 10000) + (month * 100) + day;    // Convert date to an integer score in YYYYMMDD format for easy comparison
    }
    return 0;   // Return 0 if date parsing fails, indicating an invalid date format
}

// Compares two Email structures based on priority and date scores for heap ordering
int compare_emails(Email e1, Email e2) {
    if (e1.priority_score != e2.priority_score) {   // Compare based on sender priority score first
        return e1.priority_score - e2.priority_score;   // Return positive if e1 has higher priority, negative if e2 has higher priority
    return e1.date_score - e2.date_score;   // if sender priorities are equal, compare based on date score (more recent dates have higher scores)
    }
}

// Swaps two Email elements in memory
void swap_emails(Email *a, Email *b) {
    Email temp = *a;    // Store the value of element 'a' in temporary storage
    *a = *b;    // Overwrite element 'a' with the value of element 'b'
    *b = temp;  // Overwrite element 'b' with the value stored in temporary storage (original 'a')
}

// Restores max-heap property upwards starting from given index
void heapify_up(MaxHeap *heap, int index) {
    while (index > 0) { // Loop until reaching the root of the heap
        int parent = (index - 1) / 2;   // Calculate the index of the parent node in the heap array
        // Compare the current node with its parent; if current node has higher priority, swap them
        if (compare_emails(heap->array[index], heap->array[parent]) > 0) {
            swap_emails(&heap->array[index], &heap->array[parent]); // swaps the current node with its parent to maintain max-heap property
            index = parent; // Update index to the parent's index to continue heapifying up
        } else {
            break;  // If the current node is not greater than its parent, the max-heap property is satisfied, and the loop is exited
        }
    }
}

// Restores max-heap property downwards starting from given index
void heapify_down(MaxHeap *heap, int index) {
    int max_index = index;  // Initialize max_index to the current index to track the node with the highest priority during comparisons
    while (1) {
        int left = 2 * index + 1;   // Calculate left child index
        int right = 2 * index + 2;  // Calculate right child index

        // Check if left child exists and has higher priority than current max
        if (left < heap->size && compare_emails(heap->array[left], heap->array[max_index]) > 0) {
            max_index = left;
        }

        // Check if right child exists and has higher priority than current max
        if (right < heap->size && compare_emails(heap->array[right], heap->array[max_index]) > 0) {
            max_index = right;
        }

        // If the max_index has changed, swap the current node with the child that has the higher priority
        if (index != max_index) {
            swap_emails(&heap->array[index], &heap->array[max_index]);
            index = max_index;  // Update index to the new max_index to continue heapifying down
        } else {
            break;  // If no swaps were made, the max-heap property is satisfied, and the loop is exited
        }
    }
}

// Inserts a new Email into the MaxHeap, maintaining the max-heap property
void insert_email(MaxHeap *heap, Email email) {
    // Check if the heap is at capacity and needs to be expanded
    if (heap->size >= heap->capacity) {
        heap->capacity *= 2;    // Double the capacity of the heap to accommodate more elements
        heap->array = (Email*)realloc(heap->array, sizeof(Email) * heap->capacity); // Reallocate memory for the internal array to the new capacity
        if (!heap->array) { // Check if memory reallocation failed and displays an error message before exiting the program
            printf("Memory allocation failure on expansion!\n");
            exit(1);
        }
    }
    heap->array[heap->size] = email;    // Add the new email to the end of the heap array
    heap->size++;   // Increment the size of the heap to reflect the addition of the new email
    heapify_up(heap, heap->size - 1); // Restore the max-heap property by putting the new email in its correct position in the heap
}

// Returns the highest priority email without removing it from the priority queue
Email peek_email(MaxHeap *heap, int *success) {
    if (heap->size == 0) { // edge case: empty heap peek
        *success = 0;   // Set failure flag
        Email empty = {"", "", "", 0, 0};   // create a dummy empty Email structure to return in case of an empty heap
        return empty;   // Return the dummy empty structure to indicate that the heap is empty
    }
    *success = 1;   // Set success flag to indicate that the peek operation was successful
    return heap->array[0];  // Return the highest priority email, which is located at the root of the max-heap (index 0)
}

// Removes and returns the highest priority email from the MaxHeap, maintaining the max-heap property
Email extract_max(MaxHeap *heap, int *success) {
    if (heap->size == 0) {  // edge case: empty heap extract
        *success = 0;   // Set failure flag to indicate that the extract operation failed due to an empty heap
        Email empty = {"", "", "", 0, 0};   // create a dummy empty Email structure to return in case of an empty heap
        return empty;   // Return the dummy empty structure to indicate that the heap is empty
    }
    Email max_item = heap->array[0];    // Store the highest priority email (root of the heap) to return it after extraction
    heap->array[0] = heap->array[heap->size - 1];   // Move the last email in the heap to the root position to fill the gap left by the extracted email
    heap->size--;   // Decrement the size of the heap to reflect the removal of the highest priority email
    heapify_down(heap, 0);  // Moves the new root email down the heap to restore the max-heap property after extraction
    *success = 1;   // Set success flag to indicate that the extract operation was successful
    return max_item;    // Return the extracted highest priority email to the caller
}

// Frees all dynamically allocated memory associated with the MaxHeap structure
void free_heap(MaxHeap *heap) {
    if (heap) { // Check if the heap pointer is not NULL to avoid dereferencing a NULL pointer
        if (heap->array) {  // Check if the internal array pointer is not NULL to avoid dereferencing a NULL pointer
            free(heap->array);  // Free the memory allocated for the internal array of Email elements
        }
        free(heap); // Free the memory allocated for the MaxHeap structure itself to prevent memory leaks
    }
}

// Trims leading and trailing whitespace characters from a string in place
void trim_whitespace(char *str) {
    if (str == NULL || *str == '\0') return; // Checks if the input string is NULL or empty and returns early to avoid processing

    char *start = str; // Pointer to the start of the string for trimming leading whitespace
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
        start++;    // Moves the pointer forward in order to skip leading whitespace characters
    }

    // If the string is empty after trimming leading whitespace, set it to an empty string and return early
    if (*start == '\0') {
        *str = '\0';  // If the string is empty after trimming leading whitespace, set it to an empty string
        return; // Return early since there is nothing more to trim
    }

    char *end = str + strlen(start) - 1;    // Pointer to the end of the string for trimming trailing whitespace
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) { // Loop to find the last non-whitespace character in the string
        end--;  // Move the pointer backward to skip trailing whitespace characters
    }

    *(end + 1) = '\0'; // Null-terminate the string after the last non-whitespace character to finalize the trimming process

    if (start != str) { // If the start pointer has moved, shift the trimmed string to the beginning of the original string
        memmove(str, start, end - start + 2); // Use memmove to safely copy the trimmed string to the beginning of the original string buffer
    }
}