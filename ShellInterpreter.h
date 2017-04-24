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

class ShellInterpreter {
public:
    ShellInterpreter();
    ShellInterpreter(const ShellInterpreter& orig);
    
    void update();
    
    virtual ~ShellInterpreter();
private:
    
    std::string buffer;
    std::vector<char*> execBuffer;
    bool isRunning;
    std::string shell_prompt;
    bool processWait;
    
    void cleanUp(void);
    void getUserInput(void);
    bool formatInput(void);
    void executeProcess(void);
    

};

#endif	/* SHELLINTERPRETER_H */

