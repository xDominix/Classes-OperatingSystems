#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

typedef enum{ NUMBERS, BLOCKS } Mode;

typedef struct
{
	Mode mode;
	int index;
	int noThreads;
	int width;
	int height;
	uint ** inData;
	uint ** outData;
	long timeTaken;
} TaskArgs;

void * threadTask(void * a)
{
	struct timespec begintime;
	clock_gettime(CLOCK_REALTIME, &begintime);
//
	TaskArgs * args = a;
	if(args->mode == NUMBERS)
	{
		int numStart = args->index/args->noThreads*255,numEnd;
		if(args->index == args->noThreads-1) numEnd = 255;
		else numEnd = (args->index+1)/args->noThreads*255-1;

		for(int i = 0; i < args->height; i++)
		{
			for(int j = 0; j < args->width; j++)
			{
				int inValue = args->inData[i][j];
				if(inValue >= numStart && inValue <= numEnd) args->outData[i][j] = 255-inValue;
			}
		}
	}
	else if(args->mode == BLOCKS)
	{
		int startCol = args->index/args->noThreads*args->width,endCol;
		if(args->index == args->noThreads-1) endCol = args->width;
		else	endCol = (args->index+1)/args->noThreads*args->width;

		for(int i = 0; i < args->height; i++)
		{
			for(int j = startCol; j < endCol; j++)
			{
				args->outData[i][j] = 255-args->inData[i][j];
			}
		}

	}
//
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	args->timeTaken = endtime.tv_sec*1e6+endtime.tv_nsec/1000 - begintime.tv_sec*1e6-begintime.tv_nsec/1000;
//
	pthread_exit(&(args->timeTaken));
}

uint ** loadImage(FILE * input, int * width, int * height)
{
	char * header = malloc(sizeof(char) * 512);
	fread(header, sizeof(char), 2, input);
	header[2] = 0;
	if(strcmp(header, "P2") != 0){ puts(header); return NULL; }
	if(fscanf(input, "%d %d\n", width, height) < 2)
	{
		char c;
		do {  c = fgetc(input); } while (c != '\n');
		fscanf(input, "%d %d\n", width, height);
	}
	fscanf(input, "%*d\n");

	uint ** rows = malloc(sizeof(uint*)*(*height));
	for(int i = 0; i < *height; i++) rows[i] = malloc(sizeof(uint)*(*width));
	for(int i = 0; i < *height; i++)	for(int j = 0; j < *width; j++) fscanf(input, "%u", rows[i]+j);

	free(header);
	return rows;
}

uint ** createImage(int width, int height)
{
	uint ** out = malloc(sizeof(uint*)*height);
	for(int i = 0; i < height; i++) out[i] = malloc(sizeof(uint)*(width));
	return out;
}

void saveImage(FILE * output, int width, int height, uint ** input)
{
	fprintf(output, "P2\n%d %d\n255\n", width, height);
	int j;
	for(int i = 0; i < height; i++)
	{
		for(j=0; j < width-1; j++) fprintf(output, "%d ", input[i][j]);
		fprintf(output, "%d\n", input[i][j]);
	}
}

void freeImage(uint ** input, int width, int height){ for(int i = 0; i < height; i++) free(input[i]); free(input); }

int main(int argc, char ** argv)
{
	if(argc < 5){ printf("%s","Wrong no arguments"); return 0; }

	int noThreads;
	if((noThreads = atoi(argv[1])) < 1){ printf("%s","Invalid no threads"); return 0; }
	
	int mode;
	if(strcmp(argv[2], "numbers") == 0){ mode = NUMBERS; }
	else if(strcmp(argv[2], "blocks") == 0){ mode = BLOCKS; }
	else{printf("%s","Invalid mode"); return 0; }
	
	char * fname;
	fname = argv[3];
	FILE * inFile = fopen(fname, "r");
	if(!inFile){ printf("%s","Input file error"); return 0;	}
	fname = argv[4];
	FILE * outFile = fopen(fname, "w+");
	if(!outFile){ printf("%s","Output file error"); return 0; }

	int w, h;
	uint ** image = loadImage(inFile, &w, &h);
	if(image == NULL){ printf("%s","Input file header error"); return 0; }
//
	uint ** newImage = createImage(w, h);
	pthread_t * threads = malloc(sizeof(pthread_t)*noThreads);
	TaskArgs * args = malloc(sizeof(TaskArgs)*noThreads);
//
	struct timespec begintime;
	clock_gettime(CLOCK_REALTIME, &begintime);
//
	for(int i = 0; i < noThreads; i++)
	{
		args[i].mode = mode;
		args[i].index = i;
		args[i].noThreads = noThreads;
		args[i].width = w;
		args[i].height = h;
		args[i].inData = image;
		args[i].outData = newImage;
		pthread_create(threads+i, NULL, threadTask, (void*)(args+i));//run
	}
//
	for(int i = 0; i < noThreads; i++) pthread_join(threads[i], NULL);//wait
//
	struct timespec endtime;
	clock_gettime(CLOCK_REALTIME, &endtime);
	printf("Total time [ms]: %ld\n", (long int)(endtime.tv_sec*1e6+endtime.tv_nsec/1000 - begintime.tv_sec*1e6-begintime.tv_nsec/1000));
//
	saveImage(outFile, w, h, newImage);
//
	freeImage(image, w, h);
	freeImage(newImage, w, h);
	free(threads); free(args);
	fclose(inFile); fclose(outFile);
	return 0;
}

