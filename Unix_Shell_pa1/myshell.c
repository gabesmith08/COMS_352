// Author -- Richard Smith

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define MAX_BUFFER 1024 // maximum line buffer
#define MAX_ARGS 120  // maximum amount of arguments
#define SEPARATORS " \t\n"  // token sparators

void cd(char *d);
void env(char **e);
void syserr(char *msg);
void checkIO(char **args);
int checkBackground(char **args);
void setMyShellEnv();

extern char **environ;  // environment variables
extern int errno; // system error number
pid_t pid;  // process ID
int status; // status for fork/exec process
int in, out, input, output, append; // I/O redirection parameters
char *inputFile, *outputFile; // I/O input and output files
FILE *fp; // pointer to file for ouput file

// function that implements "cd" command
void cd(char *d)
{
  char *name = "PWD"; // string PWD
  char cwd[256]; // holder for current directory
  char *newCurrent = getcwd(cwd, sizeof(cwd)); // get the current dir and put it in cwd (current working directory)
  chdir(d); // change the directory
  setenv(name, newCurrent, 1); // set the new pwd (print working directory)
}

// function that implements "environ" command (environment variables)
void env(char **e)
{
  char **env = e;

  if (output == 1)  // IO redirection
  {
    fp = fopen(outputFile, "w");
  }
  else if (append == 1)
  {
    fp = fopen(outputFile, "a");
  }
  if (output == 1 || append == 1) // if user chooses to append or use output, fprintf
  {
    while(*env)
    {
      fprintf(fp,"%s\n", *env++);
    }
    fclose(fp);
  }
  else // else, print info to the screen
  {
    while(*env)
    {
      printf("%s\n", *env++);
    }
  }
}

// function to handle errors
void syserr(char *msg)
{
  fprintf(stderr, "%s: %s\n", strerror(errno), msg);
  abort();
}

// checks user input command for I/O redirection
void checkIO(char **args)
{
  // reset input and output and append
  input = 0;
  output = 0;
  append = 0;
  int i = 0;

  while(args[i] != NULL) // loop through the command line input
  {
    if (!strcmp(args[i], "<")) // check for input "<"
    {           
      strcpy(args[i], "\0");
      inputFile = args[i + 1];
      input = 1;
    }
    else if (!strcmp(args[i], ">")) // outputfile is created if it doesn't exist and TRUNCATED if it does ">"
    {      
      outputFile = args[i + 1];
      args[i] = NULL;
      output = 1;
      break;
    }
    else if (!strcmp(args[i], ">>")) // outputfile is created if it doesn't exist and APPENDED to if it does ">>"
    {     
      outputFile = args[i + 1];
      args[i] = NULL;
      append = 1;
      break;
    }
    i++;
  }
}

// dont_wait = 1 id last non whitespace character is an ampersand
int checkBackground(char **args)
{
  int i = 0;
  int dont_wait = 0;
  while(args[i] != NULL){
    if (!strcmp(args[i], "&"))
    {
      dont_wait = 1;
      args[i] = NULL; // remove the & and set to NULL so that the commmand will work
    }
    i++;
  }
  return dont_wait;
}

// set the shell environment variable to the <homepath>/myshell
void setMyShellEnv()
{
  char home_path[1024];
  getcwd(home_path, 1024);
  strcat(home_path, "/myshell");
  setenv("shell", home_path, 1);
}

