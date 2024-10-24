// 包含必要的头文件
#include <stdio.h>
#include <algorithm>
#include <getopt.h>

#include "CycleTimer.h"

// 声明外部函数
extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int endRow,
    int maxIterations,
    int output[]);

extern void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations,
    int output[]);

extern void writePPMImage(
    int *data,
    int width, int height,
    const char *filename,
    int maxIterations);

// 缩放和平移坐标函数
void scaleAndShift(float &x0, float &x1, float &y0, float &y1,
                   float scale,
                   float shiftX, float shiftY)
{
    // 缩放坐标
    x0 *= scale;
    x1 *= scale;
    y0 *= scale;
    y1 *= scale;
    // 平移坐标
    x0 += shiftX;
    x1 += shiftX;
    y0 += shiftY;
    y1 += shiftY;
}

// 打印程序使用帮助函数
void usage(const char *progname)
{
    printf("Usage: %s [options]\n", progname);
    printf("Program Options:\n");
    printf("  -t  --threads <N>  Use N threads\n");
    printf("  -v  --view <INT>   Use specified view settings (1-6)\n");
    printf("  -?  --help         This message\n");
}

// 验证结果函数
bool verifyResult(int *gold, int *result, int width, int height)
{
    int i, j;
    // 遍历结果数组
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            // 如果结果不匹配，打印错误信息并返回 false
            if (gold[i * width + j] != result[i * width + j])
            {
                printf("Mismatch : [%d][%d], Expected : %d, Actual : %d\n",
                       i, j, gold[i * width + j], result[i * width + j]);
                return 0;
            }
        }
    }
    // 如果结果匹配，返回 true
    return 1;
}

// 定义视图数量
#define VIEWCNT 6

int main(int argc, char **argv)
{

    // 设置图像宽度和高度
    const int width = 1440;
    const int height = 900;

    // 设置最大迭代次数
    const int maxIterations = 256;

    // 设置线程数量
    int numThreads = 32;

    // 设置初始坐标
    float x0 = -2.167;
    float x1 = 1.167;
    float y0 = -1;
    float y1 = 1;

    // 定义视图缩放和平移值
    float scaleValues[VIEWCNT + 1] = {1.0f, 1.0f, 0.015f, 0.02f, 0.02f, 0.02f, 0.002f};
    float shiftXs[VIEWCNT + 1] = {0.0f, 0.0f, -0.98f, 0.35f, 0.0f, -1.5f, -1.4f};
    float shiftYs[VIEWCNT + 1] = {0.0f, 0.0f, 0.30f, 0.05f, 0.73f, 0.0f, 0.0f};

    // 解析命令行选项
    int opt;
    static struct option long_options[] = {
        {"threads", 1, 0, 't'},
        {"view", 1, 0, 'v'},
        {"help", 0, 0, '?'},
        {0, 0, 0, 0}};
    int viewIndex = 1;
    while ((opt = getopt_long(argc, argv, "t:v:?", long_options, NULL)) != EOF)
    {
        switch (opt)
        {
        case 't':
            // 设置线程数量
            numThreads = atoi(optarg);
            break;
        case 'v':
            // 设置视图索引
            viewIndex = atoi(optarg);
            // 更改视图设置
            if (viewIndex >= 1 && viewIndex <= VIEWCNT)
            {
                float scaleValue = scaleValues[viewIndex];
                float shiftX = shiftXs[viewIndex];
                float shiftY = shiftYs[viewIndex];
                scaleAndShift(x0, x1, y0, y1, scaleValue, shiftX, shiftY);
            }
            else
            {
                fprintf(stderr, "Invalid view index\n");
                return 1;
            }
            break;
        case '?':
        default:
            // 打印使用帮助
            usage(argv[0]);
            return 1;
        }
    }

    // 分配内存
    int *output_serial = new int[width * height];
    int *output_thread = new int[width * height];

    // 运行串行实现
    memset(output_serial, 0, width * height * sizeof(int));
    double minSerial = 1e30;
    for (int i = 0; i < 3; ++i)
    {
        double startTime = CycleTimer::currentSeconds();
        mandelbrotSerial(x0, y0, x1, y1, width, height, 0, height, maxIterations, output_serial);
        double endTime = CycleTimer::currentSeconds();
        minSerial = std::min(minSerial, endTime - startTime);
    }

    // 打印串行实现时间
    printf("[mandelbrot serial]:\t\t[%.3f] ms\n", minSerial * 1000);
    // 保存图像
    writePPMImage(output_serial, width, height, "mandelbrot-serial.ppm", maxIterations);

    // 运行多线程实现
    memset(output_thread, 0, width * height * sizeof(int));

    double minThread = 1e30;
    for (int i = 0; i < 5; ++i)
    {
        double startTime = CycleTimer::currentSeconds();
        mandelbrotThread(numThreads, x0, y0, x1, y1, width, height, maxIterations, output_thread);
        double endTime = CycleTimer::currentSeconds();
        minThread = std::min(minThread, endTime - startTime);
    }

    // 打印多线程实现时间
    printf("[mandelbrot thread]:\t\t[%.3f] ms\n", minThread * 1000);
    // 保存图像
    writePPMImage(output_thread, width, height, "mandelbrot-thread.ppm", maxIterations);

    // 验证结果
    if (!verifyResult(output_serial, output_thread, width, height))
    {
        printf("ERROR : Output from threads does not match serial output\n");

        delete[] output_serial;
        delete[] output_thread;

        return 1;
    }

    // 计算加速比
    printf("++++\t\t\t\t(%.2fx speedup from %d threads)\n", minSerial / minThread, numThreads);

    // 释放内存
    delete[] output_serial;
    delete[] output_thread;

    return 0;
}
