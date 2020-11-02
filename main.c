#include <stdio.h>  // for perror
#include <stdlib.h>   // for exit
#include <stdbool.h>  // for true/false
#include <unistd.h>   // for execv and fork
#include <string.h>	  // for token
#include <sys/types.h> // pid_t
#include <ctype.h>
#include <fcntl.h>

char buffer[2048];
int lastExitStatus = 0;


// Creates child process for new command
void createChildProcess(char* command, int* input, int* output1, int* output2) 
{
	// newargv[] = array of commands, token for buff
	char* newargv[512];
	char* token;
	char temp[100];
	int childStatus;
	int i = 0;
	int result;
	
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
		token = strtok(command, " ");
		newargv[i] = token;
		i++;

		// Go through rest of token and add each argument to newargv[]
		while (token != NULL) 
		{	
			token = strtok(NULL, " ");
			newargv[i] = token;
			i++;	
		}

		// Add NULL to end of newargv
		newargv[i] = NULL;

		// Consider file input/output directions if needed
		if (output2 != NULL) 
		{
			result = dup2(input, 0);
			result = dup2(output2, 1);
			close(input);
			close(output2);
		}
		else if (output1 != NULL) 
		{
			result = dup2(output1, 1);
			close(output1);
		}
		else if (input != NULL)
		{
			result = dup2(input, 0);
			close(input);
		}

		// Execvp newargv[](Command with arguments) 
		execvp(newargv[0], newargv);
		perror("execvp");
		exit(1);

		break;

	// Parent case
	default:
		// Wait for child process to terminate before continuing
		spawnpid = waitpid(spawnpid, &childStatus, 0);
		lastExitStatus = childStatus;
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


void builtInCommands() 
{
	// Create token variable to see if it can find any valid input from buffer
	char checkbuffer[2048];
	char* token;
	char dir[200];

	// Copy buffer into checkbuffer then break checkbuffer into token
	strcpy(checkbuffer, buffer);
	token = strtok(checkbuffer, " ");

	// If token was exit, run exit function
	if (strcmp(token,"exit") == 0) 
	{
		printf("$\n");
		exit(0);
	}
	// If token was cd, run cd function
	if (strcmp(token, "cd") == 0)
	{
		token = strtok(NULL," ");
		// Checks if cd has a follow up destination, if it does than change directory to that destination
		if (token != NULL) 
		{
			chdir(token);
		}
		// If theres no follow destination, then chang directory to HOME enviroment variable
		else 
		{
			chdir( getenv("HOME") );
		}

		commandPrompt();
	}
	if (strcmp(token, "status") == 0)
	{
		printf("exit value %d\n", lastExitStatus);
		commandPrompt();
	}
}

void redirectIO() 
{
	char checkbuffer[2048];
	char* command;
	char* inputFile = NULL;
	char* outputFile1 = NULL;
	char* outputFile2 = NULL;
	char* token;
	int result;

	// Copy buffer into checkbuffer then break checkbuffer into token
	strcpy(checkbuffer, buffer);
	token = strtok(checkbuffer, " ");

	// First token will be the command
	command = token;
	token = strtok(NULL, " ");

	// Next token will be either "<" or ">" which will decide input/output files
	if (strcmp(token, "<") == 0) 
	{
		token = strtok(NULL, " ");
		inputFile = token;
	}
	else if (strcmp(token, ">") == 0)
	{
		token = strtok(NULL, " ");
		outputFile1 = token;
	}

	token = strtok(NULL," ");

	// Check if theres another file to redirect
	if (token != NULL) 
	{
		token = strtok(NULL, " ");
		outputFile2 = token;
	}

	// Time to redirect files
	if (outputFile2 != NULL) 
	{
		// Open/create file for writing
		int fileOut = open(outputFile2, O_WRONLY | O_CREAT | O_TRUNC, 0644);

		// Open/create file for reading
		int fileIn = open(inputFile, O_RDONLY, 0644);

		// Error if could not open file
		if (fileOut == -1)
		{
			perror("open()");
			exit(1);
		}
		else
		{
			createChildProcess(command, fileIn, NULL, fileOut);
		}
		close(fileOut);
		close(fileIn);
	}
	else if (outputFile1 != NULL) 
	{
		// Open/create file for writing
		int fileOut = open(outputFile1, O_WRONLY | O_CREAT | O_TRUNC, 0644);

		// Error if could not open file
		if (fileOut == -1) 
		{
			perror("open()");
			exit(1);
		}
		else 
		{
			createChildProcess(command, NULL, fileOut, NULL);
		}
		close(fileOut);
	}
	else if (inputFile != NULL) 
	{
		
		// Open/create file for reading
		int fileIn = open(inputFile, O_RDONLY, 0644);

		// Error if could not open file
		if (fileIn == -1) 
		{
			perror("open()");
			exit(1);
		}
		else 
		{
			createChildProcess(command, fileIn, NULL, NULL);
		}
		close(fileIn);
	}
}



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

	// Check if buffer is calling any built-in commands
	builtInCommands();

	// Check if buffer has any commands for input/output files, if it does then run redirectIO function
	if (strstr(buffer, " <") != NULL) 
	{
		redirectIO();
		commandPrompt();
	}
	else if (strstr(buffer, " >") != NULL) 
	{
		redirectIO();
		commandPrompt();
	}
	else 
	{
		// Go create a child
		createChildProcess(buffer, NULL, NULL, NULL);
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

// Return true / 1 if there's an instance of $$
//void edit$$() 
//{
//	char checkbuffer[2048];
//	char newbuffer[2048];
//	char pid[20];
//	char* editedString;
//	char temp[100];
//	char* token;
//
//	// Get parent id to pid
//	sprintf(pid, "%d", getpid());
//
//	// copy buffer into checkbuffer and grab token to checkbuffer
//	strcpy(checkbuffer, buffer);
//	token = strtok(checkbuffer, " ");
//
//	while (token != NULL)
//	{
//		// We found $$ in token
//		if (strstr(token, "$$") != NULL)
//		{
//			// Token is "$$"
//			if (strcmp(token, "$$") == 0)
//			{
//				strcat(newbuffer, pid);
//				strcat(newbuffer, " ");
				//token = strtok(NULL, " ");
			//}
			// Token is "{text}$$"
			//else 
			//{
			//	// Copy text into edited String and add pid to it. Doesn't include $$
			//	editedString = strtok(token, "$");
			//	strcat(editedString, pid);

			//	// Copy editied string into new buffer
			//	strcat(newbuffer, editedString);
			//	strcat(newbuffer, " ");
			//	token = strtok(NULL, " ");
			//}
//		}
//		else 
//		{
//			strcat(newbuffer, token);
//			strcat(newbuffer, " ");
//			token = strtok(NULL, " ");
//		}
//	}
//
//	strcpy(buffer, newbuffer);
//
//}