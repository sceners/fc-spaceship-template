/* Dummy serial generation.
It is only a template! */

#include <windows.h>

void process_serial(char *serial)
{
	static char charset[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;
	for(i=0; i<41; i++)
	{
		if((i+1) % 6 == 0)
			serial[i] = '-';
		else
			serial[i] = charset[rand() % (sizeof(charset) - 1)];
	}
}
