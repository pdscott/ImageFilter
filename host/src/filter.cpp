#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <algorithm>
#include <pthread.h>
#include <sys/time.h>
#include "filter.h"

int main(int argc, char** argv) {
	FILE *in, *out;
	char *buffer, *header, *src, *dst;
	int *src_bottom, *dst_bottom;
	thread_data params;
	int hsize, isize, width, height;
	int mode, filtertype;
	struct timeval time_start, time_end;

	if(argc < 3) {
		fprintf(stderr,"Usage: %s <input_file> <mode (1-3)> <filter type (1-4)>\n",argv[0]);
		printf("Modes: 1) Single Core 2) Dual Core 2) FPGA\n");
		printf("Algorithms: 1) Identity 2) Edge Detection 3) Guassian Blur\n"); 
		return EXIT_FAILURE;;   	         
	}
	in = fopen(argv[1],"rb");
	mode = atoi(argv[2]);
	filtertype = atoi(argv[3]);

	if(in == NULL) {
		fprintf(stderr,"Cannot open %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	buffer = (char*) malloc(80);
	fread(buffer, sizeof(char), 80, in);
	memcpy(&isize, &buffer[2], sizeof(int));
	memcpy(&hsize, &buffer[10], sizeof(int));
	memcpy(&width, &buffer[18], sizeof(int));
	memcpy(&height, &buffer[22], sizeof(int));
	width = abs (width);
	height = abs (height);
	header = (char*) malloc(hsize);
	src = (char*) malloc(isize);
	dst = (char*) malloc(isize);
	rewind(in);
	fread(header, sizeof(char), hsize, in);
	fseek (in, hsize, SEEK_SET);
	fread(src, sizeof(char), isize, in);
	int *src_pixels = (int *) src;
	int *dst_pixels = (int *) dst;
	switch(mode) {
        case 1: //Single Core
        gettimeofday(&time_start, NULL);
        filter(src_pixels, dst_pixels, width, height, filtertype, 0);
        gettimeofday(&time_end, NULL);
        fprintf(stderr,"Filtering on single-core: %ld\n",((time_end.tv_sec * 1000000 + 
        	    time_end.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec)));
        break;
        case 2:  //Dual Core
        pthread_t tid;
        gettimeofday(&time_start, NULL);
        src_bottom = &src_pixels[(height/2 - 2)* width];
        dst_bottom = &dst_pixels[(height/2 - 2) * width];
        params.src = src_bottom;
        params.dst = dst_bottom;
        params.w = width;
        params.h = height/2 + 2;
        params.filtertype = filtertype;
        params.section = 2;
        pthread_create(&tid, NULL, filter_t, &params);
        filter(src_pixels, dst_pixels, width, height/2 + 2, filtertype, 1);
        pthread_join(tid, NULL);
        gettimeofday(&time_end, NULL);
        fprintf(stderr,"Filtering on multiple cores: %ld\n",((time_end.tv_sec * 1000000 + 
        	    time_end.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec)));
        break;
        case 3:  // FPGA
        gettimeofday(&time_start, NULL);
        //fpga_filter(src, dst, width, height, filtertype);
        gettimeofday(&time_end, NULL);
        fprintf(stderr,"Filtering on FPGA: %ld\n",((time_end.tv_sec * 1000000 + time_end.tv_usec) - 
        	   (time_start.tv_sec * 1000000 + time_start.tv_usec)));
        break;
        default: // Single Core
        gettimeofday(&time_start, NULL);
        filter(src_pixels, dst_pixels, width, height, filtertype, 0);
        gettimeofday(&time_end, NULL);
        fprintf(stderr,"Filtering on single-core: %ld\n",((time_end.tv_sec * 1000000 + 
        	    time_end.tv_usec) - (time_start.tv_sec * 1000000 + time_start.tv_usec)));
        break;
    }


    char endfile[] = {0,0,0,0}; 
    out = fopen("images/out.bmp", "wa");
    fwrite(header, sizeof(char), hsize, out);
    fwrite(dst, sizeof(char), isize, out);
    fwrite(endfile, sizeof(char), 4, out);

    fclose(in);
    fclose(out);
}

void* filter_t(void* param)  {
	thread_data* tdata = (thread_data *) param;
	filter(tdata->src, tdata->dst, tdata->w, tdata->h, tdata->filtertype, tdata->section);
	pthread_exit(0);
}

void filter(int* src, int* dst, int w, int h, int filtertype, int section) {
	double filter[5][5];
	double factor, bias;
	int filterWidth, filterHeight;
	int startrow, endrow;
	
	filterWidth = 5;
	filterHeight = 5;
	double idfilter[5][5] = {{0,0,0,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0}};
	double edgefilter[5][5] = {{-1,0,0,0,0}, {0,-2,0,0,0},{0,0,6,0,0},{0,0,0,-2,0},{0,0,0,0,-1}};
    double gaussfilter[5][5] = {{1,4,6,4,1},{4,16,24,16,4},{6,24,36,24,6},{4,16,24,16,4},{1,4,6,4,1}};
    double embossfilter[5][5] = {{-1,-1,-1,-1,0},{-1,-1,-1,0,1},{-1,-1,0,1,1},{-1,0,1,1,1},{0,1,1,1,1}};

    if (filtertype == 1) {
        memcpy(filter, idfilter, sizeof(filter));
        factor = 1.0;
        bias = 0.0;
    } else if (filtertype == 2) {
    	memcpy(filter, edgefilter, sizeof(filter));
    	factor = 1.0;
    	bias = 0.0;
    } else if (filtertype == 3) {
    	memcpy(filter, gaussfilter, sizeof(filter));
    	factor = 1.0 / 256.0;
    	bias = 0.0;
    } else if (filtertype == 4) {
    	memcpy(filter, embossfilter, sizeof(filter));
    	factor = 1.0;
    	bias = 128;	
    } else {
    	memcpy(filter, idfilter, sizeof(filter));
		factor = 1.0;
		bias = 0.0;
    }

    if (section == 0) {
    	startrow = 0;
        endrow = h;
    } else if (section == 1) {
    	startrow = 0;
    	endrow = h - 2;
    } else {
    	startrow = 2;
    	endrow = h;
    }

    for (unsigned int x = 0; x < w; x++) {
        for (unsigned int y = startrow; y < endrow; y++) {
            double red = 0.0;
            double green = 0.0;
            double blue = 0.0;
            for (unsigned int filterY = 0; filterY < filterHeight; filterY++) {
                for (unsigned int filterX = 0; filterX < filterWidth; filterX++) {
                    unsigned int imageX = (x - 5 / 2 + filterX + w) % w;
                    unsigned int imageY = (y - 5 / 2 + filterY + h) % h;
                    unsigned int redPixel = (src[imageY * w + imageX] &   0x00FF0000) >> 16;
                    unsigned int greenPixel = (src[imageY * w + imageX] & 0x0000FF00) >> 8;
                    unsigned int bluePixel =  src[imageY * w + imageX] &  0x000000FF;
                    red += redPixel * filter[filterY][filterX];
                    green += greenPixel * filter[filterY][filterX];
                    blue += bluePixel * filter[filterY][filterX];
                }
            }
                                  //0xAARRGGBB
            unsigned int newAlpha = 0xFF000000;
            unsigned int newRed = std::min(std::max((int)(factor * red + bias), 0), 255) << 16;
            unsigned int newGreen = std::min(std::max((int)(factor * green + bias), 0), 255) << 8;
            unsigned int newBlue = std::min(std::max((int)(factor * blue + bias), 0), 255);
            dst[y * w + x] =  newAlpha + newRed + newGreen + newBlue;
        }
    }
}