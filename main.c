#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>

void Usage(char*);
void readFile(FILE *fp, int *array);

void prefixSum(int *array);
void claculatePrefixSum(int *input, int *output, int step, int startIndex, int endIndex);


uint16_t numElements;
uint16_t numCores;

int main(int argc, char** argv) {
    if(argc != 5) {
        Usage(argv[0]);
    }

    FILE *inputp;
    FILE *outputp;

    numElements = atoi(argv[1]);
    numCores = atoi(argv[2]);

    if (numElements <= 0 || numCores <= 0) {
        perror("Number of elements or cores less than 0.");
    }

    inputp = fopen(argv[3], "r");
    outputp = fopen(argv[4], "w");
    if (inputp == NULL || outputp == NULL) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }


    fclose(inputp);
    fclose(outputp);

    return EXIT_SUCCESS;
}

void readFile(FILE *fp, int *array) {
    int numRead = 0;
    while (numRead < numElements && fscanf(fp, "%d", &array[numRead]) == 1) {
        numRead++;
    }
}

void calculatePrefixSum(int *input, int *output, int step, int startIndex, int endIndex) {
    for (int j = startIndex; j < endIndex; j++) {
        output[j] = input[j] + ((j >= step) ? output[j - step] : 0);
    }
}

void prefixSum(int *array) {
    int numSteps = (int)log2(numElements);

    for(int i = 0; i < numCores; i++) {
        pid_t pid;
        pid = fork();

        switch(pid) {
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);
            case 0:

        }
    }
}





void Usage(char* s) {
    fprintf(stderr, "Usage: %s num_elements num_cores input_file output_file");
    exit(EXIT_FAILURE);
}
