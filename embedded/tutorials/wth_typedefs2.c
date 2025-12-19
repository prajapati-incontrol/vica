#include <stdio.h>
#include <stdlib.h>

// Define a simple structure
typedef struct {
    int x;
    int y;
    char name[20];
} Point;

// Function declaration - must be outside main()
void printPoint(Point *p);

int main() {
    printf("=== STRUCT ACCESS DEMO (.  vs  ->) ===\n\n");
    
    // ========================================
    // EXAMPLE 1: Using the DOT operator (.)
    // ========================================
    printf("1. Using DOT operator (.) with regular struct variable:\n");
    Point p1;  // Regular struct variable (stored on stack)
    p1.x = 10;
    p1.y = 20;
    snprintf(p1.name, sizeof(p1.name), "Point A");
    
    printf("   p1.x = %d\n", p1.x);
    printf("   p1.y = %d\n", p1.y);
    printf("   p1.name = %s\n\n", p1.name);
    
    
    // ========================================
    // EXAMPLE 2: Using the ARROW operator (->)
    // ========================================
    printf("2. Using ARROW operator (->) with pointer to struct:\n");
    Point *p2 = malloc(sizeof(Point));  // Pointer to struct (on heap)
    p2->x = 30;
    p2->y = 40;
    snprintf(p2->name, sizeof(p2->name), "Point B");
    
    printf("   p2->x = %d\n", p2->x);
    printf("   p2->y = %d\n", p2->y);
    printf("   p2->name = %s\n\n", p2->name);
    
    
    // ========================================
    // EXAMPLE 3: Showing they're equivalent
    // ========================================
    printf("3. Showing -> is equivalent to (*ptr). :\n");
    Point *p3 = malloc(sizeof(Point));
    
    // These two are IDENTICAL:
    p3->x = 50;              // Using ->
    (*p3).y = 60;            // Using dereference then dot
    
    printf("   p3->x = %d  (using ->)\n", p3->x);
    printf("   (*p3).y = %d  (using dereference and dot)\n", (*p3).y);
    printf("   Both ways work the same!\n\n");
    
    
    // ========================================
    // EXAMPLE 4: Pointer to existing struct
    // ========================================
    printf("4. Creating a pointer to an existing struct:\n");
    Point p4 = {70, 80, "Point D"};  // Regular struct
    Point *ptr = &p4;                 // Pointer to p4
    
    printf("   Access via variable:  p4.x = %d\n", p4.x);
    printf("   Access via pointer:   ptr->x = %d\n", ptr->x);
    printf("   They refer to the same memory!\n\n");
    
    // Modify through pointer
    ptr->x = 100;
    printf("   After ptr->x = 100:\n");
    printf("   p4.x is now: %d\n\n", p4.x);
    
    
    // ========================================
    // EXAMPLE 5: See printPoint function below main()
    // ========================================
    printf("5. Passing struct to function:\n");
    printPoint(&p1);  // Pass address of p1
    printPoint(p2);   // p2 is already a pointer
    
    
    // Clean up heap memory
    free(p2);
    free(p3);
    
    printf("\n=== KEY TAKEAWAY ===\n");
    printf("Use '.'  when you have a struct variable\n");
    printf("Use '->' when you have a pointer to a struct\n");
    
    return 0;
}

// Function definition - must be outside main()
void printPoint(Point *p) {
    printf("   Point: (%d, %d) named '%s'\n", p->x, p->y, p->name);
}
