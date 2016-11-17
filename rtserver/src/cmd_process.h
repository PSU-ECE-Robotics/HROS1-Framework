#ifndef _DXL_MANAGER_CMD_PROCESS_H_
#define _DXL_MANAGER_CMD_PROCESS_H_

#include "LinuxDARwIn.h"

void change_current_dir();
void sighandler(int sig);

void OnOffCmd(Robot::ArbotixPro &arbotixpro, bool on);
void TurnON(Robot::ArbotixPro &arbotixpro);
void TurnOFF(Robot::ArbotixPro &arbotixpro);

#endif
