
/* _____________________
  |  _________________  |
  | |     MUAN    3.25| |
  | |_________________| |
  |  ___ ___ ___   ___  |
  | | 7 | 8 | 9 | | + | |
  | |___|___|___| |___| |
  | | 4 | 5 | 6 | | - | |
  | |___|___|___| |___| |
  | | 1 | 2 | 3 | | x | |
  | |___|___|___| |___| |
  | | . | 0 | = | | V | |
  | |___|___|___| |___| |
  |_____________________|
 */

#include <stdio.h>
#include <stdlib.h>
#include "common_structs.h"
#include "common_definitions.h"
#include "common_functions.h"
#include "smallfp.h"

// Feel free to add many Helper Functions, Consts, and Definitions!
// NaN value, sign 0, exponent 1111, mantissa 100000
// binary 0011 1110 0000
#define NAN 0x3E0
// positive infinity value, sign 0, exponent 1111, mantissa 000000
// binary 0011 1100 0000
#define PINF 0x3C0
// negative infinity value, sign 1, exponent 1111, mantissa 000000
// binary 0111 1100 0000
#define NINF 0x7C0
// positive zero value, sign 0, exponent 0000, mantissa 000000
// binary 0000 0000 0000
#define PZERO 0x000
// negative zero value, sign 1, exponent 0000, mantissa 000000
// binary 0100 0000 0000
#define NZERO 0x400
// exponent bias for our smallfp format
// formula is 2^(e-1)-1, in our case e = 4 (4 bits for exponent)
// so 2^(4-1)-1 is 7
#define BIAS 7

// helper function which takes a normalized value and other relevant
// data, rounds the number, and returns the final smallfp_s result
// Inputs:
// unsigned int val - normalized value (leading 1 plus 6-bit mantissa)
// short E - exponent
// unsigned short S - sign
// unsigned short rbit - rounding bit
// unsigned short sbit - sticky bit
// Output:
// smallfp_s result - rounded smallfp_s value
smallfp_s roundAndBuildSmallFP(unsigned int val, short E, unsigned short S, unsigned short rbit, unsigned short sbit) {
  // if the rounding bit is 1, we will maybe need to round
  if (rbit) {
    // if the sticky bit is 1, we round up
    if (sbit) {
      val += 1;
    }
    // else, we are exactly in the middle, so round to even
    else {
      // if the LSB of mantissa is 1, we round up
      if ((val & 0x1) != 0) {
        val += 1;
      }
    }
  }
  // if it was rounded up, we might have to shift again and adjust
  // the exponent so check if the leading 1 turned to leading 10
  // i.e. if the bit at the position 6 is 1 or 0
  // if it is 0, we need to shift once again
  if ((val & 0x40) == 0) {
    // shift one place to the right
    val >>= 1;
    // increment the exponent
    E++;
    // check exponent for overflow
    if (E > 7) {
      // if sign S is 1 (negative), return negative infinity
      // otherwise, return positive infinity
      return S ? NINF : PINF;
    }
  }
  // set the mantissa value by copying all bits except
  // for the leading 1 (that is, copy 6 lowest bits)
  unsigned short M = val & 0x3F;
  // add bias to exponent
  E += BIAS;
  // finally, build the small_fp value and return it
  smallfp_s result = (S << 10) | (E << 6) | M;
  return result;
}

// ----------Public API Functions (write these!)-------------------

/* toSmallFP - Converts a Number Struct (whole and fraction parts) into a SmallFP Value
 *  - number is managed by MUAN, DO NOT FREE number.
 *    - You may change the contents of number, but do not free it.
 *  - Follow the project documentation for this function.
 * Return the SmallFP Value or any legal SmallFP NaN representation if number is NULL.
 */
