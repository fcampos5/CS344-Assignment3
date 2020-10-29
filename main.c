#include <stdio.h>  // for perror
#include <stdlib.h>   // for exit
#include <stdbool.h>  // for true/false
#include <unistd.h>   // for execv and fork
#include <string.h>	  // for token
#include <sys/types.h> // pid_t
#include <ctype.h>

char buffer[2048];

// Creates child process for new command
void createChildProcess() 
{
	// newargv[] = array of commands, token for buff
	char* newargv[512];
	char* token;
	char temp[100];
	int childStatus;
	int i = 0;
	
	// Fork Child
	pid_t spawnpid = -5;
	spawnpid = fork();

	switch (spawnpid)
	{
	// Error case of bad fork :(
	case -1:
		perror("fork() failed!");
		exit(1);
		break;
	
	// Child case
	case 0:
		// Break down buffer into token and add first command to newargv[]
		token = strtok(buffer, " ");
		newargv[i] = token;
		i++;

		// Go through rest of token and add each command to newargv[]
		while (token != NULL) 
		{	
		// Checks token to see if it has "$$"
			//if (strstr(token,"$$") != NULL) 
			//{
			//	//Create new char[] where $$ in token is replaced with getpid() and add to newargv[]
			//}
			token = strtok(NULL, " ");
			newargv[i] = token;
			i++;			
		}
		newargv[i] = NULL;

		// Exec newargv[]
		execvp(newargv[0], newargv);
		perror("execvp");
		exit(1);

	// Parent case
	default:
		// Wait for child process to terminate before continuing
		spawnpid = waitpid(spawnpid, &childStatus, 0);
		break;
	}

}

// Returns false/0 if buffer is a comment or blank line
int isValid() 
{
	// Create token variable to see if it can find any valid input from buffer
	char checkbuffer[2048];
	char* token;
	strcpy(checkbuffer, buffer);
	token = strtok(checkbuffer, " ");

	// Checks if buffer has # in the begining of the string
	if (token[0] == '#') 
	{
		return 0;
	}
	// Checks if buffer is empty with blank spaces
	if (token == NULL) 
	{
		return 0;
	}
	// Buffer has valid input, return true/1
	else 
	{
		return 1;
	}
}


//void promptCheck() 
//{
//	// Create token variable to see if it can find any valid input from buffer
//	char checkbuffer[2048];
//	char* token;
//	strcpy(checkbuffer, buffer);
//	token = strtok(checkbuffer, " ");
//
//	// If input from buffer was exit, run exit function
//	if (token == "exit") 
//	{
//		printf("goodbye\n");
//		return 0;
//	}
//}

// Prompt where commands will be entered and sent to other functions
void commandPrompt() 
{
	// Display prompt for user and enter for buff
	memset(buffer, 0, sizeof(buffer));
	printf(": ");
	scanf("%[^\n]%*c", buffer);

	// Check if buffer is valid (blank lines or #), if not (returns 0) then run commandPrompt
	if (isValid() == 0)
	{
		printf("Not valid input\n");
		commandPrompt();
	}
	// Go create a child
	else
	{
		createChildProcess();
		commandPrompt();
	}
}

int main(int argc, char* argv[])
{
	// Start
	printf("$ smallsh\n");

	// Open command prompt for user
	commandPrompt();

}