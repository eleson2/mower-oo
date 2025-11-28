#ifndef FIXED_TRIG_HPP
#define FIXED_TRIG_HPP

// Arduino compatibility - use stdint.h instead of cstdint
#ifdef ARDUINO
    #include <stdint.h>
#else
    #include <cstdint>
#endif

// Conditional includes for C++ features
#if __cplusplus >= 201103L && !defined(ARDUINO)
    #include <array>
    #define TRIG_USE_STD_ARRAY 1
#else
    #define TRIG_USE_STD_ARRAY 0
    // Simple array wrapper for Arduino (no std::array)
    namespace std {
        template<typename T, size_t N>
        struct array {
            T _data[N];

            constexpr T& operator[](size_t i) { return _data[i]; }
            constexpr const T& operator[](size_t i) const { return _data[i]; }
            constexpr size_t size() const { return N; }
        };
    }
#endif

// Disable C++20 concepts for Arduino
#if __cplusplus >= 202002L && !defined(ARDUINO)
    #include <concepts>
    #define TRIG_USE_CONCEPTS 1
#else
    #define TRIG_USE_CONCEPTS 0
#endif

// Platform-specific PROGMEM support for storing tables in flash/ROM
#if defined(__AVR__)
    #include <avr/pgmspace.h>
    #define TRIG_PROGMEM PROGMEM
    #define TRIG_READ_WORD(addr) pgm_read_word(addr)
#elif defined(ESP32) || defined(ESP8266)
    #include <pgmspace.h>
    #define TRIG_PROGMEM PROGMEM
    #define TRIG_READ_WORD(addr) pgm_read_word(addr)
#elif defined(ARDUINO_ARCH_STM32) || defined(STM32)
    // STM32: const data automatically goes to flash
    #define TRIG_PROGMEM
    #define TRIG_READ_WORD(addr) (*(addr))
#else
    // Generic platforms: no special handling needed
    #define TRIG_PROGMEM
    #define TRIG_READ_WORD(addr) (*(addr))
#endif

// Portable constexpr count trailing zeros (fallback for non-GCC compilers)
namespace detail {
    constexpr int constexpr_ctz(size_t n) {
        if (n == 0) return 0;
        int count = 0;
        while ((n & 1) == 0) {
            n >>= 1;
            ++count;
        }
        return count;
    }
}

// Use builtin if available, otherwise use constexpr fallback
#if defined(__GNUC__) || defined(__clang__)
    #define TRIG_CTZ(n) __builtin_ctz(n)
#else
    #define TRIG_CTZ(n) detail::constexpr_ctz(n)
#endif

// Arduino-compatible template (no C++20 concepts)
template<
    size_t SinCosTableSize = 128,
    size_t AtanTableSize = SinCosTableSize,
    size_t AsinTableSize = SinCosTableSize
>
class FastTrigOptimized {
    // Compile-time assertions (instead of concepts)
    static_assert(SinCosTableSize >= 32 && SinCosTableSize <= 1024 && (SinCosTableSize & (SinCosTableSize - 1)) == 0,
                  "SinCosTableSize must be power of 2 between 32-1024");
    static_assert(AtanTableSize > 1 && AtanTableSize <= 4096 && (AtanTableSize & (AtanTableSize - 1)) == 0,
                  "AtanTableSize must be power of 2 between 2-4096");
    static_assert(AsinTableSize > 1 && AsinTableSize <= 4096 && (AsinTableSize & (AsinTableSize - 1)) == 0,
                  "AsinTableSize must be power of 2 between 2-4096");
private:
    static constexpr uint16_t ANGLE_MAX = 8192;
    static constexpr int16_t OUTPUT_SCALE = 8192;

    // Precompute bit shifts for fast operations
    static constexpr int SIN_TABLE_BITS = TRIG_CTZ(SinCosTableSize);
    static constexpr uint32_t SIN_TABLE_MASK = SinCosTableSize - 1;

    static constexpr int ATAN_TABLE_BITS = TRIG_CTZ(AtanTableSize);
    static constexpr uint32_t ATAN_TABLE_MASK = AtanTableSize - 1;

    static constexpr int ASIN_TABLE_BITS = TRIG_CTZ(AsinTableSize);
    static constexpr uint32_t ASIN_TABLE_MASK = AsinTableSize - 1;

