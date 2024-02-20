#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

void Usage(char*);
void readFile(FILE *fp, int *array);

void prefixSum(int *array);
void childSum(int *array, int startIndex, int chunkSize, int *prefixSums, int *flags, int steps, int identifier);
void calculatePrefixSum(int *input, int *output, int step, int startIndex, int endIndex);


uint16_t numElements;
uint16_t numCores;

int *array;

int main(int argc, char** argv) {
    if(argc != 5) {
        Usage(argv[0]);
    }

    FILE *inputp;
    FILE *outputp;

    numElements = atoi(argv[1]);
    numCores = atoi(argv[2]);

    if (numElements <= 0 || numCores <= 0) {
        fprintf(stderr, "Number of elements or cores less than 0.");
        exit(EXIT_FAILURE);
    }

    if (numCores > numElements) {
        fprintf(stderr, "Error: Number of cores defined (%d) greater than number of elements (%d)\n", numCores, numElements);
        exit(EXIT_FAILURE);
    }

    inputp = fopen(argv[3], "r");
    outputp = fopen(argv[4], "w");
    if (inputp == NULL || outputp == NULL) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    array = mmap(NULL, numElements * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    readFile(inputp, array);
    fclose(inputp);

    prefixSum(array);

    for(int i = 0; i < numElements; i++) {
        printf("%d", array[i]);
        printf("\n");
    }
    munmap(array, numElements * sizeof(int));
    fclose(outputp);

    return EXIT_SUCCESS;
}

void readFile(FILE *fp, int *array) {
    int numRead = 0;
    while (numRead < numElements && fscanf(fp, "%d", &array[numRead]) == 1) {
        numRead++;
    }

    if (numRead != numElements) {
        fprintf(stderr, "Error: Number of elements defined (%d) not equal to number of elements found (%d)\n", numElements, numRead);
        numElements = numRead;
        if (numCores > numElements) {
            fprintf(stderr, "Error: Number of cores defined (%d) greater than number of elements (%d)\n", numCores, numElements);
            exit(EXIT_FAILURE);
        }
    }


}

void calculatePrefixSum(int *input, int *output, int step, int startIndex, int endIndex) {
    for (int j = startIndex; j < endIndex; j++) {
        output[j] = input[j] + ((j >= step) ? output[j - step] : 0);
    }
}

void prefixSum(int *array) {
    int numSteps = (int)log2(numElements);
    int chunkSize = numElements/numCores;
    pid_t pid_array[numCores];

    int* prefixSums = mmap(NULL, numElements * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *flags = mmap(NULL, numCores * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    memset(prefixSums, 0, numElements * sizeof(int));
    memset(flags, 0, numCores * sizeof(int));


    for(int i = 0; i < numCores; i++) {
        pid_t pid = fork();
        pid_array[i] = pid;
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if(pid == 0) {
            int startIndex = i * chunkSize;
            childSum(array, startIndex, chunkSize, prefixSums, flags, numSteps, i);
            exit(EXIT_SUCCESS);
        }
    }
    int status;
    for(int i = 0; i < numCores; i++) {
        waitpid(pid_array[i], &status, 0);
    }

    munmap(prefixSums, numElements * sizeof(int));
    munmap(flags, numCores * sizeof(int));
}

void childSum(int *array, int startIndex, int chunkSize, int *prefixSums, int *flags, int steps, int identifier) {
    int endIndex = (identifier == numCores - 1) ? numElements : startIndex + chunkSize;

    for(int step = 1, prefixSteps = 1; step < steps; step++, prefixSteps *= 2) {
        calculatePrefixSum(array, prefixSums, prefixSteps, startIndex, endIndex);

        // oldest child rewrites input array to be the output array.
        if (identifier == 0) {
            for (int i = 1; i < numCores; i++) {
                while (flags[i] < step) {} // Wait for other children to complete current step.
            }

            for (int i = 0; i < numElements; i++) {
                array[i] = prefixSums[i];
            }

            flags[identifier] = step;   // oldest child is complete with everything.
        }

        else {
            flags[identifier] = step; // update barrier to signal completion of step[step].
            while (flags[0] < step); // wait for oldest to rewrite arrays.
        } //proceed to next step.
    }
}

void Usage(char* s) {
    fprintf(stderr, "Usage: %s num_elements num_cores input_file output_file");
    exit(EXIT_FAILURE);
}
