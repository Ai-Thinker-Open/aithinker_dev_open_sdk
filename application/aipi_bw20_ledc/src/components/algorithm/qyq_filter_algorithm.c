#define __QYQ_FILTER_ALGORITHM_C_
#include "qyq_filter_algorithm.h"

void qyq_filter_algorithm_limiting(int32_t input, int32_t *output, int32_t last_output, int32_t max_change)
{
    // 处理新的数据点，限制变化幅度
    int32_t diff = input - last_output;

    if (diff > max_change)
    {
        *output = last_output + max_change; // 上升超限
    }
    else if (diff < -max_change)
    {
        *output = last_output - max_change; // 下降超限
    }
    else
    {
        *output = input; // 在范围内，保持原值
    }
}

void qyq_filter_algorithm_median(int32_t *inputbuf, int32_t *output, uint32_t window_size)
{
    int32_t tmp_value = 0;

    for (uint32_t j = 0; j < (window_size - 1); j++)
    {
        for (uint32_t i = 0; i < (window_size - j - 1); i++)
        {
            if (inputbuf[i] > inputbuf[i + 1])
            {
                tmp_value = inputbuf[i];
                inputbuf[i] = inputbuf[i + 1];
                inputbuf[i + 1] = tmp_value;
            }
        }
    }

    *output = inputbuf[(window_size - 1) / 2];
}

void qyq_filter_algorithm_recursive_median(int32_t input, int32_t *output, int32_t *buf, int32_t *cushion_buf, uint32_t window_size)
{
    // 更新新数据
    for (uint32_t i = 0; i < window_size - 1; i++)
    {
        buf[i] = buf[i + 1];
    }
    buf[window_size - 1] = input;

    // 复制数据
    for (uint32_t i = 0; i < window_size; i++)
    {
        cushion_buf[i] = buf[i];
    }

    qyq_filter_algorithm_median(cushion_buf, output, window_size);
}

void qyq_filter_algorithm_average(int32_t *inputbuf, int32_t *output, uint32_t window_size)
{
    int32_t sum = 0;

    for (uint32_t i = 0; i < window_size; i++)
    {
        sum += inputbuf[i];
    }

    *output = sum / window_size;
}

void qyq_filter_algorithm_recursive_average(int32_t input, int32_t *output, int32_t *buf, uint32_t window_size)
{
    // 存储当前输入到缓冲区
    int32_t sum = 0;

    for (uint32_t i = 0; i < window_size - 1; i++)
    {
        buf[i] = buf[i + 1];
    }
    buf[window_size - 1] = input;

    // 计算窗口内有效数据的总和
    for (int32_t i = 0; i < window_size; i++)
    {
        sum += buf[i];
    }

    // 计算平均值并返回
    *output = sum / window_size;
}

void qyq_filter_algorithm_median_average(int32_t *inputbuf, int32_t *output, uint32_t window_size, uint8_t N)
{
    int32_t tmp_value = 0;
    int32_t sum = 0;

    for (uint32_t j = 0; j < (window_size - 1); j++)
    {
        for (uint32_t i = 0; i < (window_size - j - 1); i++)
        {
            if (inputbuf[i] > inputbuf[i + 1])
            {
                tmp_value = inputbuf[i];
                inputbuf[i] = inputbuf[i + 1];
                inputbuf[i + 1] = tmp_value;
            }
        }
    }

    for (uint32_t i = N; i < window_size - N; i++)
    {
        sum += inputbuf[i];
    }

    *output = sum / (window_size - N * 2);
}

void qyq_filter_algorithm_limiting_average(int32_t *inputbuf, int32_t *output, int32_t last_output, uint32_t window_size, int32_t max_change)
{
    // 对输入数据限幅处理
    for (uint32_t i = 0; i < window_size; i++)
    {
        qyq_filter_algorithm_limiting(inputbuf[i], &inputbuf[i], last_output, max_change);
    }

    // 平均滤波算法
    qyq_filter_algorithm_average(inputbuf, output, window_size);
}