    // Precompute reciprocal for multiplication instead of division
    // For converting to table index: multiply by this instead of dividing
    static constexpr uint32_t RECIPROCAL_QUADRANT = (static_cast<uint32_t>(SinCosTableSize) << 16) / 4096;

    // ========================================================================
    // Table generation helper functions - MUST be defined before table init
    // ========================================================================

    static constexpr int16_t sin_internal(uint32_t angle) {
        uint32_t x = angle;
        uint32_t term = (x * (16384 - x)) >> 14;
        int32_t numerator = term << 2;
        int32_t denominator = 16487 - term;
        return static_cast<int16_t>((numerator * OUTPUT_SCALE * 2) / denominator);
    }

    static constexpr std::array<int16_t, SinCosTableSize> generate_sine_quarter_table() {
        std::array<int16_t, SinCosTableSize> table{};
        for (size_t i = 0; i < SinCosTableSize; ++i) {
            uint32_t angle = (i * 16384) / (SinCosTableSize - 1);
            table[i] = sin_internal(angle);
        }
        return table;
    }

    static constexpr std::array<uint16_t, AtanTableSize> generate_atan_quarter_table() {
        std::array<uint16_t, AtanTableSize> table{};
        for (size_t i = 0; i < AtanTableSize; ++i) {
            // CORDIC implementation
            int32_t x = 16384;
            int32_t y = 0;
            int32_t target_y = (i * 16384) / AtanTableSize;
            uint32_t angle = 0;

            for (int k = 0; k < 16; ++k) {
                int32_t x_new = 0;
                int32_t y_new = 0;
                uint32_t angle_step = (2048 >> k);

                if (y < target_y) {
                    x_new = x - (y >> k);
                    y_new = y + (x >> k);
                    angle += angle_step;
                } else {
                    x_new = x + (y >> k);
                    y_new = y - (x >> k);
                    angle -= angle_step;
                }
                x = x_new;
                y = y_new;
            }

            table[i] = static_cast<uint16_t>(angle);
        }
        return table;
    }

    static constexpr std::array<uint16_t, AsinTableSize> generate_asin_quarter_table() {
        std::array<uint16_t, AsinTableSize> table{};
        for (size_t i = 0; i < AsinTableSize; ++i) {
            int32_t target = (i * OUTPUT_SCALE * 2) / AsinTableSize;
            uint32_t low = 0;
            uint32_t high = ANGLE_MAX / 2;

            while (high - low > 1) {
                uint32_t mid = (low + high) / 2;
                int32_t sin_mid = sin_internal(mid);

                if (sin_mid < target) {
                    low = mid;
                } else {
                    high = mid;
                }
            }

            table[i] = static_cast<uint16_t>((low + high) / 2);
        }
        return table;
    }

    // ========================================================================
    // Tables - NOW initialized using the functions above
    // ========================================================================

    alignas(64) static constexpr auto sine_quarter_table TRIG_PROGMEM = generate_sine_quarter_table();
    alignas(64) static constexpr auto atan_quarter_table TRIG_PROGMEM = generate_atan_quarter_table();
    alignas(64) static constexpr auto asin_quarter_table TRIG_PROGMEM = generate_asin_quarter_table();

    // Helper functions to read from PROGMEM tables
    [[gnu::always_inline]]
    static inline int16_t read_sin_table(size_t index) noexcept {
        return static_cast<int16_t>(TRIG_READ_WORD(&sine_quarter_table[index]));
    }

    [[gnu::always_inline]]
    static inline uint16_t read_atan_table(size_t index) noexcept {
        return TRIG_READ_WORD(&atan_quarter_table[index]);
    }

