/* 
 * File:   ShellInterpreter.h
 * Author: debian
 *
 * Created on 24. April 2017, 10:51
 */

#ifndef SHELLINTERPRETER_H
#define	SHELLINTERPRETER_H

#include <iostream>
#include <string.h>
#include <vector>

enum STATE {
    FOREGROUND,
    BACKGROUND,
    STOPPED,
    UNKOWN,
};

typedef struct {
    pid_t pid;
    STATE state;
} process_t;


class ShellInterpreter {
public:
    ShellInterpreter();
    ShellInterpreter(const ShellInterpreter& orig);
    
    void update();
    
    virtual ~ShellInterpreter();
private:
    
    std::string buffer;
    std::vector<char*> execBuffer;
    static std::vector<process_t> processList;
    bool isRunning;
    std::string shell_prompt;
    bool processWait;
    static const std::string cmds[];
    process_t currentProcess;
    
    /*
     * SIGNAL Handler
     */  
    static void sigint_handler(int sig);
    static void sigtstp_handler(int sig);
    static void sigchld_handler(int sig);
    
    void cleanUp(void);
    void getUserInput(void);
    bool formatInput(void);
    void executeProcess(void);
    
    /*
     * SHELL CMDS
     */
    bool shell_logout();
    bool shell_fg();
    bool shell_bg();
    
    /*
     * JOBCONTROL
     */
    static void addJob(pid_t pid);
    static void dispatchJob(pid_t pid);
    static void setForeGroundJob(pid_t pid);
    static void setBackgroundJob(pid_t pid);
    static void setStoppedJob(pid_t pid);

};

#endif	/* SHELLINTERPRETER_H */

