#ifndef PROCESSPIPE_H
#define PROCESSPIPE_H
#include <sys/wait.h>
#include <sstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <stdexcept>

void parse(const std::string&, char**);

class ProcessPipe
{
public:
	ProcessPipe(const std::string&);
	~ProcessPipe(){kill(pid_, SIGTERM);waitpid(pid_, 0, 0);};
	std::string GetOutput();
	void ReadUntilOutputIs(const std::string&);
	void Input(const std::string&);
private:
	pid_t pid_;
	// 2 pipes for 2 ways piping
	int p2c_pipe_[2];
	int c2p_pipe_[2];
	// file stream for reading and writing
	FILE *p2c_, *c2p_;
};

#endif	