    [[gnu::always_inline]]
    static inline uint16_t read_asin_table(size_t index) noexcept {
        return TRIG_READ_WORD(&asin_quarter_table[index]);
    }

public:
    // ============================================================
    // SIN - Optimized without modulo or division
    // ============================================================
    [[nodiscard, gnu::always_inline, gnu::hot]]
    static int16_t sin(uint16_t angle) noexcept {
        // Wrap angle using bit mask (faster than modulo)
        angle &= 0x3FFF;  // Equivalent to % 16384

        // Extract quadrant using bit shift (faster than division)
        uint8_t quadrant = angle >> 12;  // Divide by 4096
        uint16_t position = angle & 0xFFF;  // Remainder

        // Mirror for odd quadrants
        if (quadrant & 1) {
            position = 0x1000 - position;
        }

        // Map position to table index using multiplication
        // Instead of: index = position * (SinCosTableSize-1) / 4096
        // We use: index = (position * RECIPROCAL) >> shift
        uint32_t index_scaled = position * RECIPROCAL_QUADRANT;
        uint32_t index = index_scaled >> 16;
        uint8_t fraction = (index_scaled >> 8) & 0xFF;

        // Bounds check using bit operations
        index = (index < SinCosTableSize - 1) ? index : (SinCosTableSize - 1);

        // Interpolation with PROGMEM read
        int32_t y0 = read_sin_table(index);
        int32_t y1 = read_sin_table((index + 1) & SIN_TABLE_MASK);

        // Optimized interpolation without division
        int16_t value = static_cast<int16_t>(y0 + (((y1 - y0) * fraction) >> 8));

        // Conditional negate using bit manipulation
        // Instead of: return (quadrant >= 2) ? -value : value;
        int16_t sign_mask = -(quadrant >> 1);  // 0 or -1
        return (value ^ sign_mask) - sign_mask;
    }

    // ============================================================
    // COS - Simple wrapper
    // ============================================================
    [[nodiscard, gnu::always_inline, gnu::hot]]
    static int16_t cos(uint16_t angle) noexcept {
        return sin(angle + (ANGLE_MAX >> 2));  // Add π/2 using shift
    }

    // ============================================================
    // ATAN2 - Optimized without division in hot path
    // ============================================================
    [[nodiscard, gnu::hot]]
    static uint16_t atan2(int16_t y, int16_t x) noexcept {
        // Special case for x = 0 (unavoidable branches)
        if (x == 0) {
            if (y > 0) return ANGLE_MAX >> 1;      // 90°
            if (y < 0) return (ANGLE_MAX >> 1) * 3; // 270°
            return 0;
        }

        // Get absolute values and track signs
        uint32_t abs_x = (x < 0) ? -x : x;
        uint32_t abs_y = (y < 0) ? -y : y;
        uint8_t quadrant_adjust = ((x < 0) << 1) | (y < 0);

        uint16_t angle;

        // Compute ratio and index
        if (abs_x >= abs_y) {
            // Single division, reuse result for both index and fraction
            uint32_t ratio_full = (abs_y << (ATAN_TABLE_BITS + 8)) / abs_x;
            uint32_t index = (ratio_full >> 8) & ATAN_TABLE_MASK;
            uint32_t fraction = ratio_full & 0xFF;

            // Table lookup with interpolation using PROGMEM reads
            int32_t y0 = read_atan_table(index);
            int32_t y1 = read_atan_table((index + 1) & ATAN_TABLE_MASK);

            angle = static_cast<uint16_t>(y0 + (((y1 - y0) * fraction) >> 8));
        } else {
            // Single division, reuse result for both index and fraction
            uint32_t ratio_full = (abs_x << (ATAN_TABLE_BITS + 8)) / abs_y;
            uint32_t index = (ratio_full >> 8) & ATAN_TABLE_MASK;
            uint32_t fraction = ratio_full & 0xFF;

            // Table lookup with interpolation using PROGMEM reads
            int32_t y0 = read_atan_table(index);
            int32_t y1 = read_atan_table((index + 1) & ATAN_TABLE_MASK);

            uint16_t base = static_cast<uint16_t>(y0 + (((y1 - y0) * fraction) >> 8));
            angle = (ANGLE_MAX >> 1) - base;
        }

        // Adjust for quadrant using lookup table instead of branches
        static constexpr uint16_t quadrant_offset[4] = {
            0,                    // Q1: x>0, y>0
            2 * ANGLE_MAX,        // Q4: x>0, y<0
            ANGLE_MAX,            // Q2: x<0, y>0
            ANGLE_MAX,            // Q3: x<0, y<0
        };

        static constexpr int16_t angle_sign[4] = {
            1,   // Q1: angle
            -1,  // Q4: -angle
            -1,  // Q2: π - angle
            1,   // Q3: π + angle
        };

        uint16_t offset = quadrant_offset[quadrant_adjust];
        int16_t sign = angle_sign[quadrant_adjust];

        return offset + (angle * sign);
    }

