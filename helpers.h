#ifndef HELPERS_H
#define HELPERS_H

// static inline int printf0(const char *fmt, ...)
// {
//     return 0;
// }

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define printk(x, ...) printf(__VA_ARGS__)
#define console_log_level(x) (1)
#define CONFIG(x) (0)
#define mdelay(x) mp_hal_delay_us(x * 1000)
#define die(...)
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b)) /*!< Return the maximum of the 2 values     */
#endif
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b)) /*!< Return the minimum of the 2 values     */
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif
#define ALIGN(x, a) __ALIGN_MASK(x, (__typeof__(x))(a)-1UL)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN_UP(x, a) ALIGN((x), (a))
#define ALIGN_DOWN(x, a) ((x) & ~((__typeof__(x))(a)-1UL))
#define IS_ALIGNED(x, a) (((x) & ((__typeof__(x))(a)-1UL)) == 0)
#define DIV_ROUND_UP(x, y) ({                         \
    __typeof__(x) _div_local_x = (x);                 \
    __typeof__(y) _div_local_y = (y);                 \
    (_div_local_x + _div_local_y - 1) / _div_local_y; \
})

static inline int isupper(int ch)
{
    return (((ch >= 'A' && ch <= 'Z')) || (ch >= 0xC0 && ch <= 0xDD));
}

#endif /* HELPERS_H */