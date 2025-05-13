# STM32 通用计时与延迟库 (TimerLib)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

这是一个为单片机设计的高精度通用计时与延迟库。它提供了纳秒级精度的时间测量和延时功能（需要定时器时钟支持），非常适合需要精确时间控制的应用场景。

## 特性

- **高精度时间戳**: 提供微秒级别的时间戳功能
- **精确延时功能**: 支持纳秒(ns)和微秒(μs)级别的精确延时
- **时间间隔测量**: 便捷测量代码执行时间或事件间隔
- **多种数据类型返回**: 支持整型、单精度和双精度浮点型返回值
- **性能优化**: 针对不同时钟频率提供优化计算路径
- **轻量级实现**: 无需额外外设，仅使用单个定时器

## 性能测试结果

以下是在STM32F103（72MHz）上的性能测试结果(LL库 GCC)：

| 优化级别 | 延时函数额外耗时 | 计时函数耗时 |
|---------|--------------|------------|
| -O0     | 3μs          | 7μs        |
| -O1     | 1μs          | 4μs        |
| -O2/-O3 | 约1μs         | 约4μs      |

## 注意事项

- 建议STM32F1系列使用该库时，延时设置大于5微秒
- 优化级别-O1/-O2/-O3性能表现相近
- 高优化级别可显著提升库性能

## 如何使用

### 初始化

使用库前需要先进行全局初始化和句柄初始化：

```c
// 全局初始化，设置自动重装载值和时钟频率
TimerLib_GlobalInit(65535, 72000000);  // 例如: ARR=65534, 时钟=72MHz

// 创建并初始化时间句柄
TimerLib_Handle htim;
TimerLib_InitHandle(&htim);
```

### 配置定时器中断

在定时器溢出中断处理函数中调用：

```c
void TIM1_UP_IRQHandler(void)
{
    // 清除更新中断标志
    if(LL_TIM_IsActiveFlag_UPDATE(TIM1))
    {
        LL_TIM_ClearFlag_UPDATE(TIM1);
        TimerLib_HandleUpdateIRQ();
    }
}
```

### 测量时间间隔

测量代码执行时间：

```c
// 重置计时点
TimerLib_InitHandle(&htim);

// 需要测量时间的代码
function_to_measure();

// 获取经过的时间
float seconds = TimerLib_GetInterval_sf(&htim);  // 以秒为单位
uint32_t microseconds = TimerLib_GetInterval_us(&htim);  // 以微秒为单位
```

### 精确延时

```c
// 延时1000微秒
TimerLib_DelayUS(1000);

// 延时100纳秒
TimerLib_DelayNS(100);
```

### 获取时间戳

```c
// 获取当前时间戳
uint64_t timestamp_us = TimerLib_GetTimestamp_us();  // 以微秒为单位
float timestamp_s = TimerLib_GetTimestamp_sf();      // 以秒为单位，单精度浮点
```

## API 参考

### 初始化函数

- `TimerLib_GlobalInit(uint32_t arr, uint32_t clk_freq)`: 初始化库全局参数
- `TimerLib_InitHandle(TimerLib_Handle *htim)`: 初始化时间句柄
- `TimerLib_HandleUpdateIRQ(void)`: 更新中断处理函数，在定时器溢出时调用

### 时间间隔测量函数

- `TimerLib_GetInterval_sf(TimerLib_Handle *htim)`: 获取时间间隔(秒)，单精度浮点返回
- `TimerLib_GetInterval_sd(TimerLib_Handle *htim)`: 获取时间间隔(秒)，双精度浮点返回
- `TimerLib_GetInterval_us(TimerLib_Handle *htim)`: 获取时间间隔(微秒)
- `TimerLib_GetInterval_ns(TimerLib_Handle *htim)`: 获取时间间隔(纳秒)

### 时间戳函数

