/*
 * noworky.c
 *
 * This program tries to create two arrays of data, and then swap their
 * contents. However, not all seems to go according to plan...
 */
#include <stdio.h>
#include <stdlib.h>

#define NUM_ELEMENTS 10

/*
 * Swap the contents of the two pointer arguments.
 */
void swapContent(double *d1, double *d2)
{
	double tmp = *d2;
	*d2 = *d1;
	*d1 = tmp;
}

/*
 * Swap the contents of the two arrays.
 */
void tradeArrays(double *array1, double *array2, int size)
{
	unsigned int i;
	for (i = size-1; i >= 0; i--) {
		swapContent(array1+i, array2+i);
	}
}

/*
 * Display the elements of two arrays.
 */
void displayArrays(double *source, double *target, int size)
{
	unsigned int i;
    for (i=0; i < size; i++) {
		printf("%3d: %05.1f --> %05.1f\n", i, source[i], target[i]);
	}
}

/*
 * Create two arrays; populate them; swap them; display them.
 */
int main()
{
	unsigned int i;
	double *source, *target;
	printf("noworky: by Brian Fraser\n");

	// Allocate the arrays:
	source = malloc(sizeof(*source) * NUM_ELEMENTS);
	target = malloc(sizeof(*target) * NUM_ELEMENTS);

	if (!source || !target) {
		printf("ERROR: Allocation failed.\n");
	}

	// Initialize the arrays
	for (i=0; i < NUM_ELEMENTS; i++) {
		source[i] = i * 2.0;
		target[i] = i * 10.0;
	}

	// Display them
	printf("Initial values: \n");
	displayArrays(source, target, NUM_ELEMENTS);

	// Swap their contents
	tradeArrays(source, target, NUM_ELEMENTS);

	// Display them
	printf("Final values: \n");
	displayArrays(source, target, NUM_ELEMENTS);

	// Clean up.
	free(source);
	free(target);

	printf("Done.\n");

	return 0;
}
