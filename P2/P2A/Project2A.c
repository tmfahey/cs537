/************************************************/

/* NAME: Akshay Kumar and Taylor Fahey          */

/* DATE: 27-Sep-2014                            */

/* CS537 - Project 2A - The Unix Shell          */

/************************************************/

#include <stdlib.h>

#include <string.h>

#include <stdio.h>

#include <stdbool.h>

#include <unistd.h>

#include <sys/wait.h>

#include <limits.h>

#include <fcntl.h>



//Defines

#define COMMAND_SIZE 1024         // max command size

#define ARG_MAX 32                // max # args

#define DELIMS " \t\n"            // token sparators

#define PIPE     1                // for special features on cmds

#define REDIRECT 2

#define APPEND   3



// Declarations

void executeCmds(char **, int *, int *);



   

/* Name	       : main()

 * Description : main function of the program

 * Arguments   : number and the parameters passed through command line

 * Return Type : integer

 */

int main (int argc, char ** argv)

{

   char *prompt = "mysh> ";	  //command prompt

   char *arguments[ARG_MAX];      //pointer to arguments

   char **argumentIterator;       //used to iterate through arguments

   char cmdBuffer[COMMAND_SIZE];  //command buffer

   int args;                      //used to count arguments

   int iSplCmd;

   int iSplCmdPos;

   int i;



   char* cwd;                     // pointer to store 'pwd' dir

   char cBuffer[PATH_MAX+1];      // pwd max buffer



   //system("clear");               // Only place where system() is called

   //fprintf(stdout, "*** Welcome to Akshay & Taylor's CS537-2 Shell! ***\n");



   /* read stdin until quit is seen */

   while (!feof(stdin))

   {

      fputs (prompt, stdout);  // prompt user

        

      if (fgets(cmdBuffer, COMMAND_SIZE, stdin))  // get input

      {						

         argumentIterator = arguments;  // parse input into argument array

         *argumentIterator++ = strtok(cmdBuffer,DELIMS);  // splitting up input

         while ((*argumentIterator++ = strtok(NULL,DELIMS)));  // get arguments

            

         //reset counters and bools

         args = iSplCmd = iSplCmdPos = 0;



         if (arguments[0])   // if true, we have argument(s)

         {

            argumentIterator = arguments;  //counting arguments



            while (*argumentIterator)

            {

               args++;

               argumentIterator++;

            }



            //check for special commands

            if(args>1)

            {

               for(i = 0; i < args; i++){

                  if(!strcmp(arguments[i],"|")){

                     iSplCmd = PIPE;

                     iSplCmdPos = i;

                     break;

                  }

                  if(!strcmp(arguments[i],">")){

                     iSplCmd = REDIRECT;

                     iSplCmdPos = i;

                     break;

                  }

                  if(!strcmp(arguments[i],">>")){

                     iSplCmd = APPEND;

                     iSplCmdPos = i;

                     break;

                  }

               }

            }





            /* Parsing through arguments to identify "Build-in Commands" */

            // COMMAND: exit

            if (!strcmp(arguments[0],"exit"))  

            {
               if(arguments[1]){
                  fprintf(stderr, "Error!\n");
                  continue;
               }
               exit(0);

               break;

            }       



            // COMMAND: cd

            else if(!strcmp(arguments[0],"cd"))

            {

               if(NULL == arguments[1]){

                  if(chdir(getenv("HOME")) != 0){

                     fprintf(stderr, "Error!\n");

                  }

               }

					

               else{

                  if(chdir(arguments[1]) != 0){

                     fprintf(stderr, "Error!\n");

                  }

               }

               continue;

            }



            // COMMAND: pwd

            else if(!strcmp(arguments[0],"pwd"))

            {

               cwd = getcwd(cBuffer,PATH_MAX+1);

               if(cwd != NULL){

                  printf("%s\n", cwd);

               }

             	continue;

            }



            else

            {

               executeCmds(arguments, &iSplCmd, &iSplCmdPos);

               continue;

            }



            // Did not recieve a valid command

            fprintf(stderr, "ERROR!\n");

         }

      }

   }

   return 0; 

}







/* Name	      : executeCmd()

 * Description : Splits the commands, forks and executes them

 * Arguments   : Input commands, Cmd type (|,>,>>), Cmd position

 * Return Type : None

 */

void executeCmds(char **arg, int *iCmdType, int *iCmdPos)

