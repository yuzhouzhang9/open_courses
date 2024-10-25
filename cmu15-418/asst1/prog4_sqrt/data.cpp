#include <algorithm>

// Generate random data
void initRandom(float *values, int N)
{
    for (int i = 0; i < N; i++)
    {
        // random input values
        values[i] = .001f + 2.998f * static_cast<float>(rand()) / RAND_MAX;
    }
}

// Generate data that gives high relative speedup
// 256 / 32
void initGood(float *values, int N)
{
    for (int i = 0; i < N; i++)
    {
        // Todo: Choose values
        // values[i] = 1.0f;
        // values[i] = .001f + 2.998f * static_cast<float>(rand()) / RAND_MAX;
        // generate data equte random input values
        // if(i&1)
            values[i] = .001f + 2.998f / 8 * static_cast<float>(rand()) / RAND_MAX + 2.998 / 8 * 7;
        // else
            // values[i] = .001f + 2.998f / 8 * static_cast<float>(rand()) / RAND_MAX + 2.998 / 8 ;
    }
}

// Generate data that gives low relative speedup
void initBad(float *values, int N)
{
    for (int i = 0; i < N; i++)
    {
        // Todo: Choose values
        // values[i] = 1.0f;
        // values[i] = .001f + 2.998f * static_cast<float>(rand()) / RAND_MAX;
        // generate data equte random input values
        if(i&1)
            values[i] = .001f + 2.998f / 8 * static_cast<float>(rand()) / RAND_MAX + 2.998 / 8 * 7;
        else
            values[i] = .001f + 2.998f / 8 * static_cast<float>(rand()) / RAND_MAX + 2.998 / 8 ;
    }
}
