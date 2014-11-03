#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memalloc.h"
#include <stdbool.h>
//***********************************************************************************
int main()
{
	 FILE * pFile;
	 pFile = fopen ("test3_output.txt","w");
    int size;
    int RAM_SIZE=1<<20;//1024*1024
    void* RAM=malloc(RAM_SIZE);//1024*1024
	setup(2,RAM_SIZE,RAM);//Worst Fit, Memory size=1024*1024, Start of memory=RAM
//test 3
    size=20*1024;
	void* a=my_malloc(size);//We have 4 bytes header to save the size of that chunk in memory so the output starts at 4
    if ((int)a==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk a: %d\n",a-RAM);
    	fprintf(pFile, "End of the chunk a: %d\n\n",a+size-RAM);
    }

    size=30*1024;
    void* b=my_malloc(size);
    if ((int)b==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk b: %d\n",b-RAM);
    	fprintf(pFile, "End of the chunk b: %d\n\n",b+size-RAM);
    }

    size=15*1024;
	void* c=my_malloc(size);
    if ((int)c==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk c: %d\n",c-RAM);
    	fprintf(pFile, "End of the chunk c: %d\n\n",c+size-RAM);
    }

    size=25*1024;
	void* d=my_malloc(size);
    if ((int)d==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk d: %d\n",d-RAM);
    	fprintf(pFile, "End of the chunk d: %d\n\n",d+size-RAM);
    }

    size=35*1024;
	void* e=my_malloc(size);
    if ((int)e==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk e: %d\n",e-RAM);
    	fprintf(pFile, "End of the chunk e: %d\n\n",e+size-RAM);
    }

    size=35*1024;
	void* f=my_malloc(size);
    if ((int)f==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk f: %d\n",f-RAM);
    	fprintf(pFile, "End of the chunk f: %d\n\n",f+size-RAM);
    }

    size=25*1024;
	void* g=my_malloc(size);
    if ((int)g==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk g: %d\n",g-RAM);
    	fprintf(pFile, "End of the chunk g: %d\n\n",g+size-RAM);
    }

	my_free(b);
	my_free(d);
	my_free(f);

    size=25*1024;
	void* h=my_malloc(size);
    if ((int)h==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk h: %d\n",h-RAM);
    	fprintf(pFile, "End of the chunk h: %d\n\n",h+size-RAM);
    }

    fclose (pFile);
	return 0;
}
