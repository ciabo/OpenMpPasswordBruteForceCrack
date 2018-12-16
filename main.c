#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include <crypt.h>
#include <string.h>
#include <omp.h>

#define dim 100

int main() {
    int dictionary[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    omp_lock_t lock;
    omp_init_lock(&lock);
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *hashes[dim];

    int k = 0;
    double count=0;
    fp = fopen("../PswDb/db100.txt", "r");
    while ((read = getline(&line, &len, fp)) != -1) {
        char *hash = (char *) malloc(sizeof(char) * 14);
        for (int i = 0; i < 13; i++) {
            hash[i] = line[i + 9];
        }
        hash[13] = '\0'; //string termination
        hashes[k] = hash;
        k++;
    }
    fclose(fp);
    free(line);

    double  start = omp_get_wtime();
#pragma omp parallel
    {
    struct crypt_data data;
    data.initialized = 0;
#pragma omp for
    for(int i=0;i<=dim-1;i++) {

        for (int y = 1940; y < 2011; y++) {
            for (int m = 1; m < 13; m++) {
                for (int d = 1; d < 32; d++) {
                    int mI = m;
                    int yI = y;
                    int dI = d;

                    //conversion from int to char
                    // --------FASTER THAN sprintf WAY ---------

                    //days
                    char dd[2];
                    if (dI < 10) {
                        dd[0] = '0';
                        dd[1] = dictionary[dI];
                    } else {
                        int tens = dI / 10;
                        dd[0] = dictionary[tens];
                        int units = dI % 10;
                        dd[1] = dictionary[units];
                    }
                    //months
                    char mm[2];
                    if (mI < 10) {
                        mm[0] = '0';
                        mm[1] = dictionary[mI];
                    } else {
                        int tens = mI / 10;
                        mm[0] = dictionary[tens];
                        int units = mI % 10;
                        mm[1] = dictionary[units];
                    }
                    //years -suppose yI=1996
                    char yyyy[4];
                    int thousands = yI / 1000; //yI/1000=1 poichè è int
                    yyyy[0] = dictionary[thousands];
                    int tmp = yI % 1000; //yI%1000=996
                    int hundreds = tmp / 100; //996/100 = 9
                    yyyy[1] = dictionary[hundreds];
                    tmp = tmp % 100; // 996%100=96
                    int tens = tmp / 10;//96/10=9
                    yyyy[2] = dictionary[tens];
                    tmp = tmp % 10;//96%10 = 6
                    int units = tmp;//6
                    yyyy[3] = dictionary[units];

                    //end conversion
                    /*
                    char dd[2];
                    sprintf(dd, "%02d", d);
                    char mm[2];
                    sprintf(mm, "%02d", m);
                    char yyyy[4];
                    sprintf(yyyy, "%04d", y);
                    */
                    char yyyymmdd[9] = {yyyy[0], yyyy[1], yyyy[2], yyyy[3], mm[0], mm[1], dd[0], dd[1], '\0'};
                    char* newHash=crypt_r(yyyymmdd,"parallel\0",&data);
                    if (strcmp(hashes[i],newHash)==0) {
                        count++;
                        goto stop;
                    }
                }
            }
        }

        stop:;

    }
}
    double end = omp_get_wtime();
    printf("\n Time: %fs \n", end-start);
    printf("%f",count);
    omp_destroy_lock(&lock);
    return 0;
}