- `TimerLib_GetTimestamp_us()`: 获取当前时间戳(微秒)
- `TimerLib_GetTimestamp_sf()`: 获取当前时间戳(秒)，单精度浮点
- `TimerLib_GetTimestamp_df()`: 获取当前时间戳(秒)，双精度浮点

### 延时函数

- `TimerLib_DelayNS(uint32_t ns)`: 纳秒级延时
- `TimerLib_DelayUS(uint32_t us)`: 微秒级延时
- `TimerLib_DelayUS_32Short(uint32_t us)`: 短时间微秒延时(性能优化版)

## 配置说明

在使用前，需要配置实际的定时器访问：

1. 默认使用TIM1定时器，**可以根据实际情况修改**`get_current_cnt()`函数中的定时器访问
2. 确保正确配置定时器的时钟频率和自动重装载值
3. 确保定时器更新中断被正确设置，并在中断服务函数中调用`TimerLib_HandleUpdateIRQ()`

### 优化路径配置

TimerLib实现了多个性能优化路径，正确配置可以显著提高精度和减少CPU开销：

1. **微秒优化路径**：当时钟频率能被1,000,000整除时开启
   - 例如：8MHz, 16MHz, 24MHz, 48MHz, 72MHz, 80MHz, 84MHz, 96MHz, 168MHz 等
   - 配置例：`TimerLib_GlobalInit(65535, 72000000);`

2. **纳秒优化路径**：当时钟频率能被1,000,000,000整除时开启(基本不可能达到)
   - 例如：10000MHz 等
   - 配置例：`TimerLib_GlobalInit(65535, 1000000000);`

3. **短延时优化**：以下条件满足时可使用`TimerLib_DelayUS_32Short`
   - 微秒优化路径已开启
   - 延时时间小于1000微秒
   - 每毫秒的溢出次数不超过1次

**强烈建议，定时器的时钟能被ARR整除，如时钟为72000000，则ARR设置为：**
- 72000-1 = 71999: 每次溢出正好是1ms
- 7200-1 = 7199: 每次溢出正好是100μs
- 720-1 = 719: 每次溢出正好是10μs
- 72-1 = 71: 每次溢出正好是1μs

这样设置的好处：
1. 溢出时间间隔固定，不会有小数误差导致的时间抖动
2. 时间计算更精确，特别是累积长时间计时时
3. 便于调试和理解定时器行为
4. 对于周期性事件处理更加准确

#### 优化配置示例

```c
// 对于STM32F1系列（72MHz）- 最常见配置
TimerLib_GlobalInit(65535, 72000000);  // 进入微秒优化路径，ARR=65535

// 对于STM32F4系列（84MHz）
TimerLib_GlobalInit(65535, 84000000);  // 进入微秒优化路径，ARR=65535

// 对于STM32H7系列（480MHz）
TimerLib_GlobalInit(65535, 480000000);  // 进入微秒优化路径，ARR=65535

// 对于低溢出次数的短延时优化配置
TimerLib_GlobalInit(65535, 8000000);   // 适合TimerLib_DelayUS_32Short函数
// 时钟8MHz，ARR=65535时每毫秒溢出次数为0.122，满足优化条件
```

#### 最佳ARR值选择

- **一般应用**：使用最大ARR值（例如65535）以减少溢出次数
- **高精度短延时**：对于使用`TimerLib_DelayUS_32Short`，确保`clk_freq / arr_value / 1000 <= 1`
- **内部计算使用的ARR值**：库内部使用`arr + 1`作为实际值，所以传入65535实际用65536

## 性能与限制

- 时间测量精度取决于定时器时钟频率
- 对于72MHz的STM32F103，可以实现约14ns的延时精度
- 微秒级延时可以达到更高的精度，适合大多数应用场景

## 许可证

本项目采用MIT许可证，详情请参阅LICENSE文件。

## 贡献

欢迎提交问题报告和改进建议。如果您想贡献代码，请先创建issue讨论您想改变的内容。 