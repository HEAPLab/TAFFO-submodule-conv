#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int n;
    char *activity;
    FPOINT_DATA_TYPE SMV;
    FPOINT_DATA_TYPE SMA;
    FPOINT_DATA_TYPE AI;
    FPOINT_DATA_TYPE VI;
    FPOINT_DATA_TYPE MMZ;
    int role;
} Entry;

int readData(FILE *file);
void printEntry(Entry entry);
char *getActivityPtr(char *givenActivity);
int getActivityNumber(char *givenActivity);
void minMaxScaler();
char *KNNClassify(int index);
int run(int argc, char *argv[]);

void insertInPriorityQueue(FPOINT_DATA_TYPE local_distances[],
                           FPOINT_DATA_TYPE distance, char *local_labels[],
                           int index);

#ifdef PROFILE
#define MINMAX_VALUES 20
FPOINT_DATA_TYPE minValues[MINMAX_VALUES];
FPOINT_DATA_TYPE maxValues[MINMAX_VALUES];

int minMaxInitialized = 0;

void initializeMinMax() {
    for (int i = 0; i < MINMAX_VALUES; i++) {
        minValues[i] = DBL_MAX;
        maxValues[i] = -DBL_MAX;
    }
    minMaxInitialized = 1;
}

void setMinMaxValue(FPOINT_DATA_TYPE value, int index) {
    if (!minMaxInitialized)
        initializeMinMax();
    assert(index >= 0 && index < MINMAX_VALUES);

    if (value < minValues[index])
        minValues[index] = value;
    if (value > maxValues[index])
        maxValues[index] = value;
}

void printMinMaxValues() {
    for (int i = 0; i < MINMAX_VALUES; i++) {
        printf("Index %d: [%lf, %lf]\n", i, minValues[i], maxValues[i]);
    }
}
#endif