{

   pid_t  pid, pid1, pid2;        // PID for child processes

   int    status;



   int i;

   char *newarg1[ARG_MAX];        // New Args array for Cmd1 after splitting

   char *newarg2[ARG_MAX];        // New Args array for Cmd2 after splitting

   char **ArrayPtr;               // Pointer to access the split arrays



   int fd_out;                    // Output File Descriptors

   mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;



   int pipefds[2];                // Pipe File Descriptors





   ArrayPtr = newarg1;            // Assign pointer to first array



  

   /* Split the commands into two based on the special char (|,>,>>) */

   if((*iCmdType == PIPE) || (*iCmdType == REDIRECT) || (*iCmdType == APPEND))

   {

      for(i = 0; i<(*iCmdPos); i++){ // Copy Cmd1 into array1

         *ArrayPtr++ = *arg++;

      }



      *ArrayPtr = NULL;

      *arg++;                        // Increment once to skip the special char

      ArrayPtr = newarg2;            // Now assign pointer to second array



      while(*arg != NULL){           // Copy Cmd2 into array2

         *ArrayPtr++ = *arg++;

      }



      *ArrayPtr = NULL;





      /* To do when redirect */

      if (*iCmdType == REDIRECT)

      {

         fd_out = open(newarg2[0],(O_WRONLY | O_CREAT | O_TRUNC), mode);

         if(fd_out < 0){

            perror("Error opening file");

         }

         

         pid = fork();  /* fork child process */



         if (pid < 0){

            fprintf(stderr, "ERROR while forking child process\n");

            exit(1);

         }



         else if (pid == 0){  /* fork succeeded thus creating a child */

            dup2(fd_out,STDOUT_FILENO);

            close(fd_out);

            if (execvp(*newarg1, newarg1) < 0){  /* execute the command */

               fprintf(stderr, "Error!\n");

               exit(1);

            }

         }



         else{  /* fork() assigns and returns child pid to the parent process */

            while (wait(&status) != pid);  /* wait for completion  */

         }

      }





      /* To do when append */

      if (*iCmdType == APPEND)

      {

         fd_out = open(newarg2[0],(O_WRONLY | O_CREAT | O_APPEND), mode);

         if(fd_out < 0){

            perror("Error opening file\n");

         }



         pid = fork();  /* fork child process */



         if (pid < 0){

            fprintf(stderr, "ERROR while forking child process\n");

            exit(1);

         }



         else if (pid == 0){  /* fork succeeded thus creating a child */ 

            dup2(fd_out,STDOUT_FILENO);

            close(fd_out);      

            if (execvp(*newarg1, newarg1) < 0){  /* execute the command */

               fprintf(stderr, "Error!\n");

               exit(1);

            }            

         }



         else{  /* fork() assigns and returns child pid to the parent process */

            while (wait(&status) != pid);  /* wait for completion  */

         }

      }





      /* To do when pipe */

      if (*iCmdType == PIPE)

      {

         if (pipe(pipefds)){  // Creating the Child-to-Child Pipe

            fprintf (stderr, "ERROR while creating pipe\n");

         }



         pid1 = fork();  /* fork child process 1 */



         if (pid1 < 0){

            fprintf(stderr, "ERROR while forking child process\n");

            exit(1);

         }



         else if (pid1 == 0){  /* fork succeeded thus creating a child */

            dup2(pipefds[1],STDOUT_FILENO);

            close(pipefds[0]);

            close(pipefds[1]);



            if (execvp(*newarg1, newarg1) < 0){  /* execute the command */

               fprintf(stderr, "Error!\n");

               exit(1);

            }

         }





         pid2 = fork();  /* fork child process 2 */



         if (pid2 < 0){

            fprintf(stderr, "ERROR while forking child process\n");

            exit(1);

         }



         else if (pid2 == 0){  /* fork succeeded thus creating a child */         

            dup2(pipefds[0],STDIN_FILENO);

            close(pipefds[1]);         

            close(pipefds[0]);



            if (execvp(*newarg2, newarg2) < 0){  /* execute the command */

               fprintf(stderr, "Error!\n");

               exit(1);

            }

         }



         else{  /* Parent Process waiting till both Child terminates */

            close(pipefds[0]);

            close(pipefds[1]);                        

            waitpid(pid1, &status, WUNTRACED);  /* wait for completion  */

            waitpid(pid2, &status, WUNTRACED);  /* wait for completion  */

         }

      }

   }









   /* Normal case: when there is NO special characters */

   else

   {

      pid = fork();  /* fork child process */



      if (pid < 0){

         fprintf(stderr, "ERROR while forking child process\n");

         exit(1);

      }



      else if (pid == 0){  /* fork succeeded thus creating a child */       

         if (execvp(*arg, arg) < 0){  /* execute the command */

            fprintf(stderr, "Error!\n");

            exit(1);

         }

      }



      else{  /* fork() assigns and returns child pid to the parent process */

         while (wait(&status) != pid);  /* wait for completion  */

      }

   

   }  

}
