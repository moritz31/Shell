/* 
 * File:   ShellInterpreter.cpp
 * Author: debian
 * 
 * Created on 24. April 2017, 10:51
 */

#include "ShellInterpreter.h"

#include <string.h>
#include <iterator>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <signal.h>      // signal
#include <sys/wait.h>    // std::transform

void sig_handler(int signal) {
    std::string sig_str;
    switch(signal) {
        case SIGINT:
            sig_str = "SIGINT";
            break;
        case SIGTSTP:
            sig_str = "SIGTSTP";
            break;
        default:
            sig_str = "UNKOWN";
            break;   
    }
    
    std::cout << "Signal Received: " << sig_str << std::endl;
}

ShellInterpreter::ShellInterpreter() {
    this->isRunning = true;
    this->shell_prompt = "$";
    this->processWait = true;
    signal(SIGINT,sig_handler);
    signal(SIGTSTP,sig_handler);
}

ShellInterpreter::ShellInterpreter(const ShellInterpreter& orig) {
}

ShellInterpreter::~ShellInterpreter() {
}

/**
 * @brief Mainloop of the shell
 */
void ShellInterpreter::update() {
    
    // Loop until we should exit
    while(this->isRunning) {
    
        //Clean all input
        this->cleanUp();
        
        //Retrieve input
        this->getUserInput();

        //Validate input
        if(this->formatInput()) {
            //Execute input
            this->executeProcess();
        }
    }
}

/**
 * @brief Cleans all used var in this class for the next processing
 */
void ShellInterpreter::cleanUp(void) {
    
    // Clear each char* with NULL to be safe
    for(int i = 0; i < this->execBuffer.size();i++) {
        this->execBuffer[i] = NULL; 
    }
    this->execBuffer.clear(); // Clear the vector
    this->buffer.clear(); // Clear the string
    this->processWait = true; // Rest the wait to default  
}

/**
 * @brief Outputs the shell prompt and retrieve the user input
 */
void ShellInterpreter::getUserInput(void) {
    
    std::cout << shell_prompt << " ";
    getline(std::cin,this->buffer);
    
}

/**
 * @brief Convert a string to an char*
 * @param s string which should be converted
 * @return returns an char* which contains the string
 */
static char *convert(const std::string & s)
{
   char *pc = new char[s.size()+1];
   strcpy(pc, s.c_str());
   return pc; 
}

/**
 * @brief Converts the input which is formated as a string into an array of char* which can be passed to execvp
 * @return returns if the input is valid or not
 */
bool ShellInterpreter::formatInput() {
    
    // If the inputBuffer is empty we do not have to do anything
    if(this->buffer.empty()) {
        return false;
    } else if(this->buffer == "logout") {
        this->isRunning = false;
        return false;
    }
        
    // Create a stringstream from a buffer and pass the string to the vector
    std::stringstream ss(this->buffer);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    
    std::vector<std::string> vstrings(begin,end);
    
    // Check for & sign to get process fork info
    if(vstrings.at(vstrings.size()-1) == "&") {
        vstrings.pop_back();
        this->processWait = false;
    }  
    
    // Transform string vector to char* vector
    std::transform(vstrings.begin(),vstrings.end(),std::back_inserter(this->execBuffer),convert);
    
    return true;
}

/**
 * @brief Forks an process and wait for finish depending on wait flag set
 */
void ShellInterpreter::executeProcess(void) {

    int pid;
    
    // Try to fork a child process
    if((pid = fork()) < 0) {
        
        std::cout << "[ERROR] Forking process" << std::endl;
        //Error while forking
        exit(1);
    } else if(pid == 0) { // If we are the child process try to run the command
        
        // Try to execute the buffer
        if(execvp(this->execBuffer[0],this->execBuffer.data()) < 0) {
            
            std::cout << "[ERROR] Executing process" << std::endl;
            // Error while running 
            exit(1);
        }
        
    }
    
    // If we want to wait for an process to finish we need to call waitpid
    if(this->processWait) {
        waitpid(pid,NULL,0);
    } else {
        std::cout << "[PID] " << pid << std::endl;
    }
    
}