smallfp_s toSmallFP(Number_s *number) {
  unsigned short S; // sign
  short E; // exponent (can be negative, so it is not unsigned)
  unsigned int all; // variable to hold whole and fractional part together
  unsigned short sticky_bit = 0; // sticky bit, used for rounding
  unsigned short rounding_bit = 0; // rounding bit, used for rounding

  // constants NAN, PINF, NINF, PZERO, NZERO are defined using define
  // directives at the top of the file
  // return nan if the number pointer is null or if it is nan itself
  if (number == NULL || number->is_nan) {
    return NAN;
  }
  // else, if it is infinity, return positive or negative infinity based
  // on the sign
  else if (number->is_infinity) {
    return (number->is_negative) ? NINF : PINF;
  }
  // else, if it is zero, return positive or negative zero based on the sign
  else if (number->whole == 0 && number->fraction == 0) {
    return (number->is_negative) ? NZERO : PZERO;
  }
  // get sign value from the is_negative flag
  S = number->is_negative;
  // construct the all variable by taking the whole part,
  // shifting it left for 16 places, and then adding the fraction part
  // e.g. if whole is 0x3 and fraction is 0x4000,
  // variable all will be 0x00034000
  // this way, we will be able to shift the values more easily
  all = number->whole;
  all <<= 16;
  all |= number->fraction;

  // if the whole is greater than zero, then we should shift right
  // until we get the form where 1 is at the LSB of the whole part
  if (number->whole > 0) {
    // initialize the exponent value to 0
    E = 0;
    // loop until we get 1 at the LSB of the whole part, which is
    // the bit number 16 of the all variable
    while ((all & 0xFFFF0000) != 0x00010000) {
      // update the sticky bit with the previous rounding bit
      sticky_bit |= rounding_bit;
      // get the next rounding bit, before shifting
      rounding_bit = all & 0x1;
      // shift the all variable one place to the right
      all >>= 1;
      // increment the exponent
      E++;
    }
    // check exponent for overflow
    if (E > 7) {
      // if sign S is 1 (negative), return negative infinity
      // otherwise, return positive infinity
      return S ? NINF : PINF;
    }
    // now we have a valid exponent and a 16-bit mantissa with leading 1;
  }
  // else, if the whole is equal to zero, then we should shift left
  // until we get the form where 1 is at the LSB of the whole part
  // (at this point we know that the fraction part is not zero
  //  because we checked that case before at line 89, so we do not
  //  worry about the possibility of an endless loop)
  else {
    // initialize the exponent value to 0
    E = 0;
    // loop until we get 1 at the LSB of the whole part, which is
    // the bit number 16 of the all variable
    while ((all & 0xFFFF0000) != 0x00010000) {
      // shift the all variable one place to the right
      all <<= 1;
      // decrement the exponent
      E--;
    }
    // check exponent for underflow
    if (E < -6) {
      // if sign S is 1 (negative), return negative zero
      // otherwise, return positive zero
      return S ? NZERO : PZERO;
    }
    // now we have a valid exponent and a 16-bit mantissa with leading 1;
  }
  // before shifting the value for 10 places to the right,
  // get the rounding bit and update the sticky bit if needed
  // the rounding bit is bit 9 (the first one after 6 MSBs)
  rounding_bit = (all >> 9) & 0x1;
  // the sticky bits are bits 0-8, set our sticky bit to 1
  // if any one of them is 1
  if ((all & 0x1FF) != 0) {
    sticky_bit = 1;
  }
  // now shift everything 10 places to the right
  // we still keep the leading one for possible rounding
  all >>= 10;
  // call the function to round it and return the final value
  return roundAndBuildSmallFP(all, E, S, rounding_bit, sticky_bit);
}

/* toNumber - Converts a SmallFP Value into a Number Struct (whole and fraction parts)
 *  - number is managed by MUAN, DO NOT FREE or re-Allocate number.
 *    - It is already allocated.  Do not call malloc/calloc for number.
 *  - Follow the project documentation for this function.
 *  If the conversion is successful, return 0.
 *  - If number is NULL, return -1.
 */
