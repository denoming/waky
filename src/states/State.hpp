#pragma once

class State {
public:
    virtual void enterState() = 0;

    virtual bool run() = 0;
    
    virtual void exitState() = 0;    
};