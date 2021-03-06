#ifndef __SDP3X_H
#define __SDP3X_H
#include "sys.h"

#define SDP3X_ADDR1 0x42
#define SDP3X_ADDR2 0x44
#define SDP3X_ADDR3 0x46


void Sdp3xInit(u8 id);
void Sdp3xSetMode(u8 id,u8 addr,u16 cmd);
void Sdp3xReadOut(u8 id,u8 addr,u8 n,s16 data[3]);

#endif