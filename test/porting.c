#undef PROFILE
#define FPOINT_DATA_TYPE float
#include "porting.h"

#define MAX_ENTRIES 150
#define MAX_ACTIVITIES 20
#define KNN_NEIGHBORS 5

#define ENTRY_ANNOTATION                                                       \
    __attribute((annotate(                                                     \
        "struct[void,void,scalar(range(-25,25)),scalar(range(-30,30)),scalar(" \
        "range("                                                               \
        "-20,20)),scalar(range(-200,200)),scalar(range(-60,60)),void]")))

#define WANTED_LABEL "[u'FALLING']"

/*#define ENTRY_ANNOTATION __attribute((annotate(\
    "struct[void,void,scalar(range(0,25)),scalar(range(10,30)),scalar(range("\
    "5,20)),scalar(range(0,200)),scalar(range(0,60)),void]")))*/
/*Entry */
Entry ENTRY_ANNOTATION entries[MAX_ENTRIES];
int ent_n = 0;
char *activities[MAX_ACTIVITIES];
char *fallingPtr = NULL;
int act_n = 0;
float prova __attribute((annotate("target('prova') scalar(range(-25,25) error(0.01))")));;

#ifndef _MIOSIX
int main(int argc, char *argv[]) {
    run(argc, argv);
    return 0;
}


#else

#include "miosix.h"
#include <cstdio>
#include <unistd.h>

using namespace std;
using namespace miosix;

int main() {
    int argc = 2;
    char *argv[] = {"miosixProgram", "/sd/exported.csv"};
    run(argc, argv);
    for (;;)
        ;
}

#endif

int run(int argc, char *argv[]) {
    char *predictedLabels[MAX_ENTRIES];

    srand(1235);

    if (argc < 2) {
        printf("Gimme the file!");
        return -1;
    }

    // printf("Opening file %s...\n", argv[1]);

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error opening the file!\n");
        return -1;
    }

    // printf("Reading data from file...\n");

    for (int times = 0; times < 100; times++) {
        printf("%d, ", times);
        rewind(file);
        ent_n = readData(file);

        // printf("Got %d entries.\nGot %d different activities.\n", ent_n,
        // act_n);

        int train = 0;
        for (int i = 0; i < ent_n; i++) {
            if (entries[i].role)
                train++;
            // printEntry(entries[i]);
        }

        // printf("%d/%d (%.2f%%) used for training.\n", train, ent_n, train *
        // 100.0, ent_n);

        /*for (int i = 0; i < ent_n; i++) {
            printEntry(entries[i]);
        }*/

        // BEGIN OF COMPUTATION
        clock_t t;
        t = clock();

        // printf("Launching MinMaxScaler...\n");
        minMaxScaler();
        // printf("Scaled.\n");

        printf("Running classification...\n");
        //for(;;){
        for (int i = 0; i < ent_n; i++) {
            // Classify all cases
            predictedLabels[i] = KNNClassify(i);
            //printf("%s\t%s\n", predictedLabels[i], entries[i].activity);
        }
        //}

        // END OF COMPUTATION
        double time_taken =
            ((double)clock() - t) * 1000 / CLOCKS_PER_SEC; // in milliseconds
        printf("%lf, ", time_taken);

        // Statistics...
        int train_correct = 0;
        int test_correct = 0;
        for (int i = 0; i < ent_n; i++) {
            if (entries[i].activity == predictedLabels[i]) {
                if (entries[i].role) { // Train
                    train_correct++;
                } else { // Test
                    test_correct++;
                }
            }
        }

        /*for (int i = 0; i < ent_n; i++) {
            printEntry(entries[i]);
        }*/

        // printf("Performances on Train data: %d/%d (%lf)\n", train_correct,
        // train, ((FPOINT_DATA_TYPE)train_correct) / train);
        // printf("Performances on Test data: %d/%d (%lf)\n", test_correct,
        // ent_n - train, ((FPOINT_DATA_TYPE)test_correct) / (ent_n - train));
        printf("%d, %d, ", train_correct, train);        // on train data
        printf("%d, %d ", test_correct, ent_n - train); // On test data

        /*fallingPtr = getActivityPtr(WANTED_LABEL);

        int train_falling = 0;
        int test_falling = 0;
        for (int i = 0; i < ent_n; i++) {
            if (entries[i].activity == fallingPtr) {
                if (entries[i].role) { // Train
                    train_falling++;
                } else { // Test
                    test_falling++;
                }
            }
        }

        int train_falling_TP = 0;
        int train_falling_TN = 0;
        int train_falling_FP = 0;
        int train_falling_FN = 0;
        int test_falling_TP = 0;
        int test_falling_TN = 0;
        int test_falling_FP = 0;
        int test_falling_FN = 0;
        for (int i = 0; i < ent_n; i++) {
            // FALSE NEGATIVE
            if (entries[i].activity == fallingPtr &&
                predictedLabels[i] != fallingPtr) {
                if (entries[i].role) { // Train
                    train_falling_FN++;
                } else { // Test
                    test_falling_FN++;
                }
            }

            // FALSE POSITIVE
            if (entries[i].activity != fallingPtr &&
                predictedLabels[i] == fallingPtr) {
                if (entries[i].role) { // Train
                    train_falling_FP++;
                } else { // Test
                    test_falling_FP++;
                }
            }

            // OK
            if (entries[i].activity == fallingPtr &&
                predictedLabels[i] == fallingPtr) {
                if (entries[i].role) { // Train
                    train_falling_TP++;
                } else { // Test
                    test_falling_TP++;
                }
            }

            // OK
            if (entries[i].activity != fallingPtr &&
                predictedLabels[i] != fallingPtr) {
                if (entries[i].role) { // Train
                    train_falling_TN++;
                } else { // Test
                    test_falling_TN++;
                }
            }
        }
        
        
        printf("%d, %d, %d, %d, ", train_falling_TP, train_falling_FN, train_falling_FP, train_falling_TN);
        printf("%d, %d, %d, %d\n", test_falling_TP, test_falling_FN, test_falling_FP, test_falling_TN);*/
        printf("\n");
    }

