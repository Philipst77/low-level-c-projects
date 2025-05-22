#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Function for user input
void user_input(int *n, int *l, int *m, int *h, char ***dna_strings) {
    do {
        printf("Please enter the number of input strings for motif search (n): "); // Number of strings
        scanf("%d", n);
    } while (*n < 2 || *n > 8);

    do {
        printf("Please enter the length of each input string (l): ");   // length of each input string
        scanf("%d", l);
            if (*l< 8 || *l > 16) {
        printf("Invalid input!\n");
    }
    } while (*l < 8 || *l > 16);

    do {
        printf("Please enter the length of motifs (m): ");  // lenght of the motifs so the substrings of each string that where looking for in the dna sequence.
        scanf("%d", m);
    } while (*m < 3 || *m > 5);

    do {
        printf("Please enter the number of allowable mismatches (h): "); // number of allowed mismatches between the motifs.
        scanf("%d", h);
    } while (*h < 0 || *h > 2);

    // Allocate memory for dna_strings
    *dna_strings = (char **)malloc(*n * sizeof(char *));   // allocating memeory for the 2d array to store the dna strings inputed by the user.
    if (*dna_strings == NULL) { 
        printf("Memory allocation failed\n");   // checking if memeory allocating worked.
        exit(1);
    }
    for (int i = 0; i < *n; i++) {                                      // allocating memeory for cols of 2d array
        (*dna_strings)[i] = (char *)malloc((*l + 1) * sizeof(char));
        if ((*dna_strings)[i] == NULL) {                // checking if memeory allocating worked
            printf("Memory allocation failed\n");
            exit(1);
        }
        do {
            printf("Please enter input string #%d: ", i + 1);                 // entering each indvidual string into the 2d array
            scanf("%s", (*dna_strings)[i]); 
            if(strlen((*dna_strings)[i]) != *l || strspn((*dna_strings)[i], "ACGT") != *l){  // adding input to 2d array col we derference the ith index of the 2d array to store the input there
                printf("Invalid input! \n");
            }  
        } while (strlen((*dna_strings)[i]) != *l || strspn((*dna_strings)[i], "ACGT") != *l);  // check condition that will keep loop going as long as string lenght
        //entered is not equal to length of string inputed before and we dereference l to compared the value at l to teh string lenght of the ith index of the 2d array and
        // we are comparing the letter at the ith index and if they contain ACGT and if it does not equal to the lenght of l we keep looping until we break these condiditions
    }
}

