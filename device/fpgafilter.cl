typedef struct pixel {
	unsigned char green;
	unsigned char blue;
	unsigned char red;
	unsigned char alpha;
} Pixel;



__kernel void fpgafilter(__global Pixel *in, 
	                     __global Pixel *out,
	                     __global double *filter,
	                     const int w, 
	                     const int h ) { 


	int col = get_global_id(0); // x
	int row = get_global_id(1); // y


	double red = 0.0;
	double green = 0.0;
	double blue = 0.0;
	for (unsigned int filterY = 0; filterY < 5; filterY++) {
		for (unsigned int filterX = 0; filterX < 5; filterX++) {
			unsigned int imageX = (col - 5 / 2 + filterX + w) % w;
			unsigned int imageY = (row - 5 / 2 + filterY + h) % h;
			Pixel *pixel = &in[imageY * w + imageX];
			red += pixel->red * filter[filterY][filterX];
			green += pixel->green * filter[filterY][filterX];
			blue += pixel->blue * filter[filterY][filterX];
		}
	}
	out[row * w + col].alpha = (unsigned char) 0xFF;
	out[row * w + col].red = (unsigned char) std::min(std::max(int(factor * red + bias), 0), 255);
	out[row * w + col].green = (unsigned char) std::min(std::max(int(factor * green + bias), 0), 255);
	out[row * w + col].blue = (unsigned char) std::min(std::max(int(factor * blue + bias), 0), 255);
}