// the main function - includes 
int main(int argc, char **argv)
{
  char cwd[256]; 
  char *prompt;
  char *promptEnding = "--> ";
  char buf[MAX_BUFFER];
  char *args[MAX_ARGS];
  char **arg;
  int dont_wait = 0;
  int status;


  setMyShellEnv(); // get the shells environment

  if(argc > 1) // check access
  {
    freopen(argv[1], "r", stdin);
  }

  while(!feof(stdin))
  {
    prompt = getcwd(cwd, sizeof(cwd)); 
    strcat(prompt, promptEnding);
    fputs(prompt, stdout);

    if(fgets(buf, MAX_BUFFER, stdin))
    {
      arg = args;
      *arg++ = strtok(buf,SEPARATORS);

      while ((*arg++ = strtok(NULL,SEPARATORS)));

      checkIO(args); // check i/o redirections
      dont_wait = checkBackground(args); // check for background execution

      if (args[0]) 
      {
        if (input == 1) // if there was an input redirection (<)
        {
          if(!access(inputFile, R_OK))  // check access
          { 
            freopen(inputFile, "r", stdin); // replace the stdin with the file
          }
        }

      // int i;
      // for( i=1; i<argc-1; i++) // search for pipe
      // {
      //     int pd[2];
      //     pipe(pd);
      //     if (!fork()) {
      //         dup2(pd[1], 1); // remap output back to parent
      //         execlp(argv[i], argv[i], NULL);
      //         perror("exec");
      //         abort();
      //     }
      //     dup2(pd[0], 0); // remap output from previous child to input
      //     close(pd[1]);
      // }

      // execlp(argv[i], argv[i], NULL);
      // perror("exec");
      // abort();

        if (!strcmp(args[0], "cd")) // if statement that calls cd helper function
        { 
          cd(args[1]); // calls custom cd function
          continue;
        }

        if (!strcmp(args[0], "clr")) // if statement that implements "clr" command
        {
          pid = getpid(); // get process id
          switch(pid = fork()){
            case -1:
              syserr("fork"); // error
            case 0:
              setenv("parent", getenv("shell"), 1);  // set parent
              execvp("clear", args); // execute in the child thread
              syserr("execvp"); // error
            default:
              if(!dont_wait)  // determine if wait is necessary for the background execution
                waitpid(pid, &status, WUNTRACED);
          }
          continue;
        }

        if (!strcmp(args[0],"dir")) // if statement that implements the "dir" (directory) command 
        {
          pid = getpid();
          switch(pid = fork())
          {
            case -1:
              syserr("fork");
            case 0: 
              setenv("parent", getenv("shell"), 1); // set parent
              
              if(output == 1) // set the i/o redirection
                freopen(outputFile, "w", stdout); // replace the stdout with the outputfile
              else if(append == 1)
                freopen(outputFile, "a+", stdout); // replace the stdout with the outputfile (append)

              if (args[1])  // if there is a argument after the "dir" command ls for that directory
              {
                if(execl("/bin/ls", "ls", "-al", args[1], NULL))
                {
                syserr("execl"); // execute in the child thread
                exit(1); // exit so next part won't run
                }
              }
              
              if (execl("/bin/ls", "ls", "-al", NULL) < 0) // otherwise, ls for current directory
              {
                syserr("execl");
                exit(1);
              }
              syserr("execl");
            default:
              if(!dont_wait) //determine wait for background execution
                waitpid(pid, &status, WUNTRACED);
            }
          continue;
        }

        if (!strcmp(args[0],"environ")) // if statement that calls env helper function
        {
          env(environ); // calls custom env function to get environment variables
          continue;
        }

        if (!strcmp(args[0],"echo"))  // if statement that implements the "echo" command, but ignores all whitespace, including tabs
        {
          pid = getpid(); // get process id
          switch(pid = fork())
          {
            case -1:
              syserr("fork");
            case 0:
              setenv("parent", getenv("shell"), 1); // set parent

              // I/O redirection for 'output' files
              if (output == 1)
                freopen(outputFile, "w", stdout);
              else if (append == 1)
                freopen(outputFile, "a+", stdout);

              execvp (args[0], args);  // execute in the child thread
              syserr("execvp"); // error

            default:
              if (!dont_wait) // determine background execution wait (&)
                waitpid(pid, &status, WUNTRACED);
          }
          continue;
        }

        if (!strcmp(args[0],"help"))  // if statement that implements the README pop up in terminal
        {
          char readme_path[1024]; // set the home path and find the readme
          getcwd(readme_path, 1024);
          strcat(readme_path, "/README.txt");

          pid = getpid();
          switch (pid = fork()) 
          {
            case -1:
              syserr("fork");
            case 0:
              setenv("parent", getenv("shell"), 1);  // set parent

              if(output == 1) // I/O redireciton for the output files
                freopen(outputFile, "w", stdout);
              else if(append == 1)
                freopen(outputFile, "a+", stdout);

              if (execlp("more", "more", readme_path, NULL) < 0) // more prints out the file
              { 
                syserr("execlp"); // execute in child thread
              }
            default:
              if (!dont_wait) // determine background execution wait (&)
                waitpid(pid, &status, WUNTRACED);
            }
          continue;
        }

        if (!strcmp(args[0],"pause")) // pause the shell until ENTER is pressed
        {
          getpass("Press the ENTER key to continue "); // getpass from the GNU C library is perfect for this task, so I used it 
          continue;
        }

        if (!strcmp(args[0],"quit")) 
        {
          break; // break statement terminates program
        }

        // else statement takes all non internal commands
        else
        {
          pid = getpid();
          switch (pid = fork ()) 
          {
            case -1:
              syserr("fork");
            case 0:
              setenv("parent", getenv("shell"), 1); // set parent

              if(output == 1) // I/O redirection
                freopen(outputFile, "w", stdout);
              else if(append == 1)
                freopen(outputFile, "a+", stdout);

              execvp (args[0], args); // executes in child thread
              syserr("exec");
            default:
              if (!dont_wait) // determines background execution for waiting
                waitpid(pid, &status, WUNTRACED);
          }
          continue;
        }
      }

    }

  }
  return 0;
}
