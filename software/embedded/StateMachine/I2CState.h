#if !defined(I2CSTATE_H_)
#define I2CSTATE_H_

#include "AbstractState.h"
#include "VL6180x.h"

class I2CState : public AbstractState {
public:
    I2CState();
    void enter(int32_t pos_enc);
    AbstractState* onUiEvent(struct UiState ui_state);
    AbstractState* periodic();
    void leave();

private:
    int32_t pos_enc_init;
    int32_t last_pos_enc;

};

extern I2CState i2cState;


///////////////////////  SRF02  //////////////////////////////////

class SRF02State : public AbstractState {
public:

    enum SRF02SubState {
        SRF02_SCAN,
        SRF02_RANGE_AND_MENU,
        SRF02_CHANGING_ADDR
    };


    SRF02State();
    void enter(int32_t pos_enc);
    AbstractState* onUiEvent(struct UiState ui_state);
    AbstractState* periodic();
    void leave();

private:
    int32_t pos_enc_init;
    int32_t last_pos_enc;
    SRF02SubState substate;
    uint8_t current_addr;

    void next_address(void);

};

extern SRF02State srf02State;


/////////////////////////  VL6180x  /////////////////////////////////

class VL6180xState : public AbstractState {
public:

    VL6180xState();
    void enter(int32_t pos_enc);
    AbstractState* onUiEvent(struct UiState ui_state);
    AbstractState* periodic();
    void leave();

private:

    void range_and_display(void);
    int32_t pos_enc_init;
    int32_t last_pos_enc;
    uint8_t current_addr;

};

extern VL6180xState vl6180xState;

#endif // I2CSTATE_H_
