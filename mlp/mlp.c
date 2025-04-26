#ifdef FIRMWARE
#include "acia.h"
#include "clkcnt.h"
#include "flash.h"
#include "ili9341.h"
#include "spi.h"
#include "up5k_soc.h"
#else
#include <unistd.h>
#endif

#include <stdint.h>
#include <stdio.h>

#ifdef FIRMWARE
#include "printf.h"
#endif

// #define DEBUG

#include "mlp_params.h"

#ifdef FIRMWARE
extern uint8_t __heap_start;
uint8_t *const g_params = &__heap_start;
#else
uint8_t g_params[PARAMS_SIZE] = {};
#endif

#ifdef DEBUG
#ifdef FIRMWARE
extern uint8_t __stack_top;
extern uint8_t __stack_bottom;

#define STACK_LIMIT (24 * 1024) // 24KB

static inline void *get_sp(void)
{
    void *sp;
    __asm__ volatile("mv %0, sp" : "=r"(sp));
    return sp;
}

void stack_overflow_handler(void)
{
    printf("\n\n\n**********************\n\r");
    printf("\n\r");
    printf("    STACK OVERFLOW\n\r");
    printf("\n\r");
    printf("**********************\n\n\n\r");
    uint32_t cnt = 0;
    while (1)
    {
        gp_out = (gp_out & ~(7 << 17)) | ((cnt & 7) << 17);
        cnt++;
    }
}

#define CHECK_STACK_OVERFLOW()                         \
    do                                                 \
    {                                                  \
        uint8_t *sp = (uint8_t *)get_sp();             \
        if (sp < &__stack_bottom || sp > &__stack_top) \
        {                                              \
            stack_overflow_handler();                  \
        }                                              \
        if ((&__stack_top - sp) > STACK_LIMIT)         \
        {                                              \
            stack_overflow_handler();                  \
        }                                              \
    } while (0)
#endif

// -----------------------------------------------------------------------------
uint32_t crc32(uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xffffffff;

    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xEDB88320;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}

// -----------------------------------------------------------------------------
void print_buffer(const char *name, const int8_t *buf, uint32_t len)
{
    printf("%s:\n\r", name);
    uint32_t c = 0, l = 0;
    while (c < len)
    {
        printf("% 4d ", buf[c++]);
        if (++l == 10)
        {
            printf("\n\r");
            l = 0;
        }
    }
    if (l != 0)
    {
        printf("\n\r");
    }
}

