typedef struct pixel {
	unsigned char green;
	unsigned char blue;
	unsigned char red;
	unsigned char alpha;
} Pixel;



__kernel void fpgafilter(__global Pixel *in, 
	                     __global Pixel *out,
	                     __global double *filter,
	                     const int l, 
	                     const int w ) { 


	int col = get_global_id(0); // x
	int row = get_global_id(1); // y


	double red = 0.0;
	double green = 0.0;
	double blue = 0.0;
	for (unsigned int filterY = 0; filterY < filterHeight; filterY++) {
		for (unsigned int filterX = 0; filterX < filterWidth; filterX++) {
			unsigned int imageX = (col - filterWidth / 2 + filterX + w) % w;
			unsigned int imageY = (row - filterHeight / 2 + filterY + h) % h;
			Pixel *pixel = &in[imageY * w + imageX];
			red += pixel->red * filter[filterY][filterX];
			green += pixel->green * filter[filterY][filterX];
			blue += pixel->blue * filter[filterY][filterX];
		}
	}
	out[y * w + x].alpha = (unsigned char) 0xFF;
	out[y * w + x].red = (unsigned char) std::min(std::max(int(factor * red + bias), 0), 255);
	out[y * w + x].green = (unsigned char) std::min(std::max(int(factor * green + bias), 0), 255);
	out[y * w + x].blue = (unsigned char) std::min(std::max(int(factor * blue + bias), 0), 255);
}
