#ifndef STRING_UTILITY_H
#define STRING_UTILITY_H

// Cannot use string.h names or else compiler throws an error

// Copies a string src into dest
// Require: dest has enough elements to hold src, including its null terminator.
//          dest and src are valid strings.
void _strcpy(char* dest, const char* src);

// Returns the length of a string (does not include null terminator)
// Require: s is a valid string
// Return: positive integer representing the length of string s
int _strlen(const char* s);

// Compares string str1 with string str2.
// Require: str1 and str2 are valid strings.
// Return: integer indicating the relationship between the two strings:
//         <0: the first character that does not match has a lower value in str1
//          0: the contents of both strings are equal.
//         >0: the first character that does not match has a lower value in str2
//         In the <0 and >0 cases, the magnitude of the return value indicates
//         the index of the first character that does not match with base 1 index.
int _strcmp(const char* str1, const char*str2);



// Prints an integer i into a string s
// Require: s has enough room too hold the string representation of i
void itos(int i, char* s); 

// Returns the integer which the string represents. Parses from the 
// start of the string to the first space or the end of string.
// Require: s is a valid string.
// Return: integer that s represents.  
int stoi(char* s);

#endif
