/** 
 * @file
 * @author Jeff Lau
 * 
 * A collection of math/number-related utilities.
 */

#include "math.h"
#include "../../mcc_generated_files/tmr4.h"
#include "../../mcc_generated_files/tmr6.h"
#include <stdlib.h>

void randomize(void) {
  srand(((uint16_t)TMR6_ReadTimer() << 8) | TMR4_ReadTimer());
}

unsigned random(unsigned modulus) {
  return ((unsigned)rand()) % modulus;
}

