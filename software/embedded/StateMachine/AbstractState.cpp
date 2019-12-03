#include "AbstractState.h"

UiState::UiState() {
    ok_clicked = false;
    ret_clicked = false;
    enc_changed = false;
    pos_enc = 0;
    encoder_speed = 1;
}


UiState::UiState(volatile UiState &uis) {
    chMtxLock((ch_mutex*)&(uis.mut));
    ok_clicked = uis.ok_clicked;
    ret_clicked = uis.ret_clicked;
    enc_changed = uis.enc_changed;
    pos_enc = uis.pos_enc;
    encoder_speed = uis.encoder_speed;
    chMtxUnlock((ch_mutex*)&(uis.mut));
}

void UiState::clearEvents() volatile {
    ok_clicked = false;
    ret_clicked = false;
    enc_changed = false;
}