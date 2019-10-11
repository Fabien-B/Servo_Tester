//
// Created by fabien on 10/10/2019.
//

#ifndef EMBEDDED_MAINMENUSTATE_H
#define EMBEDDED_MAINMENUSTATE_H

#include "AbstractState.h"


class MainMenuState : public AbstractState {
public:
    void enter(int32_t pos_enc);
    AbstractState* onUiEvent(struct UiState ui_state);
    AbstractState* periodic();
    void leave();

private:
    int32_t pos_enc_init;

};

extern MainMenuState mainMenuState;

#endif //EMBEDDED_MAINMENUSTATE_H
