IntegerMath (portable template wrapper)

Overview
- `IntegerMath` is a header-only templated wrapper around a FixedTrig-style trig implementation.
- It exposes integer-only trig and angle utilities (no floating point), suitable for small MCUs.

Default alias
- `using IM = IntegerMath<DefaultTrig, int16_t, 3600, 1800, 1000, 8192, 16384>;`
  - `DefaultTrig` is `FastTrigOptimized<128,128,128>` by default.

Template parameters
- `TrigT` : type implementing the trig API. Must provide static methods with these signatures:
  - `static uint16_t atan2(int16_t y, int16_t x);` (returns 0..FIXED_ANGLE_MAX-1)
  - `static int16_t sin(uint16_t angle);` (returns ±FIXED_SCALE)
  - `static int16_t cos(uint16_t angle);`
  - `static int32_t magnitude(int16_t x, int16_t y);`
  - `static uint32_t fast_sqrt(uint32_t x);`

- `AngleT` : integer type used for mower angles (default `int16_t`).
- `ANGLE_360` / `ANGLE_180` : angle wrap constants in `AngleT` units (defaults: 3600, 1800).
- `MOWER_SCALE` : output scale for sin/cos (default 1000).
- `FIXED_SCALE` : scale used by `TrigT` for sin/cos (default 8192).
- `FIXED_ANGLE_MAX` : angle wrap used by the trig implementation (default 16384).

How to use a different trig implementation
1. Provide or include a trig implementation type `MyTrig` that implements the required static methods above.
2. Instantiate the template where needed:

   using MyIM = IntegerMath<MyTrig, int16_t, 3600, 1800, 1000, 8192, 16384>;

3. Call methods on `MyIM`, e.g.: `MyIM::sin_int(angle)` or create thin free-function wrappers.

Smaller lookup tables / memory tuning
- To reduce flash usage, pick a `FastTrigOptimized` with a smaller table, e.g. `FastTrigOptimized<64,64,64>` and use it as `TrigT`.
- Example: `using SmallTrig = FastTrigOptimized<64,64,64>; using SmallIM = IntegerMath<SmallTrig>;`

Notes
- The header avoids runtime division and modulo in critical paths to be friendly to tiny MCUs without hardware divide.
- If you need different angle units or scales, change the template parameters accordingly.
