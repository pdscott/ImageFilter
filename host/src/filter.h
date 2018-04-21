#ifndef filter_h
#define filter_h

typedef struct pixel {
	unsigned char green;
	unsigned char blue;
	unsigned char red;
	unsigned char alpha;
} Pixel;

typedef struct thread_data {
	Pixel* src;
	Pixel* dst; 
	int w; 
	int h;
	int filtertype;
	int section;
} thread_data;

void filter(Pixel* src, Pixel* dst, int w, int h, int filtertype, int section);
void* filter_t(void* param);






#endif