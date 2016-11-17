#include <iostream>
#include <ncurses.h>
#include <signal.h>
#include <string.h>
#include <term.h>
#include <termios.h>
#include <unistd.h>
#include <zmq.hpp>

#include "cmd_process.h"

#define MOTION_FILE_PATH    "HROS1-Framework/Data/motion_4096.bin"
#define INI_FILE_PATH       "HROS1-Framework/Data/config.ini"
#define U2D_DEV_NAME0       "/dev/ttyUSB0"

using namespace Robot;
using namespace std;

int main(int argc, char *argv[])
{
	LinuxArbotixPro linux_arbotixpro(U2D_DEV_NAME0);
	ArbotixPro arbotixpro(&linux_arbotixpro);

	signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGQUIT, &sighandler);
    signal(SIGINT,  &sighandler);

    char motionFile[128];

    change_current_dir();

    minIni* ini = new minIni(INI_FILE_PATH);

	strcpy(motionFile, MOTION_FILE_PATH);
    if (Action::GetInstance()->LoadFile(motionFile) == false)
	{
		cout << "Can not open " << motionFile << endl;
		exit(0);
	}

    if (MotionManager::GetInstance()->Initialize(&arbotixpro) == false)
	{
		cout << "Initializing Motion Manager failed!" << endl;
		exit(0);
	}

    Walking::GetInstance()->LoadINISettings(ini);
    MotionManager::GetInstance()->LoadINISettings(ini);

    MotionManager::GetInstance()->SetEnable(true);

    LinuxMotionTimer linuxMotionTimer;
    linuxMotionTimer.Initialize(MotionManager::GetInstance());
    linuxMotionTimer.Start();

    //  Prepare our context and socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:5555");
    string ack = "ack";

    while (true)
    {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv(&request);
        string cmd = (const char*)request.data();

        if (strcmp("exit", cmd.c_str()) == 0)
        {
            cout << "cmd " << cmd << endl;

            break;
        }

        if (strcmp("reset", cmd.c_str()) == 0)
        {
            cout << "cmd " << cmd << endl;

        	Action::GetInstance()->m_Joint.SetEnableBody(false);
        	Walking::GetInstance()->m_Joint.SetEnableBody(false);
            MotionManager::GetInstance()->SetEnable(false);

            break;
        }

		if (strcmp("walk", cmd.c_str()) == 0)
        {
	        cout << "cmd " << cmd << endl;

	        MotionManager::GetInstance()->AddModule((MotionModule*)Walking::GetInstance());
        	Walking::GetInstance()->m_Joint.SetEnableBody(true);
        	MotionManager::GetInstance()->ResetGyroCalibration();
        	usleep(8000);

        	Walking::GetInstance()->Start();
        	usleep(8000000);

        	Walking::GetInstance()->Stop();

        	while (Walking::GetInstance()->IsRunning())
        		usleep(8000);

        	MotionManager::GetInstance()->ResetGyroCalibration();
        	usleep(8000);

        	Walking::GetInstance()->m_Joint.SetEnableBody(false);
        	MotionManager::GetInstance()->RemoveModule((MotionModule*)Walking::GetInstance());
        }
		if (strcmp("page", cmd.c_str()) == 0)
        {
	        //  Send reply back to client
	        zmq::message_t reply(ack.length()+1);
	        memcpy(reply.data(), ack.c_str(), ack.length()+1);
	        socket.send(reply);

	        zmq::message_t quantaRequest;

	        //  Wait for specific action
	        socket.recv(&quantaRequest);
	        string quanta = (const char*)quantaRequest.data();
	        cout << "cmd " << cmd << " " << quanta << endl;

        	MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
        	Action::GetInstance()->m_Joint.SetEnableBody(true);

        	Action::GetInstance()->Start(atoi(quanta.c_str()));

        	while (Action::GetInstance()->IsRunning())
        		usleep(8000);

        	Action::GetInstance()->Stop();

        	while (Action::GetInstance()->IsRunning())
        		usleep(8000);

        	Action::GetInstance()->m_Joint.SetEnableBody(false);
        	MotionManager::GetInstance()->RemoveModule((MotionModule*)Action::GetInstance());
        }

        //  Send reply back to client
        zmq::message_t reply(ack.length()+1);
        memcpy(reply.data(), ack.c_str(), ack.length()+1);
        socket.send(reply);
    }

	TurnOFF(arbotixpro);
	usleep(8000);

	MotionManager::GetInstance()->RemoveModule((MotionModule*)Walking::GetInstance());
	MotionManager::GetInstance()->RemoveModule((MotionModule*)Action::GetInstance());

	Action::GetInstance()->m_Joint.SetEnableBody(false);
	Walking::GetInstance()->m_Joint.SetEnableBody(false);
    MotionManager::GetInstance()->SetEnable(false);

	linuxMotionTimer.Stop();

    exit(0);
}
