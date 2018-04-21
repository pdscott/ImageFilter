typedef struct pixel {
	unsigned char green;
	unsigned char blue;
	unsigned char red;
	unsigned char alpha;
} Pixel;


__kernel void fpgafilter(__global Pixel* restrict in, 
	                     __global Pixel* restrict out,
	                     //__global double *filter,
	                     const int w, 
	                     const int h) { 


	int col = get_global_id(0); // x
	int row = get_global_id(1); // y

	double filter[5][5] = {{0,0,0,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0}};
	double red = 0.0;
	double green = 0.0;
	double blue = 0.0;
	double bias = 0.0;
	double factor = 1.0;
	for (unsigned int filterY = 0; filterY < 5; filterY++) {
		for (unsigned int filterX = 0; filterX < 5; filterX++) {
			unsigned int imageX = (col - 5 / 2 + filterX + w) % w;
			unsigned int imageY = (row - 5 / 2 + filterY + h) % h;
			red += in[imageY * w + imageX].red * filter[filterY][filterX];
			green += in[imageY * w + imageX].green * filter[filterY][filterX];
			blue += in[imageY * w + imageX].blue * filter[filterY][filterX];
		}
	}
	out[row * w + col].alpha = (unsigned char) 0xFF;
	out[row * w + col].red = (unsigned char) min(max((int)(factor * red + bias), 0), 255);
	out[row * w + col].green = (unsigned char) min(max((int)(factor * green + bias), 0), 255);
	out[row * w + col].blue = (unsigned char) min(max((int)(factor * blue + bias), 0), 255);
}
