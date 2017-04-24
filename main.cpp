#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "ShellInterpreter.h"

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

std::string bash_start = "$ ";

std::string getInput(void);

void splitInput(std::string buffer, char** out);
void execute(char** args);

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}


char* test[64];

int main() 
{
        
        ShellInterpreter *shell = new ShellInterpreter();
        
        shell->update();

	return (0);
}

/**
 *	Waits for user input and then returns back the string
 */
std::string getInput(void) {

	std::string buffer;

	std::cout << bash_start;
	getline(std::cin,buffer);

	return buffer;
} 

void splitInput(std::string buffer, char** out) {
	std::string delimiter = " ";
	size_t pos = 0;
	std::string token;

	size_t counter = 0;

	rtrim(buffer);

	while(( pos = buffer.find(delimiter)) != std::string::npos) {
		token = buffer.substr(0,pos);
		char* cstr = new char[token.length() + 1];
		strcpy(cstr,token.c_str());
		out[counter] = cstr;
		buffer.erase(0,pos + delimiter.length());
		counter++;
	}
	char* cstr = new char[buffer.length() + 1];
	strcpy(cstr,buffer.c_str());
	out[counter] = cstr;
	out[counter+1] = NULL;
}

void execute(char** args) {
	pid_t pid;
	int status;

	bool wait = true;
	for(int i = 63;i > 1; i--) {
		if(args[i] == NULL) {
			if(args[i-1] != NULL && strcmp(args[i-1],"&") == 0) {
				args[i-1] = NULL;
				wait = false;
			}
		}
	}

	if((pid = fork()) < 0) {
		// Error while forking
		std::cout <<"ERROR while forking" << std::endl;
		exit(1);
	} else if (pid == 0) {
		// Child process
		if(execvp(args[0],args) < 0) {
			// Error
			std::cout << "ERROR while executing" << std::endl;
			exit(1);
		}
	}
		
	if(wait) {
		waitpid(pid,NULL,0);
	} else {
		std::cout << "[PID] " << pid << std::endl;
	}

	return;
}
