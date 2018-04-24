#ifndef filter_h
#define filter_h

typedef struct thread_data {
	int* src;
	int* dst;  
	int w; 
	int h;
	int filtertype;
	int section;
} thread_data;

void filter(int* src, int* dst, int w, int h, int filtertype, int section);
void* filter_t(void* param);
int fpga_filter(char* src, char* dst, int w, int h, int filtertype);






#endif