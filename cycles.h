// cycles.h            see license.txt for copyright and terms of use
// Report total number of processor cycles since the machine was turned
// on.  Uses the RDTSC instruction on x86.  This hasn't been updated to
// work with x86-64 and RDTSC is not very useful anyway.

#ifdef __cplusplus
extern "C" {
#endif


// read the processor's cycle-count register, and store the count
// into these two 'unsigned' variables; if the count isn't available,
// yields zero in both
void getCycles(unsigned *lowp, unsigned *highp);


#ifdef __GNUC__
// if we're using gcc, so the 'long long' type is available,
// here's a more convenient version
unsigned long long getCycles_ll(void);
#endif


#ifdef __cplusplus
}
#endif

