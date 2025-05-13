/**
 * MIT License
 * 
 * Copyright (c) 2024 [C17Dev562]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file TimerLib.c
 * @brief 定时器库实现文件，提供高精度计时和延时功能
 * @note 使用时，请替换实际定时器访问
 */
#include "TimerLib.h"
#include "tim.h"

static uint32_t arr_value;                 // 自动重装载值
static uint32_t clock_freq;                // 定时器时钟频率
static volatile uint32_t overflow_counter; // 溢出计数器

/**
 * @brief 优化参数缓存结构体
 */
struct
{
    bool us_optimized;      // 微秒计算是否可优化
    uint32_t us_per_tick;   // 每个tick对应的微秒数
    bool ns_optimized;      // 纳秒计算是否可优化
    uint32_t ns_per_tick;   // 每个tick对应的纳秒数
    uint32_t overflowPreMS; // 每毫秒溢出次数,用于微秒短延时优化
} optim;

__attribute__((always_inline)) static inline uint32_t get_current_cnt(void)
{
    // NOTE: 用户需替换为实际定时器访问 ==============================
    return LL_TIM_GetCounter(TIM1);
}

void TimerLib_GlobalInit(uint32_t arr, uint32_t clk_freq)
{
    arr_value = arr;
    clock_freq = clk_freq;
    overflow_counter = 0;

    // 计算微秒计算是否可优化
    optim.us_optimized = (clk_freq % 1000000 == 0);
    // if (optim.us_optimized)  //强制计算
    // {
    // 计算每个tick对应的微秒数
    optim.us_per_tick = clk_freq / 1000000;
    // }

    // 计算纳秒计算是否可优化
    optim.ns_optimized = (clk_freq % 1000000000 == 0);

    // 计算每个tick对应的纳秒数
    if (optim.ns_optimized)
    {
        optim.ns_per_tick = clk_freq / 1000000000;
    }

    // 计算每毫秒溢出次数, 用于短延时优化
    optim.overflowPreMS = clk_freq / arr_value / 1000;
}

void TimerLib_InitHandle(TimerLib_Handle *htim)
{
    uint32_t cnt, ovf;

    // 原子读取当前计数值
    do
    {
        ovf = overflow_counter;
        cnt = get_current_cnt();
    } while (ovf != overflow_counter);

    // 初始化时间句柄
    htim->last_cnt = cnt;
    htim->last_overflow = ovf;
}

inline void TimerLib_HandleUpdateIRQ(void)
{
    overflow_counter++;
}

static inline uint32_t calculate_ticks(TimerLib_Handle *htim)
{
    register uint32_t current_cnt, current_ovf, delta_cnt, delta_ovf;

    // 原子读取当前值
    do
    {
        current_ovf = overflow_counter;
        current_cnt = get_current_cnt();
    } while (current_ovf != overflow_counter);

    // 计算计数器差值
    if (current_cnt >= htim->last_cnt)
    {
        delta_cnt = current_cnt - htim->last_cnt;
        delta_ovf = current_ovf - htim->last_overflow;
    }
    else
    {
        delta_cnt = (arr_value - htim->last_cnt) + current_cnt;
        // 计算溢出次数差值
        delta_ovf = current_ovf - htim->last_overflow - 1;
    }

    // 更新记录值
    htim->last_cnt = current_cnt;
    htim->last_overflow = current_ovf;

    return delta_ovf * (arr_value) + delta_cnt;
}

static inline uint64_t calculate_Timestamp()
{
    register uint32_t current_cnt;
    register uint32_t current_ovf;

    // 原子读取当前值
    do
    {
        current_ovf = overflow_counter;
        current_cnt = get_current_cnt();
    } while (current_ovf != overflow_counter);

    // 计算时间戳
    return (uint64_t)current_ovf * (arr_value) + current_cnt;
}

uint64_t TimerLib_GetTimestamp_us()
{
    uint64_t ticks = calculate_Timestamp();

    if (optim.us_optimized)
    {
        return ticks / optim.us_per_tick;
    }
    return (uint64_t)ticks * 1000000 / clock_freq;
}

float TimerLib_GetTimestamp_sf()
{
    uint64_t ticks = calculate_Timestamp();

    if (optim.us_optimized)
    {
        return ticks / optim.us_per_tick;
    }
    return ticks / clock_freq * 1e-6f;
}

double TimerLib_GetTimestamp_df()
{
    uint64_t ticks = calculate_Timestamp();

    if (optim.us_optimized)
    {
        return ticks / optim.us_per_tick * 1e-9;
    }
    return ticks / clock_freq * 1e-9;
}

uint32_t TimerLib_GetInterval_us(TimerLib_Handle *htim)
{
    uint32_t ticks = calculate_ticks(htim);

    if (optim.us_optimized)
    {
        return ticks / optim.us_per_tick;
    }
    return (uint64_t)ticks * 1000000 / clock_freq;
}