// Function to generate candidate motifs
void gen_candidates(char ***candidates, int m) {    // pass paramteres into the function we are passing a pointer to a 2d array which is a pointer to a pointer to a pointer
    char bases[] = "ACGT";              // are base characters for the motifs
    int total_combinations = (int)pow(4, m);    // total number of combintations formula 
    *candidates = (char **)malloc(total_combinations * sizeof(char *));  // indital dynamic array to store all possible canadites and we are intialzing the size of the array to
    // be the total combinations 
    
    if (*candidates == NULL) {
        printf("Memory allocation failed\n");  // checking if memory allocationg actaully worked
        exit(1);
    }
    
    for (int i = 0; i < total_combinations; i++) {
        (*candidates)[i] = (char *)malloc((m + 1) * sizeof(char));    // allocating memeory for the columns of the 2d array which are going to store all teh combintations
        // the base array is holds all the pointers poitnign the the subaray teh columns 
        if ((*candidates)[i] == NULL) {   // checking if the memeory allocation worked.
            printf("Memory allocation failed\n");
            exit(1);
        }
        int idx = i;   //putting the idx varible equal to current motif number.
        for (int j = 0; j < m; j++) {   // looping through the length of m which is the lenght of the motifs
            (*candidates)[i][j] = bases[idx % 4];   // derefernceing the elements of teh 2d array and makign them equal to base array which holds the ACGT Letters 
            // and module operating them by 4 to get number between 1-3 to get one of the letters.
            idx /= 4; // divide by for to get the next digit.
        }
        (*candidates)[i][m] = '\0';  //putting null terminatior at the end of the string .
    }

    // Display all candidates
    printf("All candidate motifs for m=%d are as follows:\n", m);

    for (int i = 0; i < total_combinations; i++) {      // for loop to print out candidates.
    printf("%s ", (*candidates)[i]);
    if ((i + 1) % 8 == 0) {
        printf("\n");
    }
}
printf("\n");
}
// Function to generate substrings
void gen_substrings(char *input_string, int l, int m, char ***substrings, int *num_substrings, int string_index) {
    *num_substrings = l - m + 1;   // calculating number of substring of lenth m that are in the input string.
    *substrings = (char **)malloc(*num_substrings * sizeof(char *));  // allocating memeory for the base array
    if (*substrings == NULL) {  // checking if memeory allocation worked
        printf("Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < *num_substrings; i++) {        // iterating through lenght of the base array of the 2d array 
        (*substrings)[i] = (char *)malloc((m + 1) * sizeof(char)); //allocting memeory for cols that will store substrings.
        if ((*substrings)[i] == NULL) { // checking if memeory allocating worked.
            printf("Memory allocation failed\n");
            exit(1);
        }
        strncpy((*substrings)[i], &input_string[i], m);  // copies m characters from input string into the ith element of the substrings
        (*substrings)[i][m] = '\0';  // adds null terminator to end of string.
    }

    // Display all substrings
    printf("All substrings of length %d from input string #%d are as follows:\n", m, string_index + 1);

    for (int i = 0; i < *num_substrings; i++) {    // looping through all substring and printing them out 
        printf("%s ", (*substrings)[i]);   //prints ith element of substring.
    }
    printf("\n");
}

// Function to calculate Hamming distance
int hamming_dist(char *str1, char *str2, int m) {
    int dist = 0;
    for (int i = 0; i < m; i++) {     // looping through each character postion in the motif
        if (str1[i] != str2[i]) {    // comparing index in string 1 and 2 and if there not equal to each other we increment the distacne function
            dist++;                 // and when str 1 and str2 have a same character 
        }                           // we return the distance variable.
    }
    return dist;
}

// Function to match motifs in a string
void match_motifs_in_string(char **candidates, int num_candidates, char **substrings, int num_substrings, int m, int h, int dna_string_index) {
    printf("The following are the candidate motifs of length %d with at most %d mismatch with substrings from input string #%d:\n", m, h, dna_string_index + 1);

    int motifs_printed = 0;
    for (int i = 0; i < num_candidates; i++) {        
        for (int j = 0; j < num_substrings; j++) {
            if (hamming_dist(candidates[i], substrings[j], m) <= h) { // Check if motif matches the substring
                printf("%s ", candidates[i]); // Print motif
                motifs_printed++;
                if (motifs_printed % 8 == 0) { // Print 8 motifs per line for readability
                    printf("\n");
                }
                break; // Move to the next candidate once a match is found for the current one
            }
        }
    }
    if (motifs_printed % 8 != 0) {
        printf("\n"); // Ensure a newline at the end if the last line wasn't full
    }
    printf("\n");
}


// Function to find motifs common to all input strings
int find_motifs(char **candidates, int num_candidates, char ***all_substrings, int *num_substrings, int n, int m, int h) {
    int motif_count = 0; // Initialize count of motifs found in all strings
    printf("The motifs found in all %d input strings are as follows:\n", n); // Header for motifs found in all strings

    for (int i = 0; i < num_candidates; i++) { // Loop through each candidate motif
        int found_in_all = 1; // Flag to track if the motif is found in all DNA strings
        
        for (int j = 0; j < n; j++) { // Loop through each DNA string
            int found_in_this_string = 0; // Flag to check if motif is found in current DNA string
            
            for (int k = 0; k < num_substrings[j]; k++) { // Loop through substrings of the j-th DNA string
                if (hamming_dist(candidates[i], all_substrings[j][k], m) <= h) { // Compare motif and substring
                    found_in_this_string = 1; // Motif matches a substring in this DNA string
                    break; // Stop searching this DNA string since we found a match
                }
            }
            
            if (!found_in_this_string) { // If the motif isn't found in this DNA string
                found_in_all = 0; // Set flag to indicate the motif isn't in all DNA strings
                break; // Exit the loop since the motif won't qualify
            }
        }
        
        if (found_in_all) { // If motif was found in all DNA strings
            printf("%s ", candidates[i]); // Print motif directly
            motif_count++; // Increment count of motifs found in all strings
        }
    }
    printf("\n"); // Newline after all motifs have been printed
    return motif_count; // Return the number of motifs found in all DNA strings
}


int main() {
    int n, l, m, h;
    char **dna_strings;

    printf("Basic Motif Search Program\n");
    user_input(&n, &l, &m, &h, &dna_strings);

    int num_candidates = (int)pow(4, m);
    char **candidates;
    gen_candidates(&candidates, m);

    char ***all_substrings = (char ***)malloc(n * sizeof(char **));
    if (all_substrings == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    int *num_substrings = (int *)malloc(n * sizeof(int));
    if (num_substrings == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        gen_substrings(dna_strings[i], l, m, &all_substrings[i], &num_substrings[i], i);

    }

    for (int i = 0; i < n; i++) {
        match_motifs_in_string(candidates, num_candidates, all_substrings[i], num_substrings[i], m, h, i);
    }

    find_motifs(candidates, num_candidates, all_substrings, num_substrings, n, m, h);

    // Freeing allocated memory
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < num_substrings[i]; j++) {
            free(all_substrings[i][j]);
            all_substrings[i][j] = NULL;
        }
        free(all_substrings[i]);
        all_substrings[i] = NULL;
        free(dna_strings[i]);
        dna_strings[i] = NULL;
    }
    free(all_substrings);
    all_substrings = NULL;
    free(dna_strings);
    dna_strings = NULL;
    for (int i = 0; i < num_candidates; i++) {
        free(candidates[i]);
        candidates[i] = NULL;
    }
    free(candidates);
    candidates = NULL;
    free(num_substrings);
    num_substrings = NULL;

    return 0;
}