int toNumber(Number_s *number, smallfp_s value) {
  // if number is null, return -1
  if (number == NULL) {
    return -1;
  }
  // define variables for whole and fraction part
  unsigned short whole;
  unsigned short fraction;
  // get the sign, exponent and mantissa
  unsigned short S = (value & 0x400) >> 10;
  short E = (value & 0x3C0) >> 6;
  unsigned short M = value & 0x3F;
  // check for nan value
  // exponent = 1111, mantisa different from 0
  if (E == 0xF && M != 0) {
    // set the number to nan
    number->is_nan = 1;
    number->is_negative = S;
    number->is_infinity = 0;
    number->whole = 0;
    number->fraction = 0;
  }
  // check for the infinity
  else if (E == 0xF && M == 0) {
    // set the number to infinity
    number->is_nan = 0;
    number->is_negative = S;
    number->is_infinity = 1;
    number->whole = 0;
    number->fraction = 0;
  }
  // check for zero
  else if (E == 0 && M == 0) {
    // set the number to zero
    number->is_nan = 0;
    number->is_negative = S;
    number->is_infinity = 0;
    number->whole = 0;
    number->fraction = 0;
  }
  // else, it is normalized, get the whole and fraction parts
  else {
    // fill in the number flags
    number->is_nan = 0;
    number->is_negative = S;
    number->is_infinity = 0;
    // unbias the exponent
    E -= BIAS;
    // create a 32-bit variable which we will use for easier shifting
    unsigned int all;
    // fill it with the mantissa
    all = M;
    // mantissa has to be located at bits 15-10
    all <<= 10;
    // add the leading one at bit 16
    all |= 0x00010000;
    // if exponent is non-negative, we will have to multiply the value
    if (E >= 0) {
      // we can multiply by shifting to the left until exponent becomes zero
      while (E != 0) {
        // shift the value left
        all <<= 1;
        // decrement the exponent
        E--;
      }
    }
    // otherwise, we will have to divide the value
    else {
      // we can divide by shifting to the right until exponent becomes zero
      while (E != 0) {
        // shift the value right
        all >>= 1;
        // increment the exponent
        E++;
      }
    }
    // now, the whole part is upper 16 bits and the fraction part
    // is the lower 16 bits
    number->whole = all >> 16;
    number->fraction = all & 0xFFFF;
  }
  // return 0 as success
  return 0;
}

/* mulSmallFP - Performs an operation on two SmallFP values
 *  - Follow the project documentation for this function.
 * Return the resulting smallfp_s value
 */
smallfp_s mulSmallFP(smallfp_s val1, smallfp_s val2) {
  // define the sign exponent and mantissa of the result
  unsigned short S;
  short E;
  unsigned short M;
  // get the sign, exponent and mantissa of both values
  unsigned short S1 = (val1 & 0x400) >> 10;
  short E1 = (val1 & 0x3C0) >> 6;
  unsigned short M1 = val1 & 0x3F;
  unsigned short S2 = (val2 & 0x400) >> 10;
  short E2 = (val2 & 0x3C0) >> 6;
  unsigned short M2 = val2 & 0x3F;
  // define and initialize sticky and rounding bit
  unsigned short sticky_bit = 0;
  unsigned short rounding_bit = 0;

  // perform bitwise xor on signs to get the sign of the result
  S = S1 ^ S2;

  // if either val1 or val2 are nan, return nan
  // exponent = 1111, mantisa different from 0
  if ((E1 == 0xF && M1 != 0) || (E2 == 0xF && M2 != 0)) {
    return NAN;
  }
  // else, if val1 is infinity, check val2 and return the result
  // accordingly
  else if (E1 == 0xF && M1 == 0) {
    // if val2 is 0, return nan
    if (E2 == 0 && M2 == 0) {
      return NAN;
    }
    // else, return infinity with the appropriate sign
    else {
      return S ? NINF : PINF;
    }
  }
  // else, if val2 is infinity, check val1 and return the result
  // accordingly
  else if (E2 == 0xF && M2 == 0) {
    // if val1 is 0, return nan
    if (E1 == 0 && M1 == 0) {
      return NAN;
    }
    // else, return infinity with the appropriate sign
    else {
      return S ? NINF : PINF;
    }
  }
  // else, if either one of them is zero, return zero with the
  // appropriate sign
  else if ((E1 == 0 && M1 == 0) || (E2 == 0 && M2 == 0)) {
    return S ? NZERO : PZERO;
  }
  // else, perform multiplication
  else {
    // unbias the exponents
    E1 -= BIAS;
    E2 -= BIAS;
    // sum the exponents
    E = E1 + E2;
    // insert leading 1 to mantissas at position 6
    M1 |= 0x40;
    M2 |= 0x40;
    // multiply the mantissas
    M = M1 * M2;
    // since we multiplied two mantissas with 1 whole and 6 fractional,
    // the resulting mantissa has 2 bits for whole and 12 bits for fractional
    // if the higher bit of the whole part (position 13) is 1, we have to
    // shift the result one place to the right (divide by two)
    if (M & 0x2000 != 0) {
      // get the sticky bit before shifting
      sticky_bit = M & 0x1;
      // shift to the right
      M >>= 1;
      // increment the exponent
      E++;
    }
    // check exponent for the overflow
    if (E > 7) {
      // return infinity according to the sign
      return S ? NINF : PINF;
    }
    // check exponent for the underflow
    else if (E < -6) {
      // return zero according to the sign
      return S ? NZERO : PZERO;
    }
    // now M contains 1 bit for whole and 12 bits for fractional
    // before shifting the M value for 6 places to the right,
    // get the rounding bit and update the sticky bit if needed
    // the rounding bit is bit 5 (the first one after 6 MSBs)
    rounding_bit = (M >> 5) & 0x1;
    // the sticky bits are bits 0-4, set our sticky bit to 1
    // if any one of them is 1
    if ((M & 0x1F) != 0) {
      sticky_bit = 1;
    }
    // now shift M value 6 places to the right
    // we still keep the leading one for possible rounding
    M >>= 6;
    // call the function to round it and return the final value
    return roundAndBuildSmallFP(M, E, S, rounding_bit, sticky_bit);
  }
}

