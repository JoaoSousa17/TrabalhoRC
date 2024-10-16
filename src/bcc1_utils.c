#include "../include/bcc1_utils.h"

unsigned char calculate_bcc1(unsigned char address, unsigned char control) {
    return address ^ control; // BCC1 calculation: XOR of address and control field
}
