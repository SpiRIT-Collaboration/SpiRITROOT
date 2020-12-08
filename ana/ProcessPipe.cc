#include "ProcessPipe.hh"

ProcessPipe::ProcessPipe(const std::string& t_command)
{
	if(pipe(p2c_pipe_)!=0)
		throw std::runtime_error("Pipe parent to child failed to create");
	if(pipe(c2p_pipe_)!=0)
		throw std::runtime_error("Pipe child to parent failed to create");
	pid_ = fork();
	if(pid_ < 0)
		throw std::runtime_error("Failed to fock");
	// execute if it is the child process
	if(pid_ == (pid_t)0)
	{
		close(0);
		close(1);
		close(c2p_pipe_[0]);
		close(p2c_pipe_[1]);
		dup2(p2c_pipe_[0],0);
		dup2(c2p_pipe_[1],1);
		// parse entered command correctly
		char *argv[64];
		parse(t_command.c_str(), argv);
		execvp(*argv, argv);
		exit(EXIT_FAILURE);
	}else
	{
		close(p2c_pipe_[0]);
		close(c2p_pipe_[1]);
		c2p_ = fdopen(c2p_pipe_[0], "r");
		p2c_ = fdopen(p2c_pipe_[1], "w");
		//waitpid(pid_, NULL, 0);
	}
}

std::string ProcessPipe::GetOutput()
{
	char buffer[10000];
	fgets(buffer, sizeof(buffer)-1, c2p_);
	std::string answer(buffer);
	//int bytes_read = read(c2p_pipe_[0], buffer, sizeof(buffer)-1);
	//std::cout << "bytes read " << bytes_read << "\n";
	//std::cout << "size of char " << sizeof(buffer) << "\n";
	return answer;
}

void ProcessPipe::ReadUntilOutputIs(const std::string& t_tobecompared)
{
	char tobecompared[10000];
	strcpy(tobecompared, t_tobecompared.c_str());
	char buffer[10000];
	for(;fgets(buffer, sizeof(buffer)-1, c2p_), strcmp(buffer, tobecompared) !=0/* && buffer && !buffer[0]*/;){};
}

void ProcessPipe::Input(const std::string& t_input)
{
	fprintf(p2c_, "%s\n", t_input.c_str());
	fflush(p2c_);
}

void  parse(const std::string& t_line, char **argv)
{
	char *line = new char[t_line.length() + 1];
	std::strcpy(line, t_line.c_str());
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;             /* skip the argument until ...    */
     }
     *argv = (char*) '\0';                 /* mark the end of argument list  */
}
