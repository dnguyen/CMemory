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
	 pFile = fopen ("test8_output.txt","w");
    int size;
    int RAM_SIZE=1<<20;//1024*1024
    void* RAM=malloc(RAM_SIZE);//1024*1024
	setup(2,RAM_SIZE,RAM);//Worst Fit, Memory size=1024*1024, Start of memory=RAM
//test 8
    size=20*1024;
	void* a=my_malloc(size);//We have 4 bytes header to save the size of that chunk in memory so the output starts at 4
    if ((int)a==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk a: %d\n",(int)(a-RAM));
    	fprintf(pFile, "End of the chunk a: %d\n\n",(int)(a+size-RAM));
    }

    size=30*1024;
    void* b=my_malloc(size);
    if ((int)b==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk b: %d\n",(int)(b-RAM));
    	fprintf(pFile, "End of the chunk b: %d\n\n",(int)(b+size-RAM));
    }

    size=30*1024;
	void* c=my_malloc(size);
    if ((int)c==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk c: %d\n",(int)(c-RAM));
    	fprintf(pFile, "End of the chunk c: %d\n\n",(int)(c+size-RAM));
    }

    size=30*1024;
	void* d=my_malloc(size);
    if ((int)d==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk d: %d\n",(int)(d-RAM));
    	fprintf(pFile, "End of the chunk d: %d\n\n",(int)(d+size-RAM));
    }

    size=40*1024;
	void* e=my_malloc(size);
    if ((int)e==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk e: %d\n",(int)(e-RAM));
    	fprintf(pFile, "End of the chunk e: %d\n\n",(int)(e+size-RAM));
    }

	my_free(d);
	my_free(b);
	my_free(c);


    size=80*1024;
	void* f=my_malloc(size);
    if ((int)f==-1)
        fprintf(pFile, "This size can not be allocated!");
    else
    {
    	fprintf(pFile, "start of the chunk f: %d\n",(int)(f-RAM));
    	fprintf(pFile, "End of the chunk f: %d\n\n",(int)(f+size-RAM));
    }

    fclose (pFile);
	return 0;
}

