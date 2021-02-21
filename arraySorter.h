#ifndef _ARRAYSORTER_
#define _ARRAYSORTER_

// Initialize sorting module
void arraySorter_init();

// Shutdown sorting module
void arraySorter_shutdown();

// Returns current length of arrays being sorted
int arraySorter_getSize();

// Return array currently being sorted
void arraySorter_getArray();

// Return [value]th value in array
int arraySorter_getValue(int value);

// Return the total number of sorts so far
long arraySorter_getTotalSorts();

#endif