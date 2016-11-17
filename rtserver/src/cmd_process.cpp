#include <fcntl.h>
#include <libgen.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <term.h>

#include "cmd_process.h"

using namespace Robot;

void change_current_dir()
{
    char exepath[1024] = {0};
    if (readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

void sighandler(int sig)
{
    struct termios term;
    tcgetattr( STDIN_FILENO, &term );
    term.c_lflag |= ICANON | ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &term );

    exit(0);
}

void OnOffCmd(ArbotixPro &arbotixpro, bool on)
{
	for (int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
		arbotixpro.WriteByte(id, AXDXL::P_TORQUE_ENABLE, (int)on, 0);

	arbotixpro.DXLPowerOn(on);
}

void TurnON(Robot::ArbotixPro &arbotixpro)
{
	OnOffCmd(arbotixpro, true);
}

void TurnOFF(Robot::ArbotixPro &arbotixpro)
{
	OnOffCmd(arbotixpro, false);
}
