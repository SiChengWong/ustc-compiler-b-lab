#include "stdio.h"

extern void SyntaxAnalysis();

FILE *sFile;
 
int main(int argc, char* argv[])
{
//    sFile=fopen_s(argv[1],"r");
    sFile=fopen("test.txt","rt");
	SyntaxAnalysis();
	fclose(sFile);

//Free all memories

	return 0;
}
