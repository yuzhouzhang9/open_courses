#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <thread>
// Use this code to time your threads
#include "CycleTimer.h"
#include "threadpool.h"
// Core computation of Mandelbrot set membershop
// Iterate complex number c to determine whether it diverges
static inline int mandel(float c_re, float c_im, int count)
{
    float z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < count; ++i)
    {

        if (z_re * z_re + z_im * z_im > 4.f)
            break;

        float new_re = z_re * z_re - z_im * z_im;
        float new_im = 2.f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }

    return i;
}

//
// MandelbrotSerial --
//
// Compute an image visualizing the mandelbrot set.  The resulting
// array contains the number of iterations required before the complex
// number corresponding to a pixel could be rejected from the set.
//
// * x0, y0, x1, y1 describe the complex coordinates mapping
//   into the image viewport.
// * width, height describe the size of the output image
// * startRow, endRow describe how much of the image to compute
void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int endRow,
    int maxIterations,
    int output[])
{
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    for (int j = startRow; j < endRow; j++)
    {
        for (int i = 0; i < width; ++i)
        {
            float x = x0 + i * dx;
            float y = y0 + j * dy;

            int index = (j * width + i);
            output[index] = mandel(x, y, maxIterations);
        }
    }
}

// Struct for passing arguments to thread routine
typedef struct
{
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int *output;
    int threadId;
    int numThreads;
} WorkerArgs;

void mandelbrotTask(WorkerArgs args)
{
    float dx = (args.x1 - args.x0) / args.width;
    float dy = (args.y1 - args.y0) / args.height;
    for (int i = 0 ; i < args.width; i++ )
    {
        float x = args.x0 + i * dx;
        float y = args.y0 + args.threadId * dy;
        int index = (args.threadId * args.width + i);
        args.output[index] = mandel(x, y, args.maxIterations);
    }
}

void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    // 创建线程池，线程数为指定的 numThreads
    ThreadPool pool(32);

    // 初始化每个任务的参数
    WorkerArgs args;
    args.x0 = x0;
    args.y0 = y0;
    args.x1 = x1;
    args.y1 = y1;
    args.width = width;
    args.height = height;
    args.maxIterations = maxIterations;
    args.output = output;
    args.numThreads = numThreads;

    // 将任务提交给线程池
    for (int i = 0; i < numThreads; ++i)
    {
        args.threadId = i;
        pool.enqueue([args]
                     { mandelbrotTask(args); });
    }
}
