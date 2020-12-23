#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Concatenate two integers and return the result as integer.
long unsigned concatenate(long unsigned x, long unsigned y)
{
    int unsigned pow = 10;

    while (y >= pow)
        pow *= 10;

    return x * pow + y;
}

// Return an array of values
long unsigned *digitalSumElements(long unsigned *array, size_t arraySize)
{
    long unsigned *outputArray = calloc((pow(2, arraySize - 1) + 1), sizeof(long unsigned));

    if (arraySize == 1)
    {
        outputArray[0] = 1;
        outputArray[1] = 0;

        return outputArray;
    }

    size_t lowerArraySize = arraySize - 1;
    size_t upperArraySize = arraySize - lowerArraySize;

    long unsigned lowerArray[lowerArraySize];
    long unsigned upperArray[upperArraySize];

    for (int i = 0; i < arraySize; i++)
    {
        if (i < lowerArraySize)
            lowerArray[i] = 1;
        else
            upperArray[i - lowerArraySize] = 1;
    }

    long unsigned *lowerArrayProcessed = digitalSumElements(lowerArray, lowerArraySize);
    long unsigned *upperArrayProcessed = digitalSumElements(upperArray, upperArraySize);
    long unsigned *lowerArrayElement = lowerArrayProcessed;
    long unsigned counter = 0;

    while (*lowerArrayElement != 0)
    {
        long unsigned *upperArrayElement = upperArrayProcessed;

        while (*upperArrayElement != 0)
        {
            outputArray[counter] = concatenate(*lowerArrayElement, *upperArrayElement);
            counter++;

            if ((*lowerArrayElement % 10) + (*upperArrayElement % 10) <= 9)
            {
                outputArray[counter] = *lowerArrayElement + *upperArrayElement;
                counter++;
            }

            upperArrayElement++;
        }

        lowerArrayElement++;
    }

    free(lowerArrayProcessed);
    free(upperArrayProcessed);

    outputArray[counter] = 0;

    return outputArray;
}

int main(int argc, char *argv[])
{
    int inputArraySize;
    // int *inputArraySizePointer = &inputArraySize;

    if (argc == 2)
    {
        inputArraySize = atoi(argv[1]);
    }
    else if (argc > 2)
    {
        printf("ERROR: Too many arguments! \n");
        return 1;
    }
    else
    {
        printf("Please input the value of n: ");
        scanf("%d", &inputArraySize);
    }

    if (inputArraySize < 1 || inputArraySize > 19)
    {
        printf("ERROR: Only positive integers [1 - 19] allowed as input! \n");
        return 1;
    }

    long unsigned inputArray[inputArraySize];

    for (int i = 0; i < inputArraySize; i++)
    {
        inputArray[i] = 1;
    }

    long unsigned *outputArrayPointer = digitalSumElements(inputArray, inputArraySize);
    long unsigned total = 0;

    while (*outputArrayPointer != 0)
    {
        total += *outputArrayPointer;
        outputArrayPointer++;
    }

    printf("%lu \n", total);

    return 0;
}