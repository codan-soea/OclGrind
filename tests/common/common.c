#include "common.h"

#include <stdio.h>
#include <string.h>

void checkError(cl_int err, const char *operation)
{
  if (err != CL_SUCCESS)
  {
    fprintf(stderr, "Error during operation '%s': %d\n", operation, err);
    exit(1);
  }
}

Context createContext(const char *source, const char *options, const char *binary)
{
  Context cl;
  cl_int err;
  cl_int status;

  err = clGetPlatformIDs(1, &cl.platform, NULL);
  checkError(err, "getting platform");

  // Check platform is Oclgrind
  char name[256];
  err = clGetPlatformInfo(cl.platform, CL_PLATFORM_NAME, 256, name, NULL);
  checkError(err, "getting platform name");
  if (strcmp(name, "Oclgrind"))
  {
    fprintf(stderr, "Unable to find Oclgrind platform\n");
    exit(1);
  }

  err = clGetDeviceIDs(cl.platform, CL_DEVICE_TYPE_ALL, 1, &cl.device, NULL);
  checkError(err, "getting device");

  cl.context = clCreateContext(NULL, 1, &cl.device, NULL, NULL, &err);
  checkError(err, "creating context");

  cl.queue = clCreateCommandQueue(cl.context, cl.device, 0, &err);
  checkError(err, "creating command queue");

  if (source)
  {
    cl.program = clCreateProgramWithSource(cl.context, 1, &source, NULL, &err);
    checkError(err, "creating program");
  }

  if (binary)
  {
    cl.program = clCreateProgramWithBinary(cl.context, 1, &cl.device, NULL, 
                                           &binary, &status, &err);
    checkError(err, "creating program");
  }

  err = clBuildProgram(cl.program, 1, &cl.device, options, NULL, NULL);
  if (err == CL_BUILD_PROGRAM_FAILURE)
  {
    size_t sz;
    clGetProgramBuildInfo(cl.program, cl.device, CL_PROGRAM_BUILD_LOG,
                          sizeof(size_t), NULL, &sz);
    char *buildLog = malloc(++sz);
    clGetProgramBuildInfo(cl.program, cl.device, CL_PROGRAM_BUILD_LOG,
                          sz, buildLog, NULL);
    fprintf(stderr, "%s\n", buildLog);
  }
  checkError(err, "building program");

  return cl;
}

void releaseContext(Context cl)
{
  clReleaseProgram(cl.program);
  clReleaseCommandQueue(cl.queue);
  clReleaseContext(cl.context);
}
