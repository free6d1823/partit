#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


static void usage(char* name)
{
    printf("\nNAME\n");
    printf("\tpartition a file into several files\n\n");
    printf("SYNOPSIS\n");
    printf("\t%s -s <size>[M|K] -o output-file\n\n", name);
    printf("\t%s -n <num> -o output-file\n\n", name);
 
    printf("DESCRIPTION\n");
    printf("\t\t -s size   bytes of each file\n");
    printf("\t\t -n num    numbers of files with equal size\n");
    printf("\n\n");
}

off_t hex2int(char* hex)
{
	off_t val = 0;
	while (*hex && *hex != ' ') {
		unsigned char data = *hex++;
		if(data >= '0' && data <= '9')
		    data = data - '0';
		else if(data >= 'a' && data <= 'f')
		    data = data - 'a' + 10;
		else if (data >= 'A' && data <= 'F')
		    data = data - 'A' +10;
		else
			break;
		val = ( val <<4) | (data & 0xFF);
	}
    return val;
}
unsigned long convert_size(char* string)
{
	unsigned long len;
	unsigned long base = 1;
	char* p = string;
	int n = strlen(string);
	if( p[n-1] == 'M') {
		base = 1024*1024;
		p[n-1] = 0;
	}
	else if( p[n-1] == 'K') {
		base = 1024;
		p[n-1] = 0;
	}
	if (n>3 && p[0] == '0' && p[1] == 'x') {
		len = hex2int(p+2);
	}else	
		len = atoi(p);
	
	return len*base;
} 

int main(int argc, char* argv[])
{
	char* outputfile = NULL;
	char* inputfile = NULL;
	FILE* fin;
	FILE* fout;
    int ch;
    int totalNum = 0;
    unsigned long toalSize=0;
    unsigned long oneSize=0;
    int mode = -1; //0 by size, 1 by number
    
    while ((ch = getopt(argc, argv, "s:n:o:h?")) != -1)
    {
        switch (ch) {
        case 'o':
            outputfile = optarg;
            break;
        case 's':
	    	mode = 0;
	    	oneSize= convert_size(optarg);
            break;
        case 'n':
	    	mode = 1;
	    	totalNum = atoi(optarg);
            break;
	    break;
        case 'h':
        case '?':
        default:
            usage(argv[0]);
            exit(-1);
        }   
    }
    inputfile = argv[optind];
    if (! inputfile[0] || mode <0 ) {
    	usage(argv[0]);
    	exit(-1);
    }
     if (! outputfile  ) {
    	usage(argv[0]);
    	exit(-1);
    }   
    if((mode == 0 && oneSize < 16)||(mode ==1 && (totalNum <2 ||totalNum > 16)  )) {
    	printf("invalid parameters!\n");
    	exit(-3);
    }
    fin = fopen(inputfile, "rb");
    if (!fin) {
    	fprintf(stderr, "cannot open file %s\n", inputfile);
    	exit(-2);
    }
    fseek(fin, 0, SEEK_END);
    toalSize = ftell(fin);
    if(mode == 1 ) {
    	oneSize = (toalSize+ totalNum -1)/totalNum;
    } else {
    	totalNum = (toalSize + oneSize-1)/oneSize;
    }
    
	printf ("Partition %s (%ld bytes) into %d files, each %ld bytes...\n", inputfile, toalSize, totalNum, oneSize);
	
	int i;
	char* buffer = (char*) malloc(oneSize);
	char name[256];
	fseek(fin, 0, SEEK_SET);
	for (i=0; i< totalNum; i++) {
		size_t read, writen;
		sprintf(name, "%s.%d", outputfile, i+1);
		fout = fopen(name, "wb");
		if (!fout) {
			fprintf(stderr, "Failed to open file %s!\n", name);
			break;
		}
		read = fread(buffer, 1, oneSize, fin);
		if ( read < 0) {
			fprintf(stderr, "Failed to read file. Error = %d\n", errno);
			fclose(fout);
			break;
		}
		writen = fwrite(buffer, 1, read, fout);
		printf("%s - %ld bytes\n", name, writen);	
		fclose(fout);
	}
	fclose(fin);
	return 0;
}
