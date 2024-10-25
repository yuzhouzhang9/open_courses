#include <stdio.h>
#include <algorithm>
#include <getopt.h>

#include "CycleTimer.h"
#include "mandelbrot_ispc.h"

extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);

extern void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations,
    int output[]);

extern void writePPMImage(
    int* data,
    int width, int height,
    const char *filename,
    int maxIterations);

bool verifyResult (int *gold, int *result, int width, int height) {
    int i, j;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            if (gold[i * width + j] != result[i * width + j]) {
                printf ("Mismatch : [%d][%d], Expected : %d, Actual : %d\n",
                            i, j, gold[i * width + j], result[i * width + j]);
                return 0;
            }
        }
    }

    return 1;
}

void
scaleAndShift(float& x0, float& x1, float& y0, float& y1,
              float scale,
              float shiftX, float shiftY)
{

    x0 *= scale;
    x1 *= scale;
    y0 *= scale;
    y1 *= scale;
    x0 += shiftX;
    x1 += shiftX;
    y0 += shiftY;
    y1 += shiftY;

}

using namespace ispc;

void usage(const char* progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Program Options:\n");
    printf("  -v  --view <INT>   Use specified view settings  (1-6)\n");
    printf("  -?  --help         This message\n");
}


#define VIEWCNT 6

int main(int argc, char** argv) {
    const unsigned int width = 135*100;
    const unsigned int height = 75*100;
    // const unsigned int width = 135*32;
    // const unsigned int height = 10000;
    const int maxIterations = 256;

    float x0 = -2.167;
    float x1 = 1.167;
    float y0 = -1;
    float y1 = 1;


    // Support VIEWCNT views
    float scaleValues[VIEWCNT+1] = { 1.0f, 1.0f, 0.015f, 0.02f, 0.02f, 0.02f,  0.002f };
    float shiftXs[VIEWCNT+1]      = { 0.0f, 0.0f, -0.98f, 0.35f, 0.0f, -1.5f,  -1.4f };
    float shiftYs[VIEWCNT+1]      = { 0.0f, 0.0f,  0.30f, 0.05f,  0.73f, 0.0f, 0.0f };


    // parse commandline options ////////////////////////////////////////////
    int opt;
    static struct option long_options[] = {
        {"view",  1, 0, 'v'},
        {"help",  0, 0, '?'},
        {0 ,0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "tv:?", long_options, NULL)) != EOF) {
        switch (opt) {
        case 'v':
        {
            int viewIndex = atoi(optarg);
            if (viewIndex >= 1 && viewIndex <= VIEWCNT)  {
                float scaleValue = scaleValues[viewIndex];
                float shiftX = shiftXs[viewIndex];
                float shiftY = shiftYs[viewIndex];
                scaleAndShift(x0, x1, y0, y1, scaleValue, shiftX, shiftY);
            } else {
                fprintf(stderr, "Invalid view index\n");
                return 1;
            }
            break;
        }
        case '?':
        default:
            usage(argv[0]);
            return 1;
        }
    }
    // end parsing of commandline options

    int *output_multithread = new int[width*height];
    int *output_ispc_tasks = new int[width*height];

    // init
    for (unsigned int i = 0; i < width * height; ++i)
        output_multithread[i] = 0;

    //
    // Run the multithread implementation. Teport the minimum time of three
    // runs for robust timing.
    //
    double minMultiThread= 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        mandelbrotThread(height,
        x0, y0, x1, y1, 
        width, height,
        maxIterations, output_multithread);
        double endTime = CycleTimer::currentSeconds();
        minMultiThread = std::min(minMultiThread, endTime - startTime);
    }

    printf("[mandelbrot multithread]:\t\t[%.3f] ms\n", minMultiThread * 1000);
    writePPMImage(output_multithread, width, height, "mandelbrot-multithread.ppm", maxIterations);

    double minTaskISPC = 1e30;
    for (int i = 0; i < 3; ++i) {
        double startTime = CycleTimer::currentSeconds();
        mandelbrot_ispc_withtasks(x0, y0, x1, y1, 
        width, height, 
        maxIterations, output_ispc_tasks,
        height);
        double endTime = CycleTimer::currentSeconds();
        minTaskISPC = std::min(minTaskISPC, endTime - startTime);
    }

    printf("[mandelbrot multicore ispc]:\t[%.3f] ms\n", minTaskISPC * 1000);
    writePPMImage(output_ispc_tasks, width, height, "mandelbrot-task-ispc.ppm", maxIterations);

    printf("\t\t\t\t(%.2fx speedup from task output_multithread)\n", minMultiThread/minTaskISPC);

    delete[] output_multithread;
    delete[] output_ispc_tasks;


    return 0;
}
