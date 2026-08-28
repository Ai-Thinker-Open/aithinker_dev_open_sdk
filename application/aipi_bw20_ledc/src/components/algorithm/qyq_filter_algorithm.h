#ifndef __QYQ_FILTER_ALGORITHM_H_
#define __QYQ_FILTER_ALGORITHM_H_
#include "chip_types.h"
#include "stdarg.h"
#include <stdio.h>
#ifdef __QYQ_FILTER_ALGORITHM_C_
#define QYQ_FILTER_ALGORITHM_EXT
#else
#define QYQ_FILTER_ALGORITHM_EXT extern
#endif
#ifdef __cplusplus
extern "C" {
#endif
// 限幅滤波算法
/**
 * @brief 限制输入信号的变化幅度，防止急剧变化导致的噪声。
 * @param input 输入信号值。
 * @param output 指向输出信号值的指针。
 * @param last_output 上一个输出信号值。
 * @param max_change 最大允许变化幅度。
 * @note 应用场景：适用于传感器信号处理，特别是在高速变化的环境中，防止信号的突变影响后续计算。
 * @note 优点：能有效防止突发噪声，保持信号平稳。
 * @note 缺点：在信号变化较大时，可能导致滞后响应。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_limiting(int32_t input, int32_t *output, int32_t last_output, int32_t max_change);

// 中位值滤波算法
/**
 * @brief 通过中位值滤波消除输入信号中的突发噪声,滤掉异常值跟限幅滤波一样。
 * @param inputbuf 输入信号缓冲区。
 * @param output 指向输出信号值的指针。
 * @param window_size 滤波窗口大小。
 * @note 应用场景：适合用于去除传感器数据中的脉冲噪声，如温度传感器或压力传感器数据处理。
 * @note 优点：有效去除异常值，不会受到极端值的影响。
 * @note 缺点：计算量大，特别是在窗口较大时，延迟增加。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_median(int32_t *inputbuf, int32_t *output, uint32_t window_size);

// 递推中位值滤波算法
/**
 * @brief 通过递推方式计算中位值，平滑输入信号并减少噪声,滤掉异常值跟限幅滤波一样。
 * @param input 当前输入信号值。
 * @param output 指向输出信号值的指针。
 * @param buf 存储历史信号的缓冲区，用于计算中位值。
 * @param cushion_buf 缓冲buf。
 * @param window_size 滤波窗口大小。
 * @note 应用场景：适用于需要实时处理且对突发噪声敏感的信号，如视频监控、音频处理等。
 * @note 优点：能够有效去除尖峰噪声，不受极端值影响，同时保持信号的整体趋势。
 * @note 缺点：实现复杂，计算量较大，特别是在窗口较大时，延迟增加。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_recursive_median(int32_t input, int32_t *output, int32_t *buf, int32_t *cushion_buf, uint32_t window_size);

// 平均滤波算法
/**
 * @brief 通过计算输入信号的平均值来平滑信号。
 * @param inputbuf 输入信号缓冲区。
 * @param output 指向输出信号值的指针。
 * @param window_size 滤波窗口大小。
 * @note 应用场景：广泛应用于实时数据流处理，如音频信号处理和传感器信号平滑。
 * @note 优点：实现简单，计算效率高，适用于一般噪声。
 * @note 缺点：对尖峰噪声敏感，可能导致信号失真。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_average(int32_t *inputbuf, int32_t *output, uint32_t window_size);

// 递推平均滤波算法
/**
 * @brief 采用递推算法计算信号的平均值，以实现平滑效果。
 * @param input 输入信号值。
 * @param output 指向输出信号值的指针。
 * @param buf 存储历史信号的缓冲区。
 * @param window_size 滤波窗口大小。
 * @note 应用场景：适合用于需要快速响应的信号处理，如动态信号的实时监测。
 * @note 优点：计算效率高，适用于实时处理。
 * @note 缺点：对突发噪声不够鲁棒。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_recursive_average(int32_t input, int32_t *output, int32_t *buf, uint32_t window_size);

// 中位值平均滤波算法
/**
 * @brief 综合中位值与平均值滤波，平滑信号同时消除噪声。
 * @param inputbuf 输入信号缓冲区。
 * @param output 指向输出信号值的指针。
 * @param window_size 滤波窗口大小。
 * @param N 中位值滤波的窗口大小。
 * @note 应用场景：适用于需要同时消除尖峰噪声和波动的场景，如图像处理中的噪声去除。
 * @note 优点：综合了中位值和平均值的优点，抗噪声能力强。
 * @note 缺点：计算复杂度较高，延迟增加。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_median_average(int32_t *inputbuf, int32_t *output, uint32_t window_size, uint8_t N);

// 限幅平均滤波
/**
 * @brief 结合限幅和平均值滤波，防止异常值对信号的影响。
 * @param inputbuf 输入信号缓冲区。
 * @param output 指向输出信号值的指针。
 * @param last_output 上一个输出信号值。
 * @param window_size 滤波窗口大小。
 * @param max_change 最大允许变化幅度。
 * @note 应用场景：适用于需要在变化幅度受限的情况下对信号进行平滑处理，如电压或电流监测。
 * @note 优点：防止异常值干扰，适合不稳定信号。
 * @note 缺点：响应时间可能延迟，无法快速适应快速变化的信号。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_limiting_average(int32_t *inputbuf, int32_t *output, int32_t last_output, uint32_t window_size, int32_t max_change);

// 消抖滤波算法
/**
 * @brief 用于去除输入信号中的抖动，保持稳定的输出。
 * @param input 输入信号值。
 * @param output 指向输出信号值的指针。
 * @param buf 存储历史信号的缓冲区。
 * @param window_size 滤波窗口大小。
 * @note 应用场景：常用于按键、开关等设备的消抖处理，确保用户输入的稳定性。
 * @note 优点：显著提高输入信号的稳定性，减少误触发。
 * @note 缺点：可能导致信号响应延迟，影响用户体验。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_debouncing(int32_t input, int32_t *output, int32_t *buf, uint32_t window_size);

// 加权递推平均滤波算法
/**
 * @brief 根据权重进行递推平均，有效提升滤波效果。
 * @param input 输入信号值。
 * @param output 指向输出信号值的指针。
 * @param coe_list 权重列表。
 * @param buf 存储历史信号的缓冲区。
 * @param window_size 滤波窗口大小。
 * @note 应用场景：适用于需要重点关注最近数据的情况，如金融市场数据的平滑处理。
 * @note 优点：可灵活调整权重，提高对近期数据的响应能力。
 * @note 缺点：设计复杂性增加，需根据实际情况调节权重。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_weighted_recursive_average(int32_t input, int32_t *output, const int32_t *coe_list, int32_t *buf, uint32_t window_size);

// 一阶滞后滤波算法
/**
 * @brief 一阶滞后滤波，通过设置滞后因子平滑信号。
 * @param input 输入信号值。
 * @param output 指向输出信号值的指针。
 * @param last_output 上一个输出信号值。
 * @param alpha 滞后因子，范围在0到100之间。
 * @note 应用场景：用于控制系统和反馈环路中，提供平滑的控制信号。
 * @note 优点：实现简单，适合实时控制。
 * @note 缺点：可能导致系统响应迟钝，影响动态性能。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_first_order_lag(int32_t input, int32_t *output, int32_t last_output, int32_t alpha);

// 低通滤波器
/**
 * @brief 低通滤波器，过滤高频噪声，仅保留低频成分。
 * @param input 输入信号值。
 * @param output 指向输出信号值的指针。
 * @param last_output 上一个输出信号值。
 * @param alpha 滤波因子，范围在0到1之间。
 * @note 应用场景：广泛应用于音频处理、信号处理和传感器数据平滑。
 * @note 优点：有效消除高频噪声，保留低频信息。
 * @note 缺点：可能丢失高频信号中的重要信息，响应较慢。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_low_pass(int32_t input, int32_t *output, int32_t last_output, int32_t alpha);

// 最小值滤波器
/**
 * @brief 通过保留输入信号的最小值来平滑信号。
 * @param input 输入信号缓冲区。
 * @param output 指向输出信号值的指针。
 * @param window_size 滤波窗口大小。
 * @note 应用场景：适用于需要保留信号最小值的情况，如监测系统的异常检测。
 * @note 优点：可以有效抑制噪声，突出信号中的极小值特征。
 * @note 缺点：不适合处理大幅度变化的信号，可能导致信息丢失。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_minimum(int32_t *input, int32_t *output, uint32_t window_size);

// 最大值滤波器
/**
 * @brief 通过保留输入信号的最大值来平滑信号。
 * @param input 输入信号缓冲区。
 * @param output 指向输出信号值的指针。
 * @param window_size 滤波窗口大小。
 * @note 应用场景：用于需要保留信号最大值的情况，如监测系统的极端检测。
 * @note 优点：能够突出信号中的极大值特征，有效抑制噪声。
 * @note 缺点：对快速变化的信号响应慢，可能导致信号失真。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_maximum(int32_t *input, int32_t *output, uint32_t window_size);

// 卡尔曼滤波器
/**
 * @brief 实现卡尔曼滤波算法，通过线性动态系统对信号进行平滑和预测。
 * @param measurement 当前测量值。
 * @param output 指向输出信号值的指针。
 * @param state_estimate 当前状态估计。
 * @param error_estimate 当前误差估计。
 * @param process_noise 过程噪声协方差。
 * @param measurement_noise 测量噪声协方差。
 * @note 应用场景：广泛用于导航、跟踪系统、机器人、金融数据预测等领域，适合动态环境中的实时信号处理。
 * @note 优点：具有良好的预测性能，能够有效融合多源信息，适应动态变化。
 * @note 缺点：对模型假设敏感，要求系统具有线性特性，处理非线性问题时需扩展为扩展卡尔曼滤波（EKF）。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_kalman(int32_t measurement, int32_t *output, int32_t *state_estimate, int32_t *error_estimate, float process_noise, float measurement_noise);

// 傅里叶变换滤波
/**
 * @brief 使用快速傅里叶变换（FFT）对输入信号进行频域滤波。
 * @param inputbuf 输入信号缓冲区。
 * @param outputbuf 指向输出信号缓冲区的指针。
 * @param freq_domain 储存频域信号的缓冲区。
 * @param filtered_freq 储存滤波后的频域信号的缓冲区。
 * @param size 输入信号的大小（必须为2的幂）。
 * @param cutoff_freq 截止频率，低于此频率的信号将被保留。
 * @param sampling_rate 采样频率。
 * @note 应用场景：适用于去除信号中的高频噪声，如音频信号处理和图像去噪。
 * @note 优点：能够有效地在频域进行滤波，处理灵活。
 * @note 缺点：计算复杂度高，尤其是对大数据集，实时处理可能受到影响。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_fourier(int32_t *inputbuf, int32_t *outputbuf, float *freq_domain, float *filtered_freq, uint32_t size, float cutoff_freq, float sampling_rate);

// 贝叶斯滤波器
/**
 * @brief 实现简单的贝叶斯滤波器，用于估计信号的状态。
 * @param measurement 当前测量值。
 * @param prior_estimate 上一时刻的状态估计。
 * @param prior_variance 上一时刻的状态方差。
 * @param measurement_variance 测量噪声方差。
 * @param output 指向输出信号值的指针。
 * @note 应用场景：广泛用于传感器融合、定位系统和动态系统状态估计。
 * @note 优点：能够处理不确定性，适合动态变化环境下的信号估计。
 * @note 缺点：对先验分布的选择敏感，计算复杂度可能较高，特别是在高维情况下。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_bayesian(int32_t measurement, int32_t prior_estimate, int32_t prior_variance, int32_t measurement_variance, int32_t *output);

// 高通滤波器
/**
 * @brief 实现简单的高通滤波器，去除低频信号。
 * @param input 当前输入信号值。
 * @param output 指向输出信号值的指针。
 * @param last_output 上一时刻的输出值。
 * @param alpha 滤波器的平滑因子（0 < alpha < 100）。
 * @note 应用场景：适用于去除低频噪声，如音频处理和信号清理。
 * @note 优点：简单易实现，实时性强。
 * @note 缺点：可能引入相位延迟，处理高频信号时可能出现振铃效应。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_high_pass(int32_t input, int32_t *output, int32_t last_output, int32_t alpha);

// 带通滤波器
/**
 * @brief 实现简单的带通滤波器，保留特定频率范围内的信号。
 * @param input 当前输入信号值。
 * @param output 指向输出信号值的指针。
 * @param last_output 上一时刻的输出值。
 * @param alpha_low 低频端的平滑因子（0 < alpha_low < 100）。
 * @param alpha_high 高频端的平滑因子（0 < alpha_high < 100）。
 * @note 应用场景：用于音频处理、通信系统和信号分析，以提取特定频率范围内的信号。
 * @note 优点：能够有效去除低频和高频噪声，保留感兴趣的频率范围。
 * @note 缺点：设计复杂，可能需要调整参数以获得最佳性能。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_band_pass(int32_t input, int32_t *output, int32_t last_output_low, int32_t last_output_high, int32_t alpha_low, int32_t alpha_high);

// 陷波滤波器
/**
 * @brief 实现简单的陷波滤波器，用于去除特定频率的信号。
 * @param input 当前输入信号值。
 * @param output 指向输出信号值的指针。
 * @param last_output 上一时刻的输出值。
 * @param notch_freq 陷波频率（需要去除的频率）。
 * @param bandwidth 陷波带宽（去除范围的宽度）。
 * @note 应用场景：广泛用于音频处理、信号清理，特别是在去除电源干扰或其他特定频率干扰时。
 * @note 优点：能够精确去除特定频率的噪声。
 * @note 缺点：设计和实现相对复杂，可能会影响周围频率的信号。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_notch(int32_t input, int32_t *output, int32_t last_output, float notch_freq, float bandwidth);

// 高斯滤波器
/**
 * @brief 实现简单的一维高斯滤波器，用于平滑信号。
 * @param input_buf 输入信号值的缓冲区。
 * @param output 指向输出信号值的指针。
 * @param gaussian_kernel 指向高斯核的缓冲区。
 * @param window_size 窗口大小（滤波器长度）。
 * @param sigma 高斯函数的标准差。
 * @note 应用场景：图像处理、信号处理，常用于去除噪声和平滑数据。
 * @note 优点：能够有效平滑信号，保留主要特征。
 * @note 缺点：对高频信号的细节保留能力较弱，可能导致信号模糊。
 */
QYQ_FILTER_ALGORITHM_EXT void qyq_filter_algorithm_gaussian(int32_t *input_buf, int32_t *output, float *gaussian_kernel, uint32_t window_size, float sigma);
#ifdef __cplusplus
}
#endif
#endif
