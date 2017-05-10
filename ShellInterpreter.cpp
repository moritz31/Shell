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

int foreGroundProcessPID = -1;
int exitProcess = 0;

// Define shell cmds
const std::string ShellInterpreter::cmds[3]= { "logout", "fg", "bg"};

std::vector<process_t> ShellInterpreter::processList;

/**
 * 
 * @param sig
 */
void ShellInterpreter::sigint_handler(int sig) {
    if(foreGroundProcessPID != -1) {
        kill(foreGroundProcessPID,SIGINT);
        std::cout << "[SIGINT] received" << std::endl;
    }
}

/**
 * 
 * @param sig
 */
void ShellInterpreter::sigtstp_handler(int sig) {
    if(foreGroundProcessPID != -1) {
        kill(foreGroundProcessPID,SIGTSTP);
        std::cout << "[SIGTSTP] received" << std::endl;
    }
}

/**
 * 
 * @param sig
 */
void ShellInterpreter::sigchld_handler(int sig) {
    int status;  
    pid_t pid;  
    
    // Waiting for/ handling all of the child processes according to their status
    while ((pid = waitpid(-1, &status, WNOHANG|WCONTINUED)) > 0) {  
          
        // If child is continued set the job to foreground
        if (WIFCONTINUED(status)) {
            std::cout << "Job continued" << std::endl;
            ShellInterpreter::setForeGroundJob(pid);
        }
        
        // If child exited dispatch the job from the list
        if (WIFEXITED(status)) {
            std::cout << "Job dispatched" << std::endl;
            ShellInterpreter::dispatchJob(pid);
        }
    }  
}

ShellInterpreter::ShellInterpreter() {
    this->isRunning = true;
    this->shell_prompt = "$";
    this->processWait = true;
    
    signal(SIGINT,ShellInterpreter::sigint_handler);
    signal(SIGTSTP,ShellInterpreter::sigtstp_handler);
    signal(SIGCHLD,ShellInterpreter::sigchld_handler);
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
    } else if(this->buffer == cmds[0]) {
        return this->shell_logout();
    } else if(this->buffer == cmds[1]) {
        return this->shell_fg();
    } else if(this->buffer == cmds[2]) {
        return this->shell_bg();
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
    int status;
    // Try to fork a child process
    if((pid = fork()) < 0) {
        
        std::cout << "[ERROR] Forking process" << std::endl;
        //Error while forking
        exit(1);
    } else if(pid == 0) { // If we are the child process try to run the command
        // Try to execute the buffer
        setpgid(0,getpid());
        if(execvp(this->execBuffer[0],this->execBuffer.data()) < 0) {
            
            std::cout << "[ERROR] Executing process" << std::endl;
            // Error while running 
            exit(1);
        }
        
    } else {
        // If we want to wait for an process to finish we need to call waitpid
        if(this->processWait) {
            this->addJob(pid);
            this->setForeGroundJob(pid);
            foreGroundProcessPID = pid;
            waitpid(pid,&status, WUNTRACED);
            // If child is stopped set the job to stopped
            if (WIFSTOPPED(status)) {
                std::cout << "Job stopped" << std::endl;
                ShellInterpreter::setStoppedJob(pid);
            }
        
            // If child exited dispatch the job from the list
            if (WIFEXITED(status)) {
                std::cout << "Job dispatched" << std::endl;
                ShellInterpreter::dispatchJob(pid);
            }
        } else {
            this->addJob(pid);
            this->setBackgroundJob(pid);
            std::cout << "[PID] " << pid << std::endl;
        }
    }
 }
 
/**
 * 
 * @param pid
 */
void ShellInterpreter::addJob(pid_t pid) {
    process_t new_proc = {.pid = pid, .state = UNKOWN};
    ShellInterpreter::processList.push_back(new_proc);
}

/**
 * 
 * @param pid
 */
void ShellInterpreter::dispatchJob(pid_t pid) {
    std::vector<process_t>::iterator it = std::find_if(ShellInterpreter::processList.begin(), ShellInterpreter::processList.end(), [&pid](process_t& f) -> bool { return f.pid == pid; } );
    if(it != ShellInterpreter::processList.end()) {
        ShellInterpreter::processList.erase(it);
        std::cout << "Erased " << pid << std::endl;
    }

}

void ShellInterpreter::setForeGroundJob(pid_t pid) {
    std::vector<process_t>::iterator it = std::find_if(ShellInterpreter::processList.begin(), ShellInterpreter::processList.end(), [&pid](process_t& f) 
            -> bool { return f.pid == pid; } );
        it->state = FOREGROUND;
}

void ShellInterpreter::setBackgroundJob(pid_t pid) {
    std::vector<process_t>::iterator it = std::find_if(ShellInterpreter::processList.begin(), ShellInterpreter::processList.end(), [&pid](process_t& f) 
            -> bool { return f.pid == pid; } );
        it->state = BACKGROUND;
}

void ShellInterpreter::setStoppedJob(pid_t pid) {
    std::vector<process_t>::iterator it = std::find_if(ShellInterpreter::processList.begin(), ShellInterpreter::processList.end(), [&pid](process_t& f) 
            -> bool { return f.pid == pid; } );
        it->state = STOPPED;
}

/**
 * 
 * @return 
 */
bool ShellInterpreter::shell_logout(void) {
    if(this->processList.size() != 0) {
        std::cout << "[ERROR] not all backgroundjobs finished!" << std::endl;
        return false;
    }
    this->isRunning = false;
    return false;
}

/**
 * 
 * @return 
 */
bool ShellInterpreter::shell_fg(void) {
    if(foreGroundProcessPID != -1) {
        int status;
        ShellInterpreter::setForeGroundJob(foreGroundProcessPID);
        kill(foreGroundProcessPID,SIGCONT);
        waitpid(foreGroundProcessPID,&status,WUNTRACED);
            // If child is stopped set the job to stopped
            if (WIFSTOPPED(status)) {
                std::cout << "Job stopped" << std::endl;
                ShellInterpreter::setStoppedJob(foreGroundProcessPID);
            }
        
            // If child exited dispatch the job from the list
            if (WIFEXITED(status)) {
                std::cout << "Job dispatched" << std::endl;
                ShellInterpreter::dispatchJob(foreGroundProcessPID);
            }        
        return false;
    }
    
}

/**
 * 
 * @return 
 */
bool ShellInterpreter::shell_bg(void) {
    if(foreGroundProcessPID != -1) {

       // Set process to background
       ShellInterpreter::setBackgroundJob(foreGroundProcessPID);
       kill(foreGroundProcessPID,SIGCONT);
       
       // Reset foreGroundId
       foreGroundProcessPID = -1;
       return false; 
    }
    
}