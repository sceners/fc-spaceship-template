#include <windows.h>
#include "minifmod.h"
#include "rsrc/resource.h"

typedef struct 
{
	int length;
	int pos;
	void *data;
} MEMFILE;


unsigned int memopen(char *name)
{
	HRSRC	res;
	HGLOBAL	handle;
	MEMFILE *memfile;

	memfile = (MEMFILE *)calloc(sizeof(MEMFILE),1);

	res = FindResource(NULL, MAKEINTRESOURCE(ID_MYMOD), "XMMOD");
	handle = LoadResource(NULL, res);

	memfile->length = SizeofResource(NULL, res);
	memfile->data = malloc(memfile->length);
	memfile->pos = 0;
	memcpy(memfile->data, LockResource(handle), memfile->length);

	return (unsigned int)memfile;
}

void memclose(unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;
	free(memfile->data);
}

int memread(void *buffer, int size, unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;
	if (memfile->pos + size >= memfile->length)
	{
		size = memfile->length - memfile->pos;
	}
	memcpy(buffer, (char *)memfile->data+memfile->pos, size);
	memfile->pos += size;
	return size;
}

void memseek(unsigned int handle, int pos, signed char mode)
{
	MEMFILE *memfile = (MEMFILE *)handle;
	if (mode == SEEK_SET) 
		memfile->pos = pos;
	else if (mode == SEEK_CUR) 
		memfile->pos += pos;
	else if (mode == SEEK_END)
		memfile->pos = memfile->length + pos;
	if (memfile->pos > memfile->length)
		memfile->pos = memfile->length;
}

int memtell(unsigned int handle)
{
	MEMFILE *memfile = (MEMFILE *)handle;
	return memfile->pos;
}

void loadmusic(void)
{
	FSOUND_File_SetCallbacks(memopen, memclose, memread, memseek, memtell);
}
