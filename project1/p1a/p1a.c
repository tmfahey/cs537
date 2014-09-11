#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc != 5) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
        fprintf(stderr, "Usage: paradox -i inputfile -o outputfile\n");
        exit(0);
    } else { // if we got enough parameters...
        int i, j, k;
        char inputFile[100];
        char outputFile[100];
        char buf[1024];
        int  entries, positiveTrials;
        bool positiveTrial;
        FILE *input_file;
        FILE *output_file;

        for(i = 1; i < argc; i++) {  //iterate over argv to get the parameters inside.
            if (i + 1 != argc) // Check that we are not finished parsing
                if (strcmp(argv[i], "-i") == 0){
                    //next arg is inputFile
                    sprintf(inputFile, "%s", argv[i + 1]);
                } else if (strcmp(argv[i], "-o") == 0) {
                    //next arg is outputFile
                    sprintf(outputFile, "%s", argv[i + 1]);
                } else {
                    printf("Not enough or invalid arguments, please try again.\n");
                    exit(0);
                }
            //skip over filename
            i = i + 1;
        }
        input_file = fopen(inputFile, "r");
        if (input_file == 0)
        {
            //fopen returns 0, the NULL pointer, on failure
            fprintf(stderr, "Error: Cannot open file %s\n", inputFile);
            exit(-1);
        }

        output_file = fopen("outputFile", "w");
        if (output_file == NULL)
        {
            fprintf(stderr, "Error: Cannot open file %s\n", outputFile);
            exit(1);
        }

        //seed using time
        srand(time(NULL));

        while (fgets(buf, sizeof(buf), input_file)!=NULL){
          //calculating all of the probabilities
          int trials;
          int appearCount;
          int birthdays[1000];
          int people;
          int l;
          double result;
          trials = 1000;
          people = 0;
          appearCount = 0;
          positiveTrials = 0;
          
          //storing number of trials
          sscanf(buf, "%d", &people);

          //ensure people greater than zero
          if(people > 0){
            //running trials
            for(i = 0; i<1000; i++){    //we haven't yet found a positive trial
              positiveTrial = false;
              for(l = 0; l < people; l++){    //populate birthdays
                 birthdays[l] = rand() % 365;
              }
              //iterate through days until positive trial
              for(j = 0; j < 365 && !positiveTrial; j++){
                //ensure appearCount is 0
                appearCount = 0;
                //check to see if multiple people have this birthday
                for(k = 0; k < people; k++){
                  if(j == birthdays[k]){
                    //someone has this birthday, inc appearCount
                    appearCount++;
                  }
                }
                if(appearCount > 1){
                  //2+ birthdays matched, we have a positive trial!
                  positiveTrial = true;
                }
              }
             //done with this trial, check if positive and inc if so
             if(positiveTrial){
              positiveTrials++;
             }
            }

            //done with trials, write data to output file
            fprintf(output_file, "%.2f\n", positiveTrials/1000.0);
          }else{
            //people amount was zero, do nothing
          }
        }
        return 0;
    }
}
