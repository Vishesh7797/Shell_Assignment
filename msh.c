/*
Vishesh Gupta
UTA ID: 1001455100
*/

// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                               // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 10   // Mav shell only supports ten arguments
#define MAX_HIST 50 
#define MAX_PIDS 15 

int org_Child = -500;    
int non_sus = -500;        
int pids[15];       

//Process & History count vars       
int pCount = 0;
int histCount = 0;  
          
static void signal_handling (int sig) // Ctrl-C and Ctrl-Z
{  
   if(sig == 20)
       non_sus = org_Child;   
   if( sig !=2 && sig !=20)
      printf("Unable to determine the signal\n");
   printf("\nmsh> ");
}

// History store + output
static void io_history(char command[], char ***history)
{
     int loop_var;
     if(histCount<MAX_HIST)
     {
         strcpy((*history)[histCount], command);       
         histCount+=1;
     }
     else
     {
          for(loop_var = 0; loop_var < (MAX_HIST-1); loop_var++)
          {
              strcpy((*history)[loop_var],(*history)[loop_var+1]);
          }
          strcpy((*history)[49],command);
          histCount = 50; //History 50 commands
     }
}
//Pid output
static void io_pid(int pid)
{
  
  int loop_var;
  if(pCount < MAX_PIDS)           
  {
     pids[pCount] = pid;
     pCount+=1;
  }
  else
  {
      for(loop_var = 0; loop_var < (MAX_PIDS-1); loop_var++)
      {
          pids[loop_var] = pids[loop_var+1];
      }
      pids[14] = pid;
      pCount = 15;
  }
}

static void out_pids()
{
  int i;
  for( i = 0; i < pCount; i++)
  {
     printf("%d: %d\n",i, pids[i]);
  }
}

static void out_history(char ***history)
{
  int i;
  for( i = 0; i < histCount; i++)
  {
     printf("%d: %s\n",i, (*history)[i]);
  }
}

     
int main()
{            
     char** history = (char**) malloc(MAX_HIST*sizeof(char*)); 
     int loop_var = 0;
     struct sigaction sigact;
     memset (&sigact, '\0', sizeof(sigact));    
     sigact.sa_handler = &signal_handling;
      
     if (sigaction(SIGINT , &sigact, NULL) < 0) 
     {
        perror ("sigaction: ");
        return 1;
     }
     if (sigaction(SIGTSTP , &sigact, NULL) < 0) //Install SIGNT and SIGTSTP
     {
        perror ("sigaction: ");
        return 1;
     }
     
     for( loop_var = 0; loop_var<MAX_HIST;loop_var++)
     {
         history[loop_var] = (char*) malloc(MAX_COMMAND_SIZE * sizeof(char) );
     } 
     char * commandString = (char*) malloc(MAX_COMMAND_SIZE);
     char * command = (char*) malloc(MAX_COMMAND_SIZE);

     pids[pCount] = getpid();
     pCount++;
     
     while(1)
     {
        printf ("msh> ");
             
        while(!fgets (command, MAX_COMMAND_SIZE, stdin));
    
        //Making sure spaces don't mess up the output
        int spaces = 0, space_var, num_spaces = 0;
        int cmd_len = (int) strlen(command);
        for( space_var = 0; space_var < cmd_len; space_var++)
        {
          int k = (int)command[space_var];
          if( k == 32) 
            spaces++;
          if( k != 32 && k != 10)
            {
              num_spaces++; 
              break;
            }
        }
        if(num_spaces)   
        
        {     
          strncpy(commandString, command + spaces, cmd_len);
          int repeat_command = 143;
          if(commandString[0] == '!')
            {
              for(loop_var =1; loop_var < strlen(commandString); loop_var++)
                {
                  if(commandString[loop_var] == ' ' || commandString[loop_var]== '\0')
                     break;
                  else 
                    { 
                      if((int)commandString[loop_var] > 47 && (int)commandString[loop_var] < 58)
                        {
                          if(loop_var == 1)
                            { 
                              repeat_command = 0;repeat_command += (int) commandString[loop_var] - 48;
                            }
                          else
                            {
                              repeat_command = repeat_command*10;
                              repeat_command += (int) commandString[loop_var] - 48;
                            }
                        }
                      else 
                        break;
                    }
                }
                 
                  if(repeat_command < histCount)
                  {     
                       strcpy(commandString, history[repeat_command]);
                  }
            } 
            
            char *token[MAX_NUM_ARGUMENTS];
            int tk_count = 0;                                                                                                        
            char *argument_pointer;                                                                                  
            char *working_string  = strdup(commandString);                      
            char *working_root = working_string;

            
            while(((argument_pointer = strsep(&working_string, WHITESPACE)) != NULL) && 
               (tk_count<MAX_NUM_ARGUMENTS))
             {
                token[tk_count] = strndup(argument_pointer, MAX_COMMAND_SIZE);
                if(strlen( token[tk_count] ) == 0)
                    token[tk_count] = NULL;
                tk_count++;
             }

              
              if(!strcmp(token[0], "quit") || !strcmp(token[0], "exit"))
              return 0;
              
              if( !strcmp(token[0], "cd"))
              {
                  if(chdir(token[1]) == -1)
                    printf("Unable to access directory: %s.\n", token[1]);
                  else
                    io_history(commandString, &history);
              }
              else if( !strcmp(token[0], "bg")) 
              {
                  if(non_sus != -500)
                     {
                        printf("BG %d\n", non_sus );
                        kill(non_sus, SIGCONT);
                        non_sus = -500;
                     }
                  else 
                    {printf("No process running in background\n");}
                io_history("bg",&history);
              }
               else if(!strcmp(token[0], "listpids"))
               {
                  out_pids(); 
                  io_history("listpids",&history); 
               }
               else if(!strcmp(token[0], "history"))
               {    
                  io_history("history",&history);
                  out_history(&history); 
               }
               else
               {  
                    io_history(commandString, &history);
                    pid_t child_pid = fork();
                    if(child_pid !=0)
		                  org_Child = child_pid;
		                else
                      org_Child = getpid();
                    int status;

                  if(child_pid == 0)
                  {
                    if(execv(token[0], token ) == -1)
                        { 
                          char outofnames[256] = "/usr/local/bin/";
                          strcat(outofnames,token[0]);
                             if( execv(outofnames, token ) == -1)
                             {
                              char loop2[256] = "/usr/bin/";
                              strcat(loop2,token[0]);
                              if( execv(loop2, token ) == -1)
                                 {
                                  char loop3[256] = "/bin/";
                                  strcat(loop3,token[0]);
                                  if(execv(loop3, token ) == -1)
                                     {
                                      printf("%s : Command not found\n", token[0]);
                                     }
                                  }
                              }
                          } 
                      exit(EXIT_SUCCESS);
                    }
                  free(working_root);                   
                  waitpid(child_pid, &status,0);     
                  io_pid(child_pid); 
               }    
                   
             } 
             for(loop_var = 0; loop_var <256; loop_var++)
                 commandString[loop_var] = '\0';command[loop_var] = '\0';
        }
  free(commandString);
  free(command);
  return 0;
}

//finally