void qyq_filter_algorithm_debouncing(int32_t input, int32_t *output, int32_t *buf, uint32_t window_size)
{
    // 更新缓冲区，移除最旧的值，并添加新的输入数据
    for (uint32_t i = 0; i < window_size - 1; i++)
    {
        buf[i] = buf[i + 1];
    }
    buf[window_size - 1] = input; // 更新最新输入数据

    // 计算稳定次数
    uint32_t stable_count = 0;
    int32_t last_value = buf[0]; // 当前输入值

    // 计算稳定时间
    for (uint32_t i = 0; i < window_size; i++)
    {
        if (buf[i] == last_value)
        {
            stable_count++;
        }
    }

    // 判断是否稳定
    if (stable_count >= window_size)
    {
        *output = last_value; // 输出稳定值
    }
    else
    {
        *output = -1; // 输出-1表示未稳定
    }
}

void qyq_filter_algorithm_weighted_recursive_average(int32_t input, int32_t *output, const int32_t *coe_list, int32_t *buf, uint32_t window_size)
{
    // 更新缓冲区，移除最旧的值，并添加新的输入数据
    for (uint32_t i = 0; i < window_size - 1; i++)
    {
        buf[i] = buf[i + 1];
    }
    buf[window_size - 1] = input; // 更新最新输入数据

    // 计算加权和及权重和
    int32_t sum = 0;
    int32_t coe_sum = 0;

    for (uint32_t i = 0; i < window_size; i++)
    {
        sum += buf[i] * coe_list[i];
        coe_sum += coe_list[i];
    }

    // 计算输出值
    if (coe_sum != 0)
    {
        *output = sum / coe_sum; // 避免除以零
    }
    else
    {
        *output = 0; // 默认输出
    }
}

void qyq_filter_algorithm_first_order_lag(int32_t input, int32_t *output, int32_t last_output, int32_t alpha)
{
    // 计算新的输出值
    *output = (int32_t)(alpha * input + (100 - alpha) * (last_output) / 100);
}

void qyq_filter_algorithm_maximum(int32_t *input, int32_t *output, uint32_t window_size)
{
    for (uint32_t i = 0; i < window_size; i++)
    {
        *output = input[i]; // 初始化最大值
        for (uint32_t j = i + 1; j < window_size; j++)
        {
            if (input[j] > *output)
            {
                *output = input[j]; // 更新最大值
            }
        }
    }
}

void qyq_filter_algorithm_minimum(int32_t *input, int32_t *output, uint32_t window_size)
{
    for (int i = 0; i < window_size; i++)
    {
        *output = input[i]; // 初始化最小值
        for (int j = i + 1; j < window_size; j++)
        {
            if (input[j] < *output)
            {
                output[i] = input[j]; // 更新最小值
            }
        }
    }
}

void qyq_filter_algorithm_low_pass(int32_t input, int32_t *output, int32_t last_output, int32_t alpha)
{
    *output = last_output + alpha * (input - last_output) / 100;
}

void qyq_filter_algorithm_kalman(int32_t measurement, int32_t *output, int32_t *state_estimate, int32_t *error_estimate, float process_noise, float measurement_noise)
{
    // 预测步骤
    *state_estimate = *state_estimate; // 预测当前状态
    *error_estimate += process_noise;  // 更新误差估计

    // 计算卡尔曼增益
    float kalman_gain = (float)(*error_estimate) / (*error_estimate + measurement_noise);

    // 更新步骤
    *state_estimate += kalman_gain * (measurement - *state_estimate); // 更新状态估计
    *error_estimate *= (1 - kalman_gain);                             // 更新误差估计

    // 返回滤波后的输出
    *output = *state_estimate;
}

