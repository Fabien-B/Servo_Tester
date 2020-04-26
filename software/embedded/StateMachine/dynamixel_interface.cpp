#include "dynamixel_interface.h"
#include "DynamixelState.h"
#include <hal.h>

void dynamixelMove(int id, int pos) {
    dyn.move(id, pos);
}

void dynamixelMoveSpeed(int id, int pos, int speed) {
    dyn.moveSpeed(id, pos, speed);
}
