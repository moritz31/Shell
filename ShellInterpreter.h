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
    std::vector<process_t> processList;
    bool isRunning;
    std::string shell_prompt;
    bool processWait;
    static const std::string cmds[];
    process_t currentProcess;
    
    void cleanUp(void);
    void getUserInput(void);
    bool formatInput(void);
    void executeProcess(void);
    
    bool shell_logout();
    bool shell_fg();
    bool shell_bg();
    

};

#endif	/* SHELLINTERPRETER_H */

