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

// Return an array of values that have a digital sum equal to the value given
long unsigned *digitalSumElements(int inputVal)
{
    // Allocate memory space for the values that have a digital sum equal to the value given (inputVal)
    long unsigned *outputArray = calloc(pow(2, inputVal - 1) + 1, sizeof(long unsigned));

    // If 1 is given, return 1 as the only value with such digital sum
    if (inputVal == 1)
    {
        outputArray[0] = 1;
        return outputArray;
    }
    // Get an array of values that have a digital sum one less then the value given
    long unsigned *decreasedArray = digitalSumElements(inputVal - 1);
    long unsigned *decreasedArrayElement = decreasedArray;
    long unsigned counter = 0;

    // Iterate through the values that have a digital sum one less then the value given
    for (decreasedArrayElement; *decreasedArrayElement; decreasedArrayElement++)
    {
        // For each of the values: ...

        /* 
        * ... concatenate 1 to the end of it, and store the newly formed number
        * in an array containing numbers with digital sum equal to the value given
        */
        outputArray[counter] = concatenate(*decreasedArrayElement, 1);
        counter++;

        if ((*decreasedArrayElement % 10) + 1 <= 9)
        {
            /* 
            * ... add 1 to the value, if that's not going to cause the ten's 
            * placeholder to change (if the number end's with 9, we don't want 
            * to add 1, as that will cause the digital sum of the number to decrease).
            * Store the newly formed number in an array containing numbers with digital 
            * sum equal to the value given
            */
            outputArray[counter] = *decreasedArrayElement + 1;
            counter++;
        }
    }

    free(decreasedArray);
    return outputArray;
}

int main(int argc, char *argv[])
{
    int inputVal;

    if (argc == 2)
    {
        inputVal = atoi(argv[1]);
    }
    else if (argc > 2)
    {
        printf("ERROR: Too many arguments! \n");
        return 1;
    }
    // If no arguments were passed to the script, prompt the user for the input value
    else
    {
        printf("Please input the value of n: ");
        scanf("%d", &inputVal);
    }
    // The script works for values in the range of 1 to 19 inclusive - check if
    // the value given by user fits in that range
    if (inputVal < 1 || inputVal > 19)
    {
        printf("ERROR: Only positive integers [1 - 19] allowed as input! \n");
        return 1;
    }

    long unsigned *outputArrayPointer = digitalSumElements(inputVal);

    // create a copy of the pointer to the output array so that we can free that allocated space
    // at the end to avoid any memory leaks.
    long unsigned *outputArrayPointerCopy = outputArrayPointer;

    long unsigned total = 0;

    // Sum up the values that have a digital sum equal to the input
    for (outputArrayPointer; *outputArrayPointer; outputArrayPointer++)
        total += *outputArrayPointer;

    // Show that sum to the user
    printf("%lu \n", total);
    free(outputArrayPointerCopy);

    return 0;
}