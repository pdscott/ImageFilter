#include <stdio.h>
#include <stdlib.h>
#include "filter.h"
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <string>
#include <stdarg.h>
#include "CL/opencl.h"
#include "AOCL_Utils.h"

using namespace aocl_utils;


// OpenCL runtime configuration
cl_platform_id platform = NULL;
unsigned num_devices = 0;
scoped_array<cl_device_id> device;
cl_context context = NULL;
cl_command_queue queue = NULL;
cl_program program = NULL;
cl_kernel kernel = NULL;
cl_mem buffer_in, buffer_out, buffer_f;


bool init_opencl();
void cl_run(char* src, char* dst, int w, int h, int filtertype);
void cleanup();

int fpga_filter(char* src, char* dst, int w, int h, int filtertype) {
    init_opencl();
    cl_run(src, dst, w, h, filtertype);
    cleanup();
    return 0;
}

// Initializes the OpenCL objects.
bool init_opencl() {
    int err;
    cl_int status;
    printf("Initializing OpenCL\n");
    if(!setCwdToExeDir()) {
        return false;
    }

    platform = findPlatform("Altera");
    if(platform == NULL) {
        printf("ERROR: Unable to find Altera OpenCL platform.\n");
        return false;
    }

    device.reset(getDevices(platform, CL_DEVICE_TYPE_ALL, &num_devices));
    printf("Platform: %s\n", getPlatformName(platform).c_str());
    printf("Using %d device(s)\n", num_devices);
    for(unsigned i = 0; i < num_devices; ++i) {
        printf("  %s\n", getDeviceName(device[i]).c_str());
    }

    context = clCreateContext(NULL, num_devices, device, NULL, NULL, &status);
    checkError(status, "Failed to create context");

    std::string binary_file = getBoardBinaryFile("fpgafilter", device[0]);
    printf("Using AOCX: %s\n", binary_file.c_str());
    program = createProgramFromBinary(context, binary_file.c_str(), device, num_devices);

    status = clBuildProgram(program, 0, NULL, "", NULL, NULL);
    checkError(status, "Failed to build program");

    queue = clCreateCommandQueue(context, device[0], CL_QUEUE_PROFILING_ENABLE, &status);
    checkError(status, "Failed to create command queue");

    const char *kernel_name = "fpgafilter";
    kernel = clCreateKernel(program, kernel_name, &status);
    checkError(status, "Failed to create kernel");

    return true;
}

void cl_run(char* src, char* dst, int w, int h, int filtertype) {
    double filter[5][5];
	double factor, bias;
	int filterWidth, filterHeight;
    cl_int status;
    cl_event kernel_event;
    cl_event finish_event;
    cl_event write_event[1];

	// filterWidth = 5;
	// filterHeight = 5;
	// double idfilter[5][5] = {{0,0,0,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0}};
	// double edgefilter[5][5] = {{-1,0,0,0,0}, {0,-2,0,0,0},{0,0,6,0,0},{0,0,0,-2,0},{0,0,0,0,-1}};
 //    double gaussfilter[5][5] = {{1,4,6,4,1},{4,16,24,16,4},{6,24,36,24,6},{4,16,24,16,4},{1,4,6,4,1}};
 //    double embossfilter[5][5] = {{-1,-1,-1,-1,0},{-1,-1,-1,0,1},{-1,-1,0,1,1},{-1,0,1,1,1},{0,1,1,1,1}};

 //    if (filtertype == 1) {
 //        memcpy(filter, idfilter, sizeof(filter));
 //        factor = 1.0;
 //        bias = 0.0;
 //    } else if (filtertype == 2) {
 //    	memcpy(filter, edgefilter, sizeof(filter));
 //    	factor = 1.0;
 //    	bias = 0.0;
 //    } else if (filtertype == 3) {
 //    	memcpy(filter, gaussfilter, sizeof(filter));
 //    	factor = 1.0 / 256.0;
 //    	bias = 0.0;
 //    } else if (filtertype == 4) {
 //    	memcpy(filter, embossfilter, sizeof(filter));
 //    	factor = 1.0;
 //    	bias = 128;	
 //    } else {
 //    	memcpy(filter, idfilter, sizeof(filter));
	// 	factor = 1.0;
	// 	bias = 0.0;
 //    }

    buffer_in = clCreateBuffer(context, CL_MEM_READ_WRITE, w*h*sizeof(int),
    NULL, &status);
    buffer_out = clCreateBuffer(context, CL_MEM_READ_WRITE, w*h*sizeof(int),
    NULL, &status);
    // buffer_f = clCreateBuffer(context, CL_MEM_READ_WRITE, 5*5*sizeof(double),
    // NULL, &status);

    status = clEnqueueWriteBuffer(queue, buffer_in, CL_FALSE,
      0, w*h*sizeof(int), src, 0, NULL, &write_event[0]);
    checkError(status, "Failed to transfer to buffer");

    // status = clEnqueueWriteBuffer(queue, buffer_f, CL_FALSE,
    //   0, 5*5*sizeof(double), filter, 0, NULL, &write_event[1]);
    // checkError(status, "Failed to transfer to buffer");

    // set kernel args
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&buffer_in);
    checkError(status, "Failed to set argument %d", 0);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&buffer_out);
    checkError(status, "Failed to set argument %d", 0);
    // status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&filter);
    // checkError(status, "Failed to set argument %d", 0);
    status = clSetKernelArg(kernel, 2, sizeof(w), (void*)&w);
    checkError(status, "Failed to set argument %d", 0);
    status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&h);
    checkError(status, "Failed to set argument %d", 0);

    // set work size and run kernel
	const size_t global_work_size[2] = {w,h};

	status = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 1, write_event, &kernel_event);
    checkError(status, "Failed to launch kernel");
    clFinish(queue);

    // copy the result back to dst
    status = clEnqueueReadBuffer(queue, buffer_out, CL_TRUE, 0, w*h*sizeof(int), dst, 0, NULL, NULL);
        checkError(status, "Failed to transfer data back to host");

    clReleaseEvent(write_event[0]);
}


void cleanup() {
    for(unsigned i = 0; i < num_devices; ++i) {
        if(kernel) {
            clReleaseKernel(kernel);
        }
        if(queue) {
            clReleaseCommandQueue(queue);
        }
    }

    if(program) {
        clReleaseProgram(program);
    }

    if(context) {
        clReleaseContext(context);
    }
}


