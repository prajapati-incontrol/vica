#include <stdio.h>
#include <stdint.h>
#include <utils.h>

void print_binary(const char* label, uint8_t value){
	printf("%s: 0b" BYTE_TO_BINARY_PATTERN "\n", label, BYTE_TO_BINARY(value)); 
}

int main() {
    uint8_t a = 0b10110100;  // 180
    uint8_t b = 0b01101001;  // 105
    uint8_t result;
    
    printf("=== BITWISE OPERATORS ===\n\n");
    
    // Initial values
    print_binary("a        ", a);
    print_binary("b        ", b);
    printf("\n");
    
    // AND
    result = a & b;
    print_binary("a & b    ", result);
    printf("  (bits set in BOTH)\n\n");
    
    // OR
    result = a | b;
    print_binary("a | b    ", result);
    printf("  (bits set in EITHER)\n\n");
    
    // XOR
    result = a ^ b;
    print_binary("a ^ b    ", result);
    printf("  (bits set in ONE but not BOTH)\n\n");
    
    // NOT
    result = ~a;
    print_binary("~a       ", result);
    printf("  (all bits flipped)\n\n");
    
    printf("=== SHIFT OPERATORS ===\n\n");
    
    // Left shift
    result = a << 2;
    print_binary("a << 2   ", result);
    printf("  (shift left by 2, multiply by 4)\n\n");
    
    // Right shift
    result = a >> 2;
    print_binary("a >> 2   ", result);
    printf("  (shift right by 2, divide by 4)\n\n");
    
    printf("=== COMPOUND ASSIGNMENT ===\n\n");
    
    uint8_t reg = 0b00000000;
    
    // OR assignment (set bits)
    reg |= (1 << 3);
    print_binary("reg |= (1 << 3)", reg);
    printf("  (set bit 3)\n\n");
    
    // AND assignment (clear bits)
    reg &= ~(1 << 3);
    print_binary("reg &= ~(1 << 3)", reg);
    printf("  (clear bit 3)\n\n");
    
    // XOR assignment (toggle bits)
    reg = 0b00001000;
    reg ^= (1 << 3);
    print_binary("reg ^= (1 << 3)", reg);
    printf("  (toggle bit 3 from 1 to 0)\n\n");
    
    // Left shift assignment
    reg = 0b00000011;
    reg <<= 2;
    print_binary("reg <<= 2", reg);
    printf("  (multiply by 4)\n\n");
    
    // Right shift assignment
    reg >>= 1;
    print_binary("reg >>= 1", reg);
    printf("  (divide by 2)\n\n");
    
    printf("=== PRACTICAL PATTERNS ===\n\n");
    
    // Set multiple bits
    reg = 0b00000000;
    reg |= (1 << 3) | (1 << 5) | (1 << 7);
    print_binary("Set bits 3,5,7", reg);
    printf("\n");
    
    // Clear multiple bits
    reg = 0b11111111;
    reg &= ~((1 << 2) | (1 << 4) | (1 << 6));
    print_binary("Clear bits 2,4,6", reg);
    printf("\n");
    
    // Check if bit is set
    reg = 0b00100000;
    printf("Is bit 5 set? %s\n", (reg & (1 << 5)) ? "Yes" : "No");
    printf("Is bit 3 set? %s\n\n", (reg & (1 << 3)) ? "Yes" : "No");
    
    // Extract bit field (bits 4-6)
    reg = 0b01011000;
    result = (reg >> 4) & 0x07;
    print_binary("Original    ", reg);
    print_binary("Bits 4-6    ", result);
    printf("\n");
    
    // Create mask
    uint8_t mask = (1 << 4) - 1;  // Lower 4 bits
    print_binary("Mask (lower 4)", mask);
    printf("(1 << 4) - 1 = 0b00001111 \n (1<<3) - 1=0b00000111"); 	
    printf("\n");
    
    // Swap bits using XOR
    uint8_t x = 0b10100000;
    uint8_t y = 0b00001010;
    printf("=== XOR SWAP ===\n");
    print_binary("x before", x);
    print_binary("y before", y);
    x ^= y;
    y ^= x;
    x ^= y;
    print_binary("x after ", x);
    print_binary("y after ", y);
    printf("\n");
    
    // Check if power of 2
    uint8_t num = 16;
    printf("Is %u a power of 2? %s\n", num, 
           (num && !(num & (num - 1))) ? "Yes" : "No");
    num = 15;
    printf("Is %u a power of 2? %s\n\n", num, 
           (num && !(num & (num - 1))) ? "Yes" : "No");
    
    // Count set bits (population count)
    reg = 0b10110101;
    int count = 0;
    uint8_t temp = reg;
    while (temp) {
        count += temp & 1;
        temp >>= 1;
    }
    print_binary("Value   ", reg);
    printf("Set bits: %d\n\n", count);
    
    // Reverse bits
    reg = 0b10110100;
    result = 0;
    for (int i = 0; i < 8; i++) {
        result <<= 1;
        result |= (reg & 1);
        reg >>= 1;
    }
    print_binary("Original", 0b10110100);
    print_binary("Reversed", result);
    
    return 0;
}

