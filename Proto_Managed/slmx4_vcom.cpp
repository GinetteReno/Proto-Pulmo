// C library headers

#include "slmx4_vcom.h"

#include <iostream>
#include <errno.h> // Error integer and strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()


#define BUFF_SIZE 32

#define UPDATE_NUM_SAMPLERS 0






class slmx4
{
	void Begin();
	void init_device();
	void init_serial();
	void termios_config();
	void updateNumberOfSamplers();

	void OpenRadar();
	void CloseRadar();
	void* getData();
	void* Item(int CMD);

	int device_id;
	struct termios tty;

	int status;
	int numSamplers;
	int isOpen;

};


slmx4::slmx4()
{
	device_id = -1;
	isOpen = 0;
	numSamplers = 0;
	status = -1;
}

void slmx4::Begin()
{
	init_serial();
	init_device();
	OpenRadar();
	//cout << "bins = " << numSamplers << endl;
}

void slmx4::init_device()
{
	int id = device_id;
	int nbwrite = 0;

	if(id)
	{
		char* _buff = "NVA_CreateHandle()";
		nbwrite = write(id, _buff, sizeof(_buff));
		if(nbwrite != sizeof(_buff))
			fprintf(stderr, "WARNING[SIZE MISSMATCH]: 'nbwrite = write(id, _buff, sizeof(_buff));' and match char* _buff = 'NVA_CreateHandle()';");
	}


}

void* slmx4::getData()
{
	int id = device_id;
	int* _packetlength;
	char _rbuff[BUFF_SIZE];

	read(id, _packetlength, sizeof(unsigned int));
	read(id, _rbuff, sizeof(unsigned char));

	return _rbuff;
}

void slmx4::OpenRadar()
{
	int id = device_id;
	char cmd[14] = "OpenRadar(X4)";
	write(id, &cmd, sizeof(unsigned char));

	status = *((int*)getData());

	isOpen = 1;

	updateNumberOfSamplers();
}

void slmx4::CloseRadar()
{
	int id = device_id;
	char cmd[8] = "Close()";
	for(int i = 0; i < 8; ++i)
		write(id, &cmd+i, sizeof(char));
}

void* slmx4::Item(int CMD)
{
	static int _rval;
	int id = device_id;

	switch(CMD)
	{
	case UPDATE_NUM_SAMPLERS:
		char _cmd[] = "VarGetValue_ByName(SamplersPerFrame)";
		for(int i = 0; i < sizeof(_cmd); ++i)
				write(id, &_cmd, sizeof(char));
		break;
	default:
		break;
	}

	_rval = atoi((char*)getData());
	return (void*)&_rval;
}

void slmx4::updateNumberOfSamplers()
{
	int* _val = (int*)Item(UPDATE_NUM_SAMPLERS);
	numSamplers = *_val;
}

//void slmx4 TryUpdateChip(char* reg,)
//{
//
//}

void slmx4::init_serial()
{
	device_id = open("/dev/ttyUSB0", O_RDWR);

	// Check for errors
	if (device_id < 0) {
	    printf("Error %i from open: %s\n", errno, strerror(errno));
	}

	// Read in existing settings, and handle any error
	// Note: initialize struct with a call to tcgetattr() overwise behaviour
	// is undefined
	if(tcgetattr(device_id, &tty) != 0) {
	    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	}

	termios_config();

	// Set in/out baud rate to be 9600
	cfsetspeed(&tty, B9600);	//both
	//cfsetispeed(&tty, B9600);	//input
	//cfsetospeed(&tty, B9600); //output

	// Save tty settings, also checking for error
	if (tcsetattr(device_id, TCSANOW, &tty) != 0) {
	    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}
}

void slmx4::termios_config()
{
	tty.c_cflag &= ~PARENB; 		// Clear parity bit, disabling parity
	tty.c_cflag &= ~CSTOPB;			// Clear stop field, only one stop bit used in communication
	tty.c_cflag |= CS8; 			// 8 bits per byte
	tty.c_cflag &= ~CRTSCTS; 		// Disable RTS/CTS hardware flow control
	tty.c_cflag |= CREAD | CLOCAL;	// Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;	// Disable canonical mode
	tty.c_lflag &= ~ECHO; 	// Disable echo
	tty.c_lflag &= ~ECHOE; 	// Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; 	// Disable interpretation of INTR, QUIT and SUSP

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); 						 // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevzent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

	tty.c_cc[VTIME] = 10;  // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 0;
}
