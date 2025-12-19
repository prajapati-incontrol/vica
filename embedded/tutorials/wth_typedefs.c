// typdef is a keyword that is used to provide existing data types a new name, to redefine the already existing data types. 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Students {
	char name[50]; 
	char branch[50]; 
	int ID_no; 
} stu; // tag is Students, typedef alias is stu

typedef struct {
	int x; 
	int y; 
	char name[20]; 
} Point; // Anonymous tag 




int main() {
	// Using alias to define structure student 
	
	stu s; // OR struct Students s;  
	strcpy(s.name, "Geeks"); 
	strcpy(s.branch, "CSE"); 
	s.ID_no = 108; 

	printf("%s\n", s.name); 
	printf("%s\n", s.branch); 
	printf("%d", s.ID_no); 
	return 0; 
}
