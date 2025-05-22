// DO NOT MODIFY
#ifndef LISTNODE_H
#define LISTNODE_H
typedef struct ListNode{
        char * command; // the command for the List Node
        char ** arguments; // an array of char pointers.
			  //NOTE: argv[0] should be the command with the remainder of argv[…] being the command line arguments typed after the command. 
			  //The index following the last command line argument must be set to the NULL pointer
        struct ListNode * next; // a pointer to the next List Node
        int id; // the id of the List Node 
        int arguments_length; // the length of the arguments array excluding the ending NULL pointer
        char * file_contents; // represents the file contents of the List Node
}ListNode;
#endif
