#ifndef DYNAMIXEL_INTERFACE_H
#define DYNAMIXEL_INTERFACE_H

#ifdef __cplusplus
  extern "C" {
#endif

void dynamixelMove(int id, int pos);
void dynamixelMoveSpeed(int id, int pos, int speed);

#ifdef __cplusplus
  }
#endif

#endif