#ifdef PROFILE
    printMinMaxValues();
#endif
    return 0;
}

int readData(FILE *file) {
    char line[1024];
    char *readed = fgets(line, sizeof(line), file);
    int lines = 0;

    do {
        char *ptr = strchr(line, '\n');
        if (ptr) {
            *ptr = '\0';
        } else {
            printf("Warning: end of line not reached on line %d\n", lines);
        }

        int tokenId = 0;
        Entry ENTRY_ANNOTATION *temp;
        if (!lines) {
            goto NEXTLINE;
        }

        // Entry *temp = malloc(sizeof(Entry));
        // memset(temp, 0, sizeof(Entry));
        // entries[lines-1] = temp;
        temp = &entries[lines - 1];

        double tempDouble;

        for (char *token = strtok(line, ","); token != NULL;
             token = strtok(NULL, ",")) {
            switch (tokenId) {
            case 0:
                sscanf(token, "%d", &(temp->n));
                break;
            case 1:
                temp->activity = getActivityPtr(token);
                break;
            case 2:
                sscanf(token, "%lf", &(tempDouble));
                temp->SMV = tempDouble;
#ifdef PROFILE
                setMinMaxValue(temp->SMV, 0);
#endif
                break;
            case 3:
                sscanf(token, "%lf", &(tempDouble));
                temp->SMA = tempDouble;
#ifdef PROFILE
                setMinMaxValue(temp->SMA, 1);
#endif
                break;
            case 4:
                sscanf(token, "%lf", &(tempDouble));
                temp->AI = tempDouble;
#ifdef PROFILE
                setMinMaxValue(temp->AI, 2);
#endif
                break;
            case 5:
                sscanf(token, "%lf", &(tempDouble));
                temp->VI = tempDouble;
#ifdef PROFILE
                setMinMaxValue(temp->VI, 3);
#endif
                break;
            case 6:
                sscanf(token, "%lf", &(tempDouble));
                temp->MMZ = tempDouble;
#ifdef PROFILE
                setMinMaxValue(temp->MMZ, 4);
#endif
                break;
            default:
                printf("Warning: more tokens than expected: %s\n", token);
                exit(-1);
            }

            tokenId++;
        }

        // temp->role = rand() % 4 > 0 ? 1 : 0; // 75% train (1), 25% test (0)
        temp->role = 0;
    NEXTLINE:
        readed = fgets(line, sizeof(line), file);
        lines++;
    } while (readed != NULL && lines - 1 < MAX_ENTRIES);

    Entry ENTRY_ANNOTATION swap;
    for (int i = 0; i < lines - 1; i++) {
        int elId = rand() % (lines - 1);
        swap = entries[i];
        entries[i] = entries[elId];
        entries[elId] = swap;
    }
    for (int i = 0; i < (lines - 1); i++) {
        if (i < (lines - 1) * 0.75) {
            entries[i].role = 1;
        } else {
            entries[i].role = 0;
        }
    }

    return lines - 1;
}