float TimerLib_GetInterval_sf(TimerLib_Handle *htim)
{
    uint32_t ticks = calculate_ticks(htim);

    if (optim.us_optimized)
    {
        return ticks / optim.us_per_tick * 1e-6f;
    }
    return ticks / clock_freq * 1e-6f;
}

double TimerLib_GetInterval_df(TimerLib_Handle *htim)
{
    uint32_t ticks = calculate_ticks(htim);

    if (optim.ns_optimized)
    {
        return ticks / optim.ns_per_tick * 1e-9;
    }
    return ticks / clock_freq * 1e-9;
}

uint32_t TimerLib_GetInterval_ns(TimerLib_Handle *htim)
{
    uint32_t ticks = calculate_ticks(htim);

    if (optim.ns_optimized)
    {
        return ticks / optim.ns_per_tick;
    }
    return (uint64_t)ticks * 1000000000 / clock_freq;
}

void TimerLib_DelayNS(uint32_t ns)
{
    uint32_t start_ovf, start_cnt;
    do
    {
        start_ovf = overflow_counter;
        start_cnt = get_current_cnt();
    } while (start_ovf != overflow_counter);

    // 在MCU上，基本看不到1G的定时器，所有直接计算所需的tick
    const uint64_t delay_ticks = (uint64_t)ns * clock_freq / 1000000000;

    uint32_t current_ovf, current_cnt;
    uint64_t elapsed;

    // 等待时间到达
    while (1)
    {
        // 原子读取当前值
        do
        {
            current_ovf = overflow_counter;
            current_cnt = get_current_cnt();
        } while (current_ovf != overflow_counter);

        // 计算经过的tick数
        if (current_ovf == start_ovf)
        {
            elapsed = current_cnt - start_cnt;
        }
        else
        {
            elapsed = (arr_value - start_cnt) +
                      (current_ovf - start_ovf - 1) * (arr_value) +
                      current_cnt;
        }

        if (elapsed >= delay_ticks)
            break;
    }
}

void TimerLib_DelayUS(uint32_t us)
{
    uint32_t start_ovf, start_cnt, current_ovf, current_cnt;
    uint64_t delay_ticks, elapsed;

    do
    {
        start_ovf = overflow_counter;
        start_cnt = get_current_cnt();
    } while (start_ovf != overflow_counter);

    // 优化路径计算
    if (optim.us_optimized)
    {
        delay_ticks = us * optim.us_per_tick;
    }
    else
    {
        delay_ticks = (uint64_t)us * clock_freq / 1000000;
    }

    while (1)
    {
        // 原子读取当前值
        do
        {
            current_ovf = overflow_counter;
            current_cnt = get_current_cnt();
        } while (current_ovf != overflow_counter);

        // 计算经过的tick数
        if (current_ovf == start_ovf)
        {
            elapsed = current_cnt - start_cnt;
        }
        else
        {
            // 计算溢出次数差值
            elapsed = (arr_value - start_cnt) +
                      (current_ovf - start_ovf - 1) * (arr_value) +
                      current_cnt;
        }

        if (elapsed >= delay_ticks)
            break;
    }
}

void TimerLib_DelayUS_32(uint32_t us)
{
    uint32_t start_ovf, start_cnt, current_ovf, current_cnt, elapsed, delay_ticks;
    do
    {
        start_ovf = overflow_counter;
        start_cnt = get_current_cnt();
    } while (start_ovf != overflow_counter);

    // 优化路径计算
    if (optim.us_optimized)
    {
        delay_ticks = us * optim.us_per_tick;
    }
    else
    {
        delay_ticks = us * clock_freq / 1000000;
    }

    while (1)
    {
        // 原子读取当前值
        do
        {
            current_ovf = overflow_counter;
            current_cnt = get_current_cnt();
        } while (current_ovf != overflow_counter);

        // 计算经过的tick数
        if (current_ovf == start_ovf)
        {
            elapsed = current_cnt - start_cnt;
        }
        else
        {
            elapsed = (arr_value - start_cnt) +
                      (current_ovf - start_ovf - 1) * (arr_value) +
                      current_cnt;
        }

        if (elapsed >= delay_ticks)
            break;
    }
}

int TimerLib_DelayUS_32Short(uint32_t us)
{
    uint32_t start_ovf, start_cnt, current_ovf, current_cnt, elapsed, delay_ticks;

    do
    {
        start_ovf = overflow_counter;
        start_cnt = get_current_cnt();
    } while (start_ovf != overflow_counter);

    // 优化路径计算
    if (!(optim.us_optimized && us < 1000 && optim.overflowPreMS <= 1))
    {
        return -1;
    }

    delay_ticks = us * optim.us_per_tick;

    while (1)
    {
        // 原子读取当前值
        do
        {
            current_ovf = overflow_counter;
            current_cnt = get_current_cnt();
        } while (current_ovf != overflow_counter);

        // 计算经过的tick数
        if (current_ovf > start_ovf)
        {
            elapsed = (arr_value - start_cnt) + current_cnt;
        }
        else
        {
            elapsed = current_cnt - start_cnt;
        }

        if (elapsed >= delay_ticks)
            break;
    }
    return 0;
}