#define PRINT_CRC(buf, len) \
    printf(#buf " (CRC32): %d\n\r", crc32((uint8_t *)buf, (len)))

#define PRINT_BUFFER(buf, size) \
    print_buffer(#buf, buf, size)

#define PRINT_BUFFER_SUMMARY(buf, size)                                \
    printf(#buf ": %d %d %d ... %d %d %d\n\r", buf[0], buf[1], buf[2], \
           buf[(size) - 3], buf[(size) - 2], buf[(size) - 1])

#endif

#ifndef CHECK_STACK_OVERFLOW
#define CHECK_STACK_OVERFLOW()
#endif

// -----------------------------------------------------------------------------
// LUT with approximate uint32 e^x for x=-128..127
static const uint32_t g_exp_lut_32[256] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000001,
    0x00000002,
    0x00000005,
    0x0000000D,
    0x00000023,
    0x0000005E,
    0x00000100,
    0x000002B8,
    0x00000764,
    0x00001416,
    0x00003699,
    0x0000946A,
    0x0001936E,
    0x000448A2,
    0x000BA4F5,
    0x001FA715,
    0x00560A77,
    0x00E9E224,
    0x027BC2CB,
    0x06C02D64,
    0x1259AC49,
    0x31E1995F,
    0x87975E85,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
};

// -----------------------------------------------------------------------------
// saturate a 32-bit integer to int8 range [INT8_MIN..INT8_MAX]
static inline int8_t saturate_to_int8(int32_t x)
{
    if (x > INT8_MAX)
    {
        return INT8_MAX;
    }
    else if (x < INT8_MIN)
    {
        return INT8_MIN;
    }
    else
    {
        return (int8_t)x;
    }
}

#if 0
// -----------------------------------------------------------------------------
// saturate a 64-bit integer to int32 range [INT32_MIN..INT32_MAX]
static inline int32_t saturate_to_int32(int64_t x)
{
    if (x > INT32_MAX)
    {
        return INT32_MAX;
    }
    else if (x < INT32_MIN)
    {
        return INT32_MIN;
    }
    else
    {
        return (int32_t)x;
    }
}

// -----------------------------------------------------------------------------
// inspired on tensorflow's
// MultiplyByQuantizedMultiplier()
// source: https://github.com/tensorflow/tensorflow/blob/99e7ad988852f2a95f47b80f93032a5ab6a6e595/tensorflow/core/kernels/uniform_quant_ops/math_utils.h#L47
static inline int32_t multiply_by_quantized_multiplier(int32_t val, uint32_t multiplier, int32_t shift)
{
    int64_t total_shift = 31 - shift;
    int64_t round = ((int64_t)1) << (total_shift - 1);
    int64_t result = val * ((int64_t)multiplier) + round;
    result = result >> total_shift;
    return saturate_to_int32(result);
}
#else
// -----------------------------------------------------------------------------
// saturating rounding doubling high mul: (a * b + 2^30) >> 31
static inline int32_t saturating_rounding_doubling_high_mul(int32_t a, int32_t b)
{
    // special case: INT32_MIN * INT32_MIN => saturates
    if (a == INT32_MIN && b == INT32_MIN)
    {
        return INT32_MAX;
    }

    // 32-bit only version of: (int64_t)a * (int64_t)b
    int32_t a_high = a >> 16;
    int32_t a_low = a & 0xffff;
    int32_t b_high = b >> 16;
    int32_t b_low = b & 0xffff;

    int32_t high_high = a_high * b_high;
    int32_t high_low = a_high * b_low;
    int32_t low_high = a_low * b_high;
    int32_t low_low = a_low * b_low;

    // 32x32 = 64 approximation: extract top 32 bits of result
    // combine partial products and simulate rounding
    int32_t mid = (low_high + high_low);
    int32_t mid_high = mid >> 15;
    int32_t result = high_high << 1;
    result += mid_high;
    result += (low_low >> 31); // rounding

    return result;
}

// -----------------------------------------------------------------------------
// rounding right shift by power-of-two
static inline int32_t rounding_divide_by_pot(int32_t x, int32_t exponent)
{
    int32_t mask = (1 << exponent) - 1;
    int32_t remainder = x & mask;
    int32_t threshold = (mask >> 1) + ((x < 0) ? 1 : 0);
    return (x >> exponent) + (remainder > threshold ? 1 : 0);
}

// -----------------------------------------------------------------------------
// multiply by quantized multiplier
static inline int32_t multiply_by_quantized_multiplier(int32_t val, uint32_t multiplier, int32_t shift)
{
    int32_t result = saturating_rounding_doubling_high_mul(val, (int32_t)multiplier);
    return rounding_divide_by_pot(result, shift);
}
#endif

// -----------------------------------------------------------------------------
// ReLU on int8
static inline int8_t relu_int8(int8_t x, int8_t zero_point)
{
    if (x < zero_point)
    {
        return zero_point;
    }
    else
    {
        return x;
    }
}

// -----------------------------------------------------------------------------
// dense (int8)
static void dense_int8(
    const int8_t *inputs,
    int8_t input_zp,
    const int8_t *weights,
    const int8_t *weight_zps,
    const int32_t *biases,
    int8_t *outputs,
    int8_t output_zp,
    const uint32_t *multipliers,
    const int32_t *shifts,
    uint32_t input_size,
    uint32_t output_size)
{
    for (uint32_t oc = 0; oc < output_size; ++oc)
    {
        // accumulate in 32-bit
        int32_t acc = 0;

        // weighted sum
        for (uint32_t ic = 0, wc = oc * input_size; ic < input_size; ++ic, ++wc)
        {
            int32_t x = (int32_t)inputs[ic] - (int32_t)input_zp;
            int32_t w = (int32_t)weights[wc] - (int32_t)weight_zps[oc];
            acc += x * w;
        }

        acc += biases[oc];

        acc = multiply_by_quantized_multiplier(acc, multipliers[oc], shifts[oc]);

        acc += (int32_t)output_zp;

        outputs[oc] = saturate_to_int8(acc);
    }
}

// -----------------------------------------------------------------------------
// dense + ReLU (int8)
static void dense_relu_int8(
    const int8_t *inputs,
    int8_t input_zp,
    const int8_t *weights,
    const int8_t *weight_zps,
    const int32_t *biases,
    int8_t *outputs,
    int8_t output_zp,
    const uint32_t *multipliers,
    const int32_t *shifts,
    uint32_t input_size,
    uint32_t output_size)
{
    dense_int8(
        inputs,
        input_zp,
        weights,
        weight_zps,
        biases,
        outputs,
        output_zp,
        multipliers,
        shifts,
        input_size,
        output_size);

    for (uint32_t oc = 0; oc < output_size; ++oc)
    {
        outputs[oc] = relu_int8(outputs[oc], output_zp);
    }
}

// -----------------------------------------------------------------------------
// in-place approximate softmax (int8)
//   1) find max logit
//   2) shift each logit by (logit[i] - max_logit)
//   3) convert to e^(shifted_value) using LUT
//   4) accumulate sum
//   5) out[i] = (127 * e^shifted_value) / sum (clamped to int8)
static void softmax_int8_inplace(int8_t *data, uint32_t length)
{
    // 1) find max
    int8_t max_val = data[0];
    for (uint32_t i = 1; i < length; ++i)
    {
        if (data[i] > max_val)
        {
            max_val = data[i];
        }
    }

    // 2) exponential transform
    // store intermediate e^x in a 32-bit
    // sum_exp in 32-bit as well
    // clamp partial sums to avoid overflow
    uint32_t exp_array[16]; // arbitrary max length
    uint32_t sum_exp = 0;

    for (uint32_t i = 0; i < length; ++i)
    {
        int32_t shift_val = (int32_t)data[i] - (int32_t)max_val; // range -255..255
        if (shift_val < -128)
        {
            shift_val = -128; // clamp
        }
        else if (shift_val > 127)
        {
            shift_val = 127; // clamp
        }

        // look up e^shift_val from the LUT [0..255]
        uint32_t e_val = g_exp_lut_32[shift_val + 128];

        exp_array[i] = e_val;

        // accumulte
        sum_exp += e_val;

        if (sum_exp < e_val)
        {
            // clamp sum_exp to max 32-bit
            sum_exp = UINT32_MAX;
        }
    }

    // 3) out[i] = (127 * exp_array[i]) / sum_exp
    for (uint32_t i = 0; i < length; ++i)
    {
        if (sum_exp == 0)
        {
            data[i] = 0;
        }
        else
        {
            uint32_t val = (exp_array[i] * 127) / sum_exp;
            if (val > 127)
            {
                val = 127;
            }
            data[i] = (int8_t)val;
        }
    }
}

// -----------------------------------------------------------------------------
// forward pass:
//   1) dense+ReLU
//   2) dense
//   3) softmax
static void forward_pass(const int8_t *inputs, int8_t *outputs)
{
    int8_t hiddens[HIDDEN_SIZE];

    const int8_t *hidden_weights = (int8_t *)&g_params[HIDDEN_WEIGHT_OFFSET];
    const int32_t *hidden_biases = (int32_t *)&g_params[HIDDEN_BIAS_OFFSET];
    const int8_t *output_weights = (int8_t *)&g_params[OUTPUT_WEIGHT_OFFSET];
    const int32_t *output_biases = (int32_t *)&g_params[OUTPUT_BIAS_OFFSET];
    int8_t input_zp = (int8_t)g_params[INPUT_ZP_OFFSET];
    const int8_t *hidden_weight_zps = (int8_t *)&g_params[HIDDEN_WEIGHT_ZP_OFFSET];
    int8_t hidden_zp = (int8_t)g_params[HIDDEN_ZP_OFFSET];
    const uint32_t *layer1_multipliers = (uint32_t *)&g_params[LAYER1_MULTIPLIER_OFFSET];
    const int32_t *layer1_scales = (int32_t *)&g_params[LAYER1_SCALE_OFFSET];
    const int8_t *output_weight_zps = (int8_t *)&g_params[OUTPUT_WEIGHT_ZP_OFFSET];
    int8_t output_zp = (int8_t)g_params[OUTPUT_ZP_OFFSET];
    const uint32_t *layer2_multipliers = (uint32_t *)&g_params[LAYER2_MULTIPLIER_OFFSET];
    const int32_t *layer2_scales = (int32_t *)&g_params[LAYER2_SCALE_OFFSET];

    // 1) dense+ReLU: input -> hidden
    dense_relu_int8(
        inputs,
        input_zp,
        hidden_weights,
        hidden_weight_zps,
        hidden_biases,
        hiddens,
        hidden_zp,
        layer1_multipliers,
        layer1_scales,
        INPUT_SIZE,
        HIDDEN_SIZE);

#ifdef DEBUG
    PRINT_BUFFER(hiddens, HIDDEN_SIZE);
    PRINT_CRC(hiddens, HIDDEN_SIZE);
#endif

    // 2) dense (no activation) for final logits
    dense_int8(
        hiddens,
        hidden_zp,
        output_weights,
        output_weight_zps,
        output_biases,
        outputs,
        output_zp,
        layer2_multipliers,
        layer2_scales,
        HIDDEN_SIZE,
        OUTPUT_SIZE);

    // 3) in-place quantized softmax
    softmax_int8_inplace(outputs, OUTPUT_SIZE);
}

// -----------------------------------------------------------------------------
// states
#define WAITING_PARAMS 0
#define READING_PARAMS 1
#define WAITING_INPUT 2
#define READING_INPUT 3
#define INFERRING 4

#ifdef FIRMWARE
#ifdef getc
#undef getc
#endif
#define getc(x) acia_getc()
#define sleep(x) clkcnt_delayms((x))
#else
#define getc(x) getc((x))
#define sleep(x) usleep((x) * 1000)
static char *s_filenames[10] = {
    "zero.jpg.input.bin",
    "one.jpg.input.bin",
    "two.jpg.input.bin",
    "three.jpg.input.bin",
    "four.jpg.input.bin",
    "five.jpg.input.bin",
    "six.jpg.input.bin",
    "seven.jpg.input.bin",
    "eight.jpg.input.bin",
    "nine.jpg.input.bin"};
#endif

// -----------------------------------------------------------------------------
void main(void)
{
    uint8_t state = WAITING_PARAMS;
    int8_t inputs[INPUT_SIZE];
    int8_t outputs[OUTPUT_SIZE];
    int rcv_byte = 0;
    uint8_t digit = 0;

#ifdef FIRMWARE
    init_printf(0, acia_printf_putc);
#endif

    printf("\n\n\r");
    printf("  __  __   _        _____ \n\r");
    printf(" |  \\/  | | |      |  __ \\  \n\r");
    printf(" | \\  / | | |      | |__) | \n\r");
    printf(" | |\\/| | | |      |  ___/  \n\r");
    printf(" | |  | | | |____  | |      \n\r");
    printf(" |_|  |_| |______| |_|      \n\r");
    printf("\n\r");

    printf("Initializing\n\r");

#ifdef FIRMWARE
    spi_init(SPI0);
    spi_init(SPI1);

    flash_init(SPI0);

    ili9341_init(SPI1);

    {
        uint8_t logo_data[LOGO_WIDTH * LOGO_HEIGHT / 8];
        flash_read(SPI0, logo_data, LOGO_FLASH_ADDR, sizeof(logo_data));
        ili9341_blit_binary(0, 0, LOGO_WIDTH, LOGO_HEIGHT, 8, logo_data);
    }
#endif

    printf("Waiting to read params\n\r");

    while (1)
    {
        CHECK_STACK_OVERFLOW();

        switch (state)
        {
        case WAITING_PARAMS:
        {
            rcv_byte = getc(stdin);
            if (rcv_byte != EOF)
            {
                printf("Reading params\n\r");
                state = READING_PARAMS;
            }
        }
        break;
        case READING_PARAMS:
        {
#ifdef FIRMWARE
            uint32_t rcv_cnt = 0;
            uint32_t addr = PARAMS_FLASH_ADDR;
            uint8_t batch_size = 128;
            printf("[%08d/%08d]", rcv_cnt, PARAMS_SIZE);
            while (rcv_cnt < PARAMS_SIZE)
            {
                if ((rcv_cnt + batch_size) > PARAMS_SIZE)
                {
                    batch_size = PARAMS_SIZE - rcv_cnt;
                }
                flash_read(SPI0, (uint8_t *)&g_params[rcv_cnt], addr, batch_size);
                rcv_cnt += batch_size;
                addr += batch_size;
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                printf("[%08d/%08d]", rcv_cnt, PARAMS_SIZE);
            }
#else
            FILE *params_fp = fopen("mlp.params.bin", "rb");

            uint32_t rcv_cnt = 0;
            uint8_t prt_cnt = 0;
            printf("[%08d/%08d]", rcv_cnt, PARAMS_SIZE);
            while (rcv_cnt < PARAMS_SIZE)
            {
                rcv_byte = getc(params_fp);
                if (rcv_byte != EOF)
                {
                    g_params[rcv_cnt++] = (uint8_t)rcv_byte;
                    if (++prt_cnt % 128 == 0)
                    {
                        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                        printf("[%08d/%08d]", rcv_cnt, PARAMS_SIZE);
                        prt_cnt = 0;
                    }
                }
            }
            if (prt_cnt > 0)
            {
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                printf("[%08d/%08d]", rcv_cnt, PARAMS_SIZE);
            }

            fclose(params_fp);
#endif
            printf("\n\rParams read!\n\r");

#ifdef DEBUG
            const int8_t *hidden_weights = (int8_t *)&g_params[HIDDEN_WEIGHT_OFFSET];
            const int32_t *hidden_biases = (int32_t *)&g_params[HIDDEN_BIAS_OFFSET];
            const int8_t *output_weights = (int8_t *)&g_params[OUTPUT_WEIGHT_OFFSET];
            const int32_t *output_biases = (int32_t *)&g_params[OUTPUT_BIAS_OFFSET];
            int8_t input_zp = (int8_t)g_params[INPUT_ZP_OFFSET];
            const int8_t *hidden_weight_zps = (int8_t *)&g_params[HIDDEN_WEIGHT_ZP_OFFSET];
            int8_t hidden_zp = (int8_t)g_params[HIDDEN_ZP_OFFSET];
            const uint32_t *layer1_multipliers = (uint32_t *)&g_params[LAYER1_MULTIPLIER_OFFSET];
            const int32_t *layer1_scales = (int32_t *)&g_params[LAYER1_SCALE_OFFSET];
            const int8_t *output_weight_zps = (int8_t *)&g_params[OUTPUT_WEIGHT_ZP_OFFSET];
            int8_t output_zp = (int8_t)g_params[OUTPUT_ZP_OFFSET];
            const uint32_t *layer2_multipliers = (uint32_t *)&g_params[LAYER2_MULTIPLIER_OFFSET];
            const int32_t *layer2_scales = (int32_t *)&g_params[LAYER2_SCALE_OFFSET];

            PRINT_CRC(hidden_weights, INPUT_SIZE * HIDDEN_SIZE);

            PRINT_CRC(hidden_biases, HIDDEN_SIZE);

            PRINT_BUFFER(output_weights, HIDDEN_SIZE * OUTPUT_SIZE);
            PRINT_CRC(output_weights, HIDDEN_SIZE * OUTPUT_SIZE);

            PRINT_CRC(output_biases, OUTPUT_SIZE);

            printf("input_zp: %d\n\r", input_zp);

            PRINT_CRC(hidden_weight_zps, HIDDEN_SIZE);

            printf("hidden_zp: %d\n\r", hidden_zp);

            PRINT_CRC(layer1_multipliers, HIDDEN_SIZE);

            PRINT_CRC(layer1_scales, HIDDEN_SIZE);

            PRINT_CRC(output_weight_zps, OUTPUT_SIZE);

            printf("output_zp: %d\n\r", output_zp);

            PRINT_CRC(layer2_multipliers, OUTPUT_SIZE);

            PRINT_CRC(layer2_scales, OUTPUT_SIZE);
#endif

            printf("Select digit [0-9]: ");
            state = WAITING_INPUT;
        }
        break;
        case WAITING_INPUT:
        {
            rcv_byte = getc(stdin);
            if (rcv_byte >= '0' && rcv_byte <= '9')
            {
                digit = (uint8_t)(rcv_byte - '0');
                printf("%d\n\r", digit);
                state = READING_INPUT;
            }
        }
        break;
        case READING_INPUT:
        {
#ifdef FIRMWARE
            uint32_t rcv_cnt = 0;
            uint32_t addr = INPUT_FLASH_ADDR + (digit * INPUT_SIZE);
            uint8_t batch_size = 128;
            printf("[%08d/%08d]", rcv_cnt, INPUT_SIZE);
            while (rcv_cnt < INPUT_SIZE)
            {
                if ((rcv_cnt + batch_size) > INPUT_SIZE)
                {
                    batch_size = INPUT_SIZE - rcv_cnt;
                }
                flash_read(SPI0, (uint8_t *)&inputs[rcv_cnt], addr, batch_size);
                rcv_cnt += batch_size;
                addr += batch_size;
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                printf("[%08d/%08d]", rcv_cnt, INPUT_SIZE);
            }
#else
            FILE *input_fp = fopen(s_filenames[digit], "rb");

            uint32_t rcv_cnt = 0;
            uint8_t prt_cnt = 0;
            printf("[%08d/%08d]", rcv_cnt, INPUT_SIZE);
            while (rcv_cnt < INPUT_SIZE)
            {
                rcv_byte = getc(input_fp);
                if (rcv_byte != EOF)
                {
                    inputs[rcv_cnt++] = (int8_t)rcv_byte;
                    if (++prt_cnt % 128 == 0)
                    {
                        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                        printf("[%08d/%08d]", rcv_cnt, INPUT_SIZE);
                        prt_cnt = 0;
                    }
                }
            }
            if (prt_cnt > 0)
            {
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                printf("[%08d/%08d]", rcv_cnt, INPUT_SIZE);
            }

            fclose(input_fp);
#endif
            printf("\n\rInput read!\n\r");
#ifdef DEBUG
            PRINT_CRC(inputs, INPUT_SIZE);
#endif
#ifdef FIRMWARE
            ili9341_fill_screen(ILI9341_BLACK);

            uint32_t offset = 0;
            for (uint32_t y = 0, p_y = (ILI9341_TFTHEIGHT - (INPUT_HEIGHT << 1)) / 2; y < INPUT_HEIGHT; ++y, p_y += 2)
            {
                for (uint32_t x = 0, p_x = (ILI9341_TFTWIDTH - (INPUT_WIDTH << 1)) / 2; x < INPUT_WIDTH; ++x, p_x += 2)
                {
                    uint8_t input = (uint8_t)(inputs[offset++] + 128);
                    uint16_t color = ili9342_color565(input, input, input);
                    for (uint32_t ky = 0; ky < 2; ++ky)
                    {
                        for (uint32_t kx = 0; kx < 2; ++kx)
                        {
                            ili9341_draw_pixel(p_x + kx, p_y + ky, color);
                        }
                    }
                }
            }

            clkcnt_delayms(1000);
#endif

            printf("Inferring\n\r");
            state = INFERRING;
        }
        break;
        case INFERRING:
        {
#ifdef FIRMWARE
            clkcnt_reg = 0;
#endif
            forward_pass(inputs, outputs);
#ifdef FIRMWARE
            uint32_t elapsed_cycles = clkcnt_reg;
            printf("Elapsed cycles: %d\n\r", elapsed_cycles);
#endif
            printf("******************************\n\r");
            for (uint32_t i = 0; i < OUTPUT_SIZE; ++i)
            {
                printf("Class %d => %d\n\r", i, outputs[i]);
            }
            printf("******************************\n\r");

            printf("Select digit [0-9]: ");
            state = WAITING_INPUT;
            break;
        }
        default:
            break;
        }
    }
}
