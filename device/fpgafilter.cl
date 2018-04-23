
// __kernel void fpgafilter(__global Pixel* restrict in, 
// 	                     __global Pixel* restrict out,
__kernel void fpgafilter(__global unsigned int* restrict in, 
 	                     __global unsigned int* restrict out,
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
            unsigned int redPixel = (src[imageY * w + imageX] &   0x00FF0000) >> 16;
            unsigned int greenPixel = (src[imageY * w + imageX] & 0x0000FF00) >> 8;
            unsigned int bluePixel =  src[imageY * w + imageX] &  0x000000FF;
			red += redPixel * filter[filterY][filterX];
			green += greenPixel * filter[filterY][filterX];
			blue += bluePixel * filter[filterY][filterX];
		}
	}
    unsigned int newAlpha = 0xFF000000;
    unsigned int newRed = std::min(std::max((int)(factor * red + bias), 0), 255) << 16;
    unsigned int newGreen = std::min(std::max((int)(factor * green + bias), 0), 255) << 8;
    unsigned int newBlue = std::min(std::max((int)(factor * blue + bias), 0), 255);
	out[row * w + col] =  newAlpha + newRed + newGreen + newBlue;
}
