#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int length = 100;
int *array;

int* createArray(int length) {
    // initialize randomizer
    time_t t;
    srand((unsigned) time(&t));

    // malloc space for array
    int *array = malloc(length * sizeof(int));

    // fill array with random integers from 0 to length
    for(int i = 0; i < length; i++){
        array[i] = rand() % length;
    }
    return array;
}

void printArray(int* array, int length) {
    for(int i = 0; i < length; i++) {
        if(i != length-1){
            printf("%d, ", array[i]);
        }
        else{
            printf("%d\n", array[i]);
        }
    }
}

void sort(int* array, int length) {
    int placeholder;
    int swapped = 1;
    while(swapped == 1){
        swapped = 0;
        for(int i = 0; i < length-1; i++) {
            if(array[i] > array[i+1]) {
                placeholder = array[i];
                array[i] = array[i+1];
                array[i+1] = placeholder;
                swapped = 1;
            }
        }
    }
}

void freeArray(int* array) {
    free(array);
    array = NULL;
}

int Sort_getSize() {
    return length;
}

int* Sort_getArray() {
    printArray(array, length);
    return array;
}

int Sort_getValue(int value) {
    if(value-1 < 0 || value > length) {
        printf("Error! Value out of index.\n");
    }
    else{
        printf("value: %d\n", array[value-1]);
        return array[value-1];
    }
    return -1;
}

int main() {
    array = createArray(length);
    printArray(array, length);
    sort(array, length);
    printArray(array, length);
    Sort_getArray();
    Sort_getValue(1);
    Sort_getValue(2);
    Sort_getValue(3);
    Sort_getValue(4);
    Sort_getValue(101);
    Sort_getValue(0);
    freeArray(array);
}