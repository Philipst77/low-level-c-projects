
 
#include <stdio.h>
#include <stdlib.h>
#include "common_structs.h"
#include "common_definitions.h"
#include "common_functions.h"
#include "testing_support.h"
#include "smallfp.h"

// Prototypes
void test_negate();
static void print_macro_demo();

// Function Definitions
int main() {
  print_macro_demo(); // Comment me out to get rid of the demonstration messages
  test_negate();

  return 0;
}

// Example testing function for negSmallFP
void test_negate() {
  PRINT_INFO("Beginning test on negSmallFP()");

  PRINT_STATUS("Testing negSmallFP(0x1c0) // Value 1.00");
  smallfp_s val = 0x1c0; // From ref_all_values, this should equal 1.00
  val = negSmallFP(val);
  // With S == 1, val should now equal 0x160  (1 0111  000000 = 0101 1100 0000 = 0x5c0)
  //                                           S  exp  frac
  if(val != 0x5c0) {
    PRINT_WARNING("Expected negSmallFP(0x1c0) to return 0x5c0, but got 0x%03X instead.", val);
  }

  PRINT_STATUS("Testing negSmallFP(0x5c0) // Value -1.00");
  val = 0x5c0; // From ref_all_values, but with S == 1, this should equal -1.00
  val = negSmallFP(val);
  // With S == 0, val should now equal 0x1c0  (0 0111  000000 = 0001 1100 0000 = 0x1c0)
  //                                           S  exp  frac
  if(val != 0x1c0) {
    PRINT_WARNING("Expected negSmallFP(0x5c0) to return 0x1c0, but got 0x%03X instead.", val);
  }
}

// Demo of some macros you are free to use if you like.
static void print_macro_demo() {
  PRINT_INFO("You can print an info message like using printf.  %d", 42);
  PRINT_STATUS("You can print a status message like using printf.  %d", 42);
  PRINT_WARNING("You can print a warning message like using printf.  %d", 42);
  MARK("You can mark a line with a message like using printf.  %d", 42);
}