/* addSmallFP - Performs an operation on two SmallFP values
 *  - Follow the project documentation for this function.
 * Return the resulting smallfp_s value
 */
smallfp_s addSmallFP(smallfp_s val1, smallfp_s val2) {
  // define the sign exponent and mantissa of the result
  unsigned short S;
  short E;
  short M; // mantissa might be negative in the process of adding
  // get the sign, exponent and mantissa of both values
  unsigned short S1 = (val1 & 0x400) >> 10;
  short E1 = (val1 & 0x3C0) >> 6;
  short M1 = val1 & 0x3F;
  unsigned short S2 = (val2 & 0x400) >> 10;
  short E2 = (val2 & 0x3C0) >> 6;
  short M2 = val2 & 0x3F;

  // if either val1 or val2 are nan, return nan
  // exponent = 1111, mantisa different from 0
  if ((E1 == 0xF && M1 != 0) || (E2 == 0xF && M2 != 0)) {
    return NAN;
  }
  // else, if val1 is infinity, check val2 and return the result
  // accordingly
  else if (E1 == 0xF && M1 == 0) {
    // if val2 is also infinity, check the sign
    if (E2 == 0xF && M2 == 0) {
      // if signs are different, return nan
      if (S1 != S2) {
        return NAN;
      }
      // otherwise, return infinity with appropriate sign
      else {
        S1 ? NINF : PINF;
      }
    }
    // else, return infinity with the sign of val1
    else {
      return S1 ? NINF : PINF;
    }
  }
  // else, if val2 is infinity, return infinity with the sign of val2
  else if (E2 == 0xF && M2 == 0) {
    // no need to check for val1 infinity, as it was checked before
    // just return infinity value with the sign of val2
    return S2 ? NINF : PINF;
  }
  // else, if val1 is zero, check val2 and return the result
  // accordingly
  else if (E1 == 0 && M1 == 0) {
    // if val2 is also zero, set the sign and return zero
    if (E2 == 0 && M2 == 0) {
      // sign will be 1 only if both are negative
      S = S1 & S2;
      // return zero according to the sign
      return S ? NZERO : PZERO;
    }
    // otherwise, just return val2
    else {
      return val2;
    }
  }
  // else, if val2 is zero, return val1
  else if (E2 == 0 && M2 == 0) {
    return val1;
  }
  // else, perform addition
  else {
    // define and initialize sticky and rounding bit
    unsigned short sticky_bit = 0;
    unsigned short rounding_bit = 0;
    // unbias the exponents
    E1 -= BIAS;
    E2 -= BIAS;
    // insert leading 1 to mantissas at position 6
    M1 |= 0x40;
    M2 |= 0x40;
    // if E1 is greater than E2, shift M1 until exponents become equal
    if (E1 > E2) {
      // shift it to the left by the difference of two exponents
      M1 <<= (E1 - E2);
      // set the resulting exponent to that of val2
      E = E2;
    }
    // else, if E2 is greater than E1, shift M2 until exponents become equal
    else if (E2 > E1) {
      // shift it to the left by the difference of two exponents
      M2 <<= (E2 - E1);
      // set the resulting exponent to that of val1
      E = E1;
    }
    // else, if they are equal, just set E to that of val1
    else {
      E = E1;
    }
    // negate M1 and M2 if their signs are negative
    M1 = S1 ? -M1 : M1;
    M2 = S2 ? -M2 : M2;
    // now add them
    M = M1 + M2;
    // if negative, set the sign and revert it back to absolute value
    if (M < 0) {
      S = 1;
      M = -M;
    }
    // else, if positive, set the sign to 0
    else {
      S = 0;
    }
    // now there is a possibility that it is not normalized due to
    // shifting and adding, the leading 1 could be on any side
    // so, first check if it is zero
    if (M == 0) {
      // if it is zero, return zero according to the sign
      return S ? NZERO : PZERO;
    }
    // else, if the leading one is somewhere to the left of the
    // fractional point, shift the M value to the right to find it
    else if ((M & 0xFFC0) != 0) {
      // shift until we reach leading one at position 6
      while ((M & 0xFFC0) != 0x0040) {
        // update the sticky bit with the previous rounding bit
        sticky_bit |= rounding_bit;
        // get the next rounding bit, before shifting
        rounding_bit = M & 0x1;
        // shift the M value one place to the right
        M >>= 1;
        // increment the exponent
        E++;
      }
      // check exponent for overflow
      if (E > 7) {
        // if sign S is 1 (negative), return negative infinity
        // otherwise, return positive infinity
        return S ? NINF : PINF;
      }
    }
    // else, the leading one must be to the right of the fractional
    // point, so shift the M value to the left
    else {
      // shift until we reach leading one at position 6
      while ((M & 0xFFC0) != 0x0040) {
        // shift the M value one place to the left
        M <<= 1;
        // decrement the exponent
        E--;
      }
      // check exponent for underflow
      if (E < -6) {
        // if sign S is 1 (negative), return negative zero
        // otherwise, return positive zero
        return S ? NZERO : PZERO;
      }
    }
    // now we have a valid exponent and a normalized 6-bit mantissa
    // call the function to round it and return the final value
    return roundAndBuildSmallFP(M, E, S, rounding_bit, sticky_bit);
  }
}

