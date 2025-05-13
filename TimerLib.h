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

/* TimerLib.h */
#pragma once
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 定时器句柄结构体
 */
typedef struct {
    uint32_t last_cnt;      // 上次计数器值
    uint32_t last_overflow; // 上次溢出计数
} TimerLib_Handle;

/**
 * @brief 初始化定时器库全局参数
 * @param arr 自动重装载值
 * @param clk_freq 定时器时钟频率(Hz)
 */
void TimerLib_GlobalInit(uint32_t arr, uint32_t clk_freq);

/**
 * @brief 初始化时间句柄
 * @param htim 定时器句柄指针
 */
void TimerLib_InitHandle(TimerLib_Handle *htim);

/**
 * @brief 更新中断处理函数，在定时器溢出时调用
 */
void TimerLib_HandleUpdateIRQ(void);

/**
 * @brief 获取时间间隔(秒)，单精度浮点型返回
 * @param htim 定时器句柄指针
 * @return 自上次调用以来的时间间隔(秒)
 */
float TimerLib_GetInterval_sf(TimerLib_Handle *htim);

/**
 * @brief 获取时间间隔(秒)，双精度浮点型返回
 * @param htim 定时器句柄指针
 * @return 自上次调用以来的时间间隔(秒)
 */
double TimerLib_GetInterval_sd(TimerLib_Handle *htim);

/**
 * @brief 获取时间间隔(微秒)
 * @param htim 定时器句柄指针
 * @return 自上次调用以来的时间间隔(微秒)
 */
uint32_t TimerLib_GetInterval_us(TimerLib_Handle *htim);

/**
 * @brief 获取时间间隔(纳秒)
 * @param htim 定时器句柄指针
 * @return 自上次调用以来的时间间隔(纳秒)
 */
uint32_t TimerLib_GetInterval_ns(TimerLib_Handle *htim);

/**
 * @brief 获取当前时间戳(微秒)
 * @return 当前时间戳(微秒)
 */
uint64_t TimerLib_GetTimestamp_us();

/**
 * @brief 获取当前时间戳(秒)，单精度浮点型返回
 * @return 当前时间戳(秒)
 */
float TimerLib_GetTimestamp_sf();

/**
 * @brief 获取当前时间戳(秒)，双精度浮点型返回
 * @return 当前时间戳(秒)
 */
double TimerLib_GetTimestamp_df();

/**
 * @brief 纳秒级延时函数
 * @param ns 延时时间(纳秒)
 */
void TimerLib_DelayNS(uint32_t ns);

/**
 * @brief 微秒级延时函数
 * @param us 延时时间(微秒)
 */
void TimerLib_DelayUS(uint32_t us);

/**
 * @brief 微秒级短延时函数(仅适用于较短延时)
 * @param us 延时时间(微秒)
 * @return 0表示成功，-1表示参数不适合短延时
 */
int TimerLib_DelayUS_32Short(uint32_t us);
