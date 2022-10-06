#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For reading user input for exiting the program without memorys leeks.
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


#ifdef COMPILEDEBUG
	#define ALLOCATOR_DEBUG
	#define ALLOCATOR_DEBUG_BORDERCHECK 6
	#define ALLOCATOR_RUN_AFTERCHECK
#endif

#include "Libs/File.c"
#include "Libs/Folder.c"
#include "Libs/Memory.c"
#include "Libs/String.c"
#include "Libs/LinkedList.c"
#include "Libs/StateMachine.c"

#include "Libs/Allocator.c"
#include "Libs/Server/Filesystem_Server.c"

int kbhit(void);

StateMachine g_StateMachine;

int main(int argc, char* argv[])
{
	#ifdef ALLOCATOR_DEBUG
		Allocator_Open("AllocatorDebug.txt");
	#endif

	int doExit = 1;

	StateMachine_Initialize(&g_StateMachine);
	
	Filesystem_Server* server = NULL;
	int success = Filesystem_Server_InitializePtr(&g_StateMachine, "Shared", &server);
	
	printf("Success: %i\r\n", success);

	struct timespec tim, tim2;
	tim.tv_sec = 0;
	tim.tv_nsec = 0;
	while(doExit == 1)
	{
		StateMachine_Work(&g_StateMachine);
		
		if(kbhit())
		{
			UInt8 chr = getchar();
			if(chr == 'q')
				doExit = 0;

			else if(chr == 'c')
				system("clear");
			
			else
				printf(">%c", chr);

		}

		nanosleep(&tim, &tim2);
	}

	if(server != NULL)
		Filesystem_Server_Dispose(server);

	StateMachine_Dispose(&g_StateMachine);

	#ifdef ALLOCATOR_DEBUG
		Allocator_Close();
	#endif
	
	return 0;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}