void printEntry(Entry ENTRY_ANNOTATION entry) {
    printf("Entry: %d, %s, %lf, %lf, %lf, %lf, %lf, role: %d\n", entry.n,
           entry.activity, entry.SMV, entry.SMA, entry.AI, entry.VI, entry.MMZ,
           entry.role);
}

char *getActivityPtr(char *givenActivity) {
    int num = getActivityNumber(givenActivity);
    if (num >= 0) {
        return activities[num];
    }

    // printf("Allocating new activity for %s... ", givenActivity);
    activities[act_n] =
        (char *)malloc((strlen(givenActivity) + 1) * sizeof(char));
    // printf("Allocated at 0x%x\n", activities[act_n]);
    memcpy(activities[act_n], givenActivity, strlen(givenActivity) + 1);

    act_n++;

    return activities[act_n - 1];
}

int getActivityNumber(char *givenActivity) {
    for (int i = 0; i < act_n; i++) {
        if (!strcmp(givenActivity, activities[i])) {
            return i;
        }
    }
    return -1;
}

void minMaxScaler() {
    FPOINT_DATA_TYPE minSMV
        __attribute((annotate("target('minSMV') scalar(range(-25,25))")));
    FPOINT_DATA_TYPE minSMA
        __attribute((annotate("target('minSMA') scalar(range(-30,30))")));
    FPOINT_DATA_TYPE minAI
        __attribute((annotate("target('minAI') scalar(range(-20,20))")));
    FPOINT_DATA_TYPE minVI
        __attribute((annotate("target('minVI') scalar(range(-200,200))")));
    FPOINT_DATA_TYPE minMMZ
        __attribute((annotate("target('minMMZ') scalar(range(-60,60))")));
    FPOINT_DATA_TYPE maxSMV
        __attribute((annotate("target('maxSMV') scalar(range(-25,25))")));
    FPOINT_DATA_TYPE maxSMA
        __attribute((annotate("target('maxSMA') scalar(range(-30,30))")));
    FPOINT_DATA_TYPE maxAI
        __attribute((annotate("target('maxAI') scalar(range(-20,20))")));
    FPOINT_DATA_TYPE maxVI
        __attribute((annotate("target('maxVI') scalar(range(-200,200))")));
    FPOINT_DATA_TYPE maxMMZ
        __attribute((annotate("target('maxMMZ') scalar(range(-60,60))")));
    short init = 0;

    for (int i = 0; i < ent_n; i++) {
        if (entries[i].role) {
            if (!init) {
                init = 1;
                minSMV = entries[i].SMV;
                maxSMV = entries[i].SMV;
                minSMA = entries[i].SMA;
                maxSMA = entries[i].SMA;
                minAI = entries[i].AI;
                maxAI = entries[i].AI;
                minVI = entries[i].VI;
                maxVI = entries[i].VI;
                minMMZ = entries[i].MMZ;
                maxMMZ = entries[i].MMZ;
            } else {
                if (entries[i].SMV < minSMV)
                    minSMV = entries[i].SMV;
                if (entries[i].SMV > maxSMV)
                    maxSMV = entries[i].SMV;

                if (entries[i].SMA < minSMA)
                    minSMA = entries[i].SMA;
                if (entries[i].SMA > maxSMA)
                    maxSMA = entries[i].SMA;

                if (entries[i].AI < minAI)
                    minAI = entries[i].AI;
                if (entries[i].AI > maxAI)
                    maxAI = entries[i].AI;

                if (entries[i].VI < minVI)
                    minVI = entries[i].VI;
                if (entries[i].VI > maxVI)
                    maxVI = entries[i].VI;

                if (entries[i].MMZ < minMMZ)
                    minMMZ = entries[i].MMZ;
                if (entries[i].MMZ > maxMMZ)
                    maxMMZ = entries[i].MMZ;
            }
        }
    }

    for (int i = 0; i < ent_n; i++) {
        entries[i].SMV = (entries[i].SMV - minSMV) / (maxSMV - minSMV);
        entries[i].SMA = (entries[i].SMA - minSMA) / (maxSMA - minSMA);
        entries[i].AI = (entries[i].AI - minAI) / (maxAI - minAI);
        entries[i].VI = (entries[i].VI - minVI) / (maxVI - minVI);
        entries[i].MMZ = (entries[i].MMZ - minMMZ) / (maxMMZ - minMMZ);
    }
}