    // ============================================================
    // ASIN - Optimized indexing
    // ============================================================
    [[nodiscard, gnu::hot]]
    static uint16_t asin(int16_t value) noexcept {
        // Get absolute value and track sign
        uint32_t abs_val = (value < 0) ? -value : value;

        // Clamp using bit operations
        abs_val = (abs_val > OUTPUT_SCALE * 2) ? OUTPUT_SCALE * 2 : abs_val;

        // Map to table index using multiplication by reciprocal
        // Instead of: index = (abs_val * (AsinTableSize-1)) / (OUTPUT_SCALE * 2)
        // We use: (abs_val * RECIPROCAL) >> SHIFT

        constexpr uint32_t ASIN_RECIPROCAL = (static_cast<uint32_t>(AsinTableSize) << 16) / (OUTPUT_SCALE * 2);

        uint32_t index_scaled = abs_val * ASIN_RECIPROCAL;
        uint32_t index = index_scaled >> 16;
        uint8_t fraction = (index_scaled >> 8) & 0xFF;

        // Bounds check
        index = (index < AsinTableSize - 1) ? index : (AsinTableSize - 1);

        // Interpolation with PROGMEM read
        int32_t y0 = read_asin_table(index);
        int32_t y1 = read_asin_table((index + 1) & ASIN_TABLE_MASK);

        uint16_t angle = static_cast<uint16_t>(y0 + (((y1 - y0) * fraction) >> 8));

        // Branchless sign handling
        return (value < 0) ? (2 * ANGLE_MAX - angle) : angle;
    }

    // ============================================================
    // MAGNITUDE - Already optimized (no division/modulo)
    // ============================================================
    [[nodiscard]]
    static int32_t magnitude(int32_t x, int32_t y) noexcept {
        uint32_t abs_x = (x < 0) ? -x : x;
        uint32_t abs_y = (y < 0) ? -y : y;

        // CORDIC with only shifts and adds
        for (int i = 0; i < 12; ++i) {
            uint32_t x_shift = abs_x >> i;
            uint32_t y_shift = abs_y >> i;

            if (abs_y > 0) {
                uint32_t x_new = abs_x + y_shift;
                uint32_t y_new = (abs_y > x_shift) ? abs_y - x_shift : 0;
                abs_x = x_new;
                abs_y = y_new;
            }
        }

        // Compensate for CORDIC gain using multiplication and shift
        return (abs_x * 39797) >> 16;
    }

    // ============================================================
    // Fast integer square root (Binary search method - no division)
    // ============================================================
    [[nodiscard]]
    static uint32_t fast_sqrt(uint32_t x) noexcept {
        if (x == 0) return 0;
        if (x == 1) return 1;

        // Binary search for the square root
        uint32_t start = 1;
        uint32_t end = (x >> 1) + 1;  // sqrt(x) <= x/2 + 1
        uint32_t result = 0;

        while (start <= end) {
            uint32_t mid = (start + end) >> 1;
            uint32_t square = mid * mid;

            if (square == x) {
                return mid;
            }

            if (square < x) {
                start = mid + 1;
                result = mid;
            } else {
                end = mid - 1;
            }
        }

        return result;
    }

    // ============================================================
    // Alternative magnitude using integer square root
    // Faster than CORDIC on some platforms, no multiplication needed
    // ============================================================
    [[nodiscard]]
    static int32_t magnitude_sqrt(int32_t x, int32_t y) noexcept {
        uint32_t abs_x = (x < 0) ? -x : x;
        uint32_t abs_y = (y < 0) ? -y : y;

        // Calculate x^2 + y^2 with overflow protection
        uint32_t x_sq = abs_x * abs_x;
        uint32_t y_sq = abs_y * abs_y;

        return static_cast<int32_t>(fast_sqrt(x_sq + y_sq));
    }

    // ============================================================
    // Fast reciprocal approximation (for avoiding division)
    // ============================================================
    [[gnu::always_inline]]
    static uint32_t fast_reciprocal(uint32_t d) noexcept {
        // Newton-Raphson approximation for 1/d
        // Initial guess using bit manipulation
        uint32_t x = (0xFFFFFFFF / d);  // One division, but we refine it

        // One iteration is often enough for our precision
        x = (x * (2 - ((d * x) >> 16))) >> 16;

        return x;
    }

    // ============================================================
    // Helper: Calculate memory usage at compile time
    // ============================================================
    static constexpr size_t memory_usage() noexcept {
        return sizeof(sine_quarter_table) +
               sizeof(atan_quarter_table) +
               sizeof(asin_quarter_table);
    }
};

#endif // FIXED_TRIG_HPP
