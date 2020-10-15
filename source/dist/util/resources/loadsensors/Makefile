CFLAGS = -g
# Remove either of these, as appropriate, and add -I for nvml.h and CL/cl.h
CPPFLAGS = -DHAVE_NVML=1 -DHAVE_OPENCL=1
# Add -L for -lOpenCL and -lnvidia-ml.  Put ATI path (if any) first to
# pick up that lOpenCL rather than the CUDA one.
LDFLAGS =
# Remove either, as appropriate.
LIBS = -lOpenCL -lnvidia-ml

.PHONY: clean

gpu-loadsensor: gpu-loadsensor.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o gpu-loadsensor gpu-loadsensor.c $(LDFLAGS)

clean:
	rm -f gpu-loadsensor *.o
