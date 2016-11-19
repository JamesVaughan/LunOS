#ifndef PROMPT_H
#define PROMPT_H 0
#include <String.h>

typedef struct {
	bool params;
	LunOS::String name;
	void (*command)();
	void (*paramCommand) (int argc, unsigned char** argv);
} Command;

typedef struct{
	Command list[80];
	unsigned int ammount;
} CommandList;

class CommandPrompt{
public:
	void Init(void);
	void ResetPosition(void);
	void Run(void);
	void AddCommand(Command* c);
	void AddParamCommand(Command* c);
};

#endif

