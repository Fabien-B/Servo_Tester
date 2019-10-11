//
// Created by fabien on 10/10/2019.
//

#ifndef EMBEDDED_SERVOSTATE_H
#define EMBEDDED_SERVOSTATE_H

#include "AbstractState.h"


class ServoState : public AbstractState {
public:
    ServoState();
    void enter(int32_t pos_enc);
    AbstractState* onUiEvent(struct UiState ui_state);
    AbstractState* periodic();
    void leave();
    static const uint16_t SERVO_MIN = 500;
    static const uint16_t SERVO_MAX = 2500;

private:
    int32_t pos_enc_init;
    uint16_t servo_positions[5];
    bool driving_servo;
    int no_servo;
    char txt_pos_servo[16];
    int32_t last_pos_enc;

};

extern ServoState servoState;


#endif //EMBEDDED_SERVOSTATE_H