/* opSmallFP - Performs an operation on two SmallFP values
 *  - Follow the project documentation for this function.
 * Return the resulting smallfp_s value
 */
smallfp_s subSmallFP(smallfp_s val1, smallfp_s val2) {
  // define the sign exponent and mantissa of the result
  unsigned short S;
  short E;
  short M; // mantissa might be negative in the process of adding
  // get the sign, exponent and mantissa of both values
  unsigned short S1 = (val1 & 0x400) >> 10;
  short E1 = (val1 & 0x3C0) >> 6;
  short M1 = val1 & 0x3F;
  unsigned short S2 = (val2 & 0x400) >> 10;
  short E2 = (val2 & 0x3C0) >> 6;
  short M2 = val2 & 0x3F;

  // if either val1 or val2 are nan, return nan
  // exponent = 1111, mantisa different from 0
  if ((E1 == 0xF && M1 != 0) || (E2 == 0xF && M2 != 0)) {
    return NAN;
  }
  // else, if val1 is infinity, check val2 and return the result
  // accordingly
  else if (E1 == 0xF && M1 == 0) {
    // if val2 is also infinity, check the sign
    if (E2 == 0xF && M2 == 0) {
      // if signs are the same, return nan
      if (S1 == S2) {
        return NAN;
      }
      // otherwise, return infinity with the sign of val1
      else {
        S1 ? NINF : PINF;
      }
    }
    // else, return infinity with the sign of val1
    else {
      return S1 ? NINF : PINF;
    }
  }
  // else, if val2 is infinity, return infinity with the inverted sign of val2
  else if (E2 == 0xF && M2 == 0) {
    // no need to check for val1 infinity, as it was checked before
    // just return infinity value with the inverted sign of val2
    return S2 ? PINF : NINF;
  }
  // else, if val1 is zero, check val2 and return the result
  // accordingly
  else if (E1 == 0 && M1 == 0) {
    // if val2 is also zero, set the sign and return zero
    if (E2 == 0 && M2 == 0) {
      // sign will be negative only if S1 is negative and S2 is positive
      if (S1 == 1 && S2 == 0) {
        S = 1;
      }
      else {
        S = 0;
      }
      // return zero according to the sign
      return S ? NZERO : PZERO;
    }
    // otherwise, just return negative val2
    else {
      return (val2 ^ 0x400);
    }
  }
  // else, if val2 is zero, return val1
  else if (E2 == 0 && M2 == 0) {
    return val1;
  }
  // else, perform subtraction
  else {
    // define and initialize sticky and rounding bit
    unsigned short sticky_bit = 0;
    unsigned short rounding_bit = 0;
    // unbias the exponents
    E1 -= BIAS;
    E2 -= BIAS;
    // insert leading 1 to mantissas at position 6
    M1 |= 0x40;
    M2 |= 0x40;
    // if E1 is greater than E2, shift M1 until exponents become equal
    if (E1 > E2) {
      // shift it to the left by the difference of two exponents
      M1 <<= (E1 - E2);
      // set the resulting exponent to that of val2
      E = E2;
    }
    // else, if E2 is greater than E1, shift M2 until exponents become equal
    else if (E2 > E1) {
      // shift it to the left by the difference of two exponents
      M2 <<= (E2 - E1);
      // set the resulting exponent to that of val1
      E = E1;
    }
    // else, if they are equal, just set E to that of val1
    else {
      E = E1;
    }
    // negate M1 and M2 if their signs are negative
    M1 = S1 ? -M1 : M1;
    M2 = S2 ? -M2 : M2;
    // now subtract them
    M = M1 - M2;
    // if negative, set the sign and revert it back to absolute value
    if (M < 0) {
      S = 1;
      M = -M;
    }
    // else, if positive, set the sign to 0
    else {
      S = 0;
    }
    // now there is a possibility that it is not normalized due to
    // shifting and subtracting, the leading 1 could be on any side
    // so, first check if it is zero
    if (M == 0) {
      // if it is zero, return zero according to the sign
      return S ? NZERO : PZERO;
    }
    // else, if the leading one is somewhere to the left of the
    // fractional point, shift the M value to the right to find it
    else if ((M & 0xFFC0) != 0) {
      // shift until we reach leading one at position 6
      while ((M & 0xFFC0) != 0x0040) {
        // update the sticky bit with the previous rounding bit
        sticky_bit |= rounding_bit;
        // get the next rounding bit, before shifting
        rounding_bit = M & 0x1;
        // shift the M value one place to the right
        M >>= 1;
        // increment the exponent
        E++;
      }
      // check exponent for overflow
      if (E > 7) {
        // if sign S is 1 (negative), return negative infinity
        // otherwise, return positive infinity
        return S ? NINF : PINF;
      }
    }
    // else, the leading one must be to the right of the fractional
    // point, so shift the M value to the left
    else {
      // shift until we reach leading one at position 6
      while ((M & 0xFFC0) != 0x0040) {
        // shift the M value one place to the left
        M <<= 1;
        // decrement the exponent
        E--;
      }
      // check exponent for underflow
      if (E < -6) {
        // if sign S is 1 (negative), return negative zero
        // otherwise, return positive zero
        return S ? NZERO : PZERO;
      }
    }
    // now we have a valid exponent and a normalized 6-bit mantissa
    // call the function to round it and return the final value
    return roundAndBuildSmallFP(M, E, S, rounding_bit, sticky_bit);
  }
}

/* negSmallFP - Negates a SmallFP Value.
 *  - Follow the project documentation for this function.
 * Return the resulting SmallFP Value
 */
smallfp_s negSmallFP(smallfp_s value) {
  // just flip the sign bit at position 12 and return the same value
  return (value ^ 0x400);
}
