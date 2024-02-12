#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


void Usage(char*);
int* readFile(FILE *fp);


uint16_t numElements;
uint16_t numCores;
int* ints;


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

    ints = readFile(inputp);
    if(ints == NULL){
        fclose(inputp);
        fclose(outputp);
        free(ints);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numElements; i++) {
        printf("%d ", ints[i]);
    }

    fclose(inputp);
    fclose(outputp);
    free(ints);
    return EXIT_SUCCESS;
}

int* readFile(FILE *fp) {
    int* integers = (int*)malloc(numElements * sizeof(int)); // Allocate memory for the integers array
    if (integers == NULL) {
        perror("Error allocating memory");
        return NULL;
    }

    int numRead = 0;
    while (fscanf(fp, "%d", &integers[numRead]) == 1) {
        numRead++;
    }

    return integers;
}




void Usage(char* s) {
    fprintf(stderr, "Usage: %s num_elements num_cores input_file output_file");
    exit(EXIT_FAILURE);
}
