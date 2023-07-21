#include <stdio.h>
#include <stdint.h>


#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif


int
main(void) {
    int i;

    printf("Starting...\n");

    for (i = 0; i < 5; i++) {
	  printf("foo: %d\n", i);
    }
    while (1);
}
