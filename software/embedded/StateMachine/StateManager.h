//
// Created by fabien on 10/10/2019.
//

#ifndef EMBEDDED_STATEMANAGER_H
#define EMBEDDED_STATEMANAGER_H

#include "AbstractState.h"

class StateManager {
public:
    void init();
    void run();

private:
    AbstractState* current_state;

};


#endif //EMBEDDED_STATEMANAGER_H