__attribute((annotate("target('computeDistance') scalar(range(-50,50))")))
FPOINT_DATA_TYPE
computeDistance(Entry ENTRY_ANNOTATION a, Entry ENTRY_ANNOTATION b) {
    FPOINT_DATA_TYPE res =
        ((a.SMV - b.SMV) * (a.SMV - b.SMV) + (a.SMA - b.SMA) * (a.SMA - b.SMA) +
         (a.AI - b.AI) * (a.AI - b.AI) + (a.VI - b.VI) * (a.VI - b.VI) +
         (a.MMZ - b.MMZ) * (a.MMZ - b.MMZ));
    /*FPOINT_DATA_TYPE smv = (a.SMV - b.SMV) * (a.SMV - b.SMV);
    FPOINT_DATA_TYPE sma = (a.SMA - b.SMA) * (a.SMA - b.SMA);
    FPOINT_DATA_TYPE ai = (a.AI - b.AI) * (a.AI - b.AI);
    FPOINT_DATA_TYPE vi = (a.VI - b.VI) * (a.VI - b.VI);
    FPOINT_DATA_TYPE mmz = (a.MMZ - b.MMZ) * (a.MMZ - b.MMZ);*/
    return res;
}

char *KNNClassify(int toClassify) {
    int scores[MAX_ACTIVITIES];
    char *local_labels[KNN_NEIGHBORS];
    FPOINT_DATA_TYPE __attribute((annotate("scalar(range(-50,50))")))
    local_distances[KNN_NEIGHBORS];

    // cleanup (maybe bug of llvm?)
    for (int i = 0; i < MAX_ACTIVITIES; i++) {
        scores[i] = 0;
    }
    for (int i = 0; i < KNN_NEIGHBORS; i++) {
        local_labels[i] = NULL;
        local_distances[i] = -1;
    }

    for (int i = 0; i < ent_n; i++) {
        if (entries[i].role) { // is train case
            FPOINT_DATA_TYPE __attribute((annotate("scalar(range(-50,50))")))
            distance = computeDistance(entries[i], entries[toClassify]);
            // printf("(%d) %f\n", i, distance);
            insertInPriorityQueue(local_distances, distance, local_labels, i);
        }
    }

    // for(;;);

    for (int i = 0; i < KNN_NEIGHBORS; i++) {
        int actn = getActivityNumber(local_labels[i]);
        scores[actn]++;
    }
    // for(;;);

    char *indexMax = NULL;
    int localMax = -1;
    for (int i = 0; i < MAX_ACTIVITIES; i++) {
        if (scores[i] > localMax) {
            indexMax = activities[i];
            localMax = scores[i];
        }
    }
    return indexMax;
}

void insertInPriorityQueue(FPOINT_DATA_TYPE
                           __attribute((annotate("scalar(range(-50,50))")))
                           local_distances[],
                           FPOINT_DATA_TYPE
                           __attribute((annotate("scalar(range(-50,50))")))
                           distance,
                           char *local_labels[], int index) {
    for (int i = 0; i < KNN_NEIGHBORS; i++) {
        if (distance < local_distances[i] || local_distances[i] == -1) {
            for (int j = KNN_NEIGHBORS - 1; j > i; j--) {
                local_distances[j] = local_distances[j - 1];
                local_labels[j] = local_labels[j - 1];
            }

            local_distances[i] = distance;
            local_labels[i] = entries[index].activity;
            return;
        }
    }
}
