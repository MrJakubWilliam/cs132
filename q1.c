#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Return an array of values that have a digital sum equal to the value given
long long unsigned *digitalSumNumbers(short inputVal)
{
    // Allocate memory space for the values that have a digital sum equal to the value given (inputVal)
    long long unsigned *outputArray = calloc(pow(2, inputVal - 1) + 1, sizeof(long long unsigned));

    // If 1 is given, return 1 as the only value with such digital sum
    if (inputVal == 1)
    {
        outputArray[0] = 1;
        return outputArray;
    }
    // Get an array of values that have a digital sum one less then the value given
    long long unsigned *arrayPrevious = digitalSumNumbers(inputVal - 1);
    size_t outputArrayCount = 0;
    size_t arrayPrevCount;

    // Iterate through the values that have a digital sum one less then the value given
    for (arrayPrevCount = 0; arrayPrevious[arrayPrevCount]; arrayPrevCount++)
    {
        // For each of the values: ...

        /* 
        * ... concatenate 1 to the end of it, and store the newly formed number
        * in an array containing numbers with digital sum equal to the value given
        */
        outputArray[outputArrayCount] = (arrayPrevious[arrayPrevCount] * 10) + 1;
        outputArrayCount++;

        if ((arrayPrevious[arrayPrevCount] % 10) + 1 <= 9)
        {
            /* 
            * ... add 1 to the value, if that's not going to cause the ten's 
            * placeholder to change (if the number end's with 9, we don't want 
            * to add 1, as that will cause the digital sum of the number to decrease).
            * Store the newly formed number in an array containing numbers with digital 
            * sum equal to the value given
            */
            outputArray[outputArrayCount] = arrayPrevious[arrayPrevCount] + 1;
            outputArrayCount++;
        }
    }

    free(arrayPrevious);
    return outputArray;
}

int main(int argc, char *argv[])
{
    float inputVal;

    if (argc == 2)
    {
        // If 1 command line argument was passed to the script
        // convert the users input to a float and store it in variable.
        // Not coverting to int, as that would not allow us to later check if actually
        // a float was passed (like 4.5) (only integers accepted).
        inputVal = atof(argv[1]);
    }
    else if (argc > 2)
    {
        // If too many command line arguments were passed to the
        // script, display a suitable error message
        printf("ERROR: Too many arguments! \n");
        return 1;
    }
    // If no arguments were passed to the script, prompt the user for the input value
    else
    {
        printf("Please input the value of n: ");
        scanf("%f", &inputVal);
    }

    // The script works for integer values in the range of 1 to 19 inclusive - check if
    // the value given by user satisfies that requirement
    if (floorf(inputVal) != inputVal || inputVal < 1 || inputVal > 19)
    {
        printf("ERROR: Only positive integers [1 - 19] allowed as input! \n");
        return 1;
    }
    // Get the values (without digit '0's) that have a digital sum equal to the input
    long long unsigned *outputArray = digitalSumNumbers(inputVal);
    long long unsigned total = 0;
    size_t outputArrayCount;

    // Sum up the values that have a digital sum equal to the input
    for (outputArrayCount = 0; outputArray[outputArrayCount]; outputArrayCount++)
        total += outputArray[outputArrayCount];

    // Show that sum to the user
    printf("%llu \n", total);
    free(outputArray);

    return 0;
}