// void qyq_filter_algorithm_fourier(int32_t *inputbuf, int32_t *outputbuf, float *freq_domain, float *filtered_freq, uint32_t size, float cutoff_freq, float sampling_rate)
// {
//     // 进行傅里叶变换（FFT）
//     for (uint32_t k = 0; k < size; k++)
//     {
//         freq_domain[k] = 0;
//         for (uint32_t n = 0; n < size; n++)
//         {
//             freq_domain[k] += inputbuf[n] * cexp(-2.0 * I * PI * k * n / size);
//         }
//     }

//     // 频域滤波
//     for (uint32_t k = 0; k < size; k++)
//     {
//         float frequency = k * sampling_rate / size;
//         if (frequency < cutoff_freq)
//         {
//             filtered_freq[k] = freq_domain[k]; // 保留低频信号
//         }
//         else
//         {
//             filtered_freq[k] = 0; // 去除高频信号
//         }
//     }

//     // 进行逆傅里叶变换（IFFT）
//     for (uint32_t n = 0; n < size; n++)
//     {
//         outputbuf[n] = 0;
//         for (uint32_t k = 0; k < size; k++)
//         {
//             outputbuf[n] += creal(filtered_freq[k] * cexp(2.0 * I * PI * k * n / size));
//         }
//         outputbuf[n] /= size; // 归一化
//     }
// }

void qyq_filter_algorithm_bayesian(
    int32_t measurement,
    int32_t prior_estimate,
    int32_t prior_variance,
    int32_t measurement_variance,
    int32_t *output)
{
    // 计算卡尔曼增益
    float kalman_gain = (float)prior_variance / (prior_variance + measurement_variance);

    // 更新状态估计
    *output = prior_estimate + (int32_t)(kalman_gain * (measurement - prior_estimate));

    // 更新方差
    prior_variance = (1 - kalman_gain) * prior_variance;
}

void qyq_filter_algorithm_high_pass(int32_t input, int32_t *output, int32_t last_output, int32_t alpha)
{
    // 高通滤波公式
    *output = (int32_t)(alpha * (last_output + input - last_output) / 100);
}

void qyq_filter_algorithm_band_pass(int32_t input, int32_t *output, int32_t last_output_low, int32_t last_output_high, int32_t alpha_low, int32_t alpha_high)
{
    // 低通部分
    int32_t low_pass_output = (int32_t)(alpha_low * (last_output_low + input - last_output_low) / 100);

    // 高通部分
    int32_t high_pass_output = (int32_t)(alpha_high * (last_output_high + input - last_output_high) / 100);

    // 带通滤波输出为低通与高通的差值
    *output = low_pass_output - high_pass_output;
}

void qyq_filter_algorithm_notch(int32_t input, int32_t *output, int32_t last_output, float notch_freq, float bandwidth)
{
    // 计算陷波因子
    float notch_factor = 1.0f - (bandwidth / (bandwidth + (notch_freq - input)));

    // 应用陷波滤波
    *output = (int32_t)(last_output * notch_factor + input * (1.0f - notch_factor));
}

void qyq_filter_algorithm_gaussian(int32_t *input_buf, int32_t *output, float *gaussian_kernel, uint32_t window_size, float sigma)
{
    // float sum = 0.0f;
    // float weight_sum = 0.0f;

    // int32_t half_window = window_size / 2;

    // // 计算高斯核
    // for (int32_t i = -half_window; i <= half_window; i++)
    // {
    //     gaussian_kernel[i + half_window] = exp(-(i * i) / (2 * sigma * sigma));
    //     weight_sum += gaussian_kernel[i + half_window];
    // }

    // // 归一化高斯核
    // for (int32_t i = 0; i < window_size; i++)
    // {
    //     gaussian_kernel[i] /= weight_sum;
    // }

    // // 应用高斯滤波
    // for (int32_t i = 0; i < window_size; i++)
    // {
    //     sum += input_buf[i] * gaussian_kernel[i];
    // }

    // *output = (int32_t)sum;
}
