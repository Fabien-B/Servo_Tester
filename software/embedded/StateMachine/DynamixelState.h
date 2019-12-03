//
// Created by fabien on 10/10/2019.
//

#ifndef EMBEDDED_DYNAMIXELSTATE_H
#define EMBEDDED_DYNAMIXELSTATE_H

#include "AbstractState.h"
#include "DynamixelSerial.h"

enum DynamixelFunction {
    E_DynamixelMove,
    E_DynamixelSetId,
    E_DynamixelSetSpeed,
    E_DynamixelSetTorque,
};

class DynamixelIDState : public AbstractState {
public:
    DynamixelIDState();
    void enter(int32_t pos_enc);
    AbstractState* onUiEvent(struct UiState ui_state);
    AbstractState* periodic();
    void leave();

    uint8_t get_dynamixel_id() {
        return dynamixel_id;
    }

    void set_dynamixel_id(uint8_t new_id) {
        dynamixel_id = new_id;
    }

private:
    int32_t pos_enc_init;
    char txt_id[17];
    uint8_t dynamixel_id;

};

////////////////////////////////////////////////////////////////////////////////

class DynamixelFnMenuState : public AbstractState {
public:
    DynamixelFnMenuState();
    void enter(int32_t pos_enc);
    AbstractState* onUiEvent(struct UiState ui_state);
    AbstractState* periodic();
    void leave();
    DynamixelFunction get_function() {
        return selectedFn;
    }

private:
    int32_t pos_enc_init;
    DynamixelFunction selectedFn;

};

////////////////////////////////////////////////////////////////////////////////

class DynamixelMoveState : public AbstractState {
public:
    DynamixelMoveState();
    void enter(int32_t pos_enc);
    AbstractState* onUiEvent(struct UiState ui_state);
    AbstractState* periodic();
    void leave();

private:
    int32_t pos_enc_init;
    int32_t last_enc;
    int16_t pos;
    char txt_position[17];
    DynamixelSerial::RotationDirection endless_direction;

};

extern DynamixelIDState dynamixelIDState;
extern DynamixelFnMenuState dynamixelFnMenuState;
extern DynamixelMoveState dynamixelMoveState;


#endif //EMBEDDED_DYNAMIXELSTATE_H
