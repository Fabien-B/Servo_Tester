//
// Created by fabien on 10/10/2019.
//

#ifndef EMBEDDED_ABSTRACTSTATE_H
#define EMBEDDED_ABSTRACTSTATE_H

#include "stdint.h"
#include <ch.h>
#include <hal.h>

struct UiState {
    UiState(volatile UiState &uis); // chMtxLock(&mut);  
    UiState();
    void clearEvents() volatile;

    bool ok_clicked;
    bool ret_clicked;
    bool enc_changed;
    int32_t pos_enc;
    uint16_t encoder_speed;
    MUTEX_DECL(mut);
};

class AbstractState {
public:
    virtual void enter(int32_t pos_enc) = 0;
    virtual AbstractState* onUiEvent(struct UiState ui_state) = 0;
    virtual AbstractState* periodic() = 0;
    virtual void leave() = 0;

};


#endif //EMBEDDED_ABSTRACTSTATE_H
