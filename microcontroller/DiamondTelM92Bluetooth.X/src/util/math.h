/** 
 * @file
 * @author Jeff Lau
 * 
 * A collection of math/number-related utilities.
 */

#ifndef MATH_H
#define	MATH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * Seeds the pseudorandom number generator with a value derived from the 
 * current values of TMR4 and TMR6.
 */
void randomize(void);

/**
 * Gets a pseudorandom unsigned number in the range [0, modulus)
 * @param modulus - The (exclusive) upper limit of the random number, or
 *        the count of possible different values.
 * @return A pseudorandom unsigned number in the range [0, modulus)
 */
unsigned random(unsigned modulus);

#ifdef	__cplusplus
}
#endif

#endif	/* MATH_H */

