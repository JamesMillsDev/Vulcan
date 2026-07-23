#pragma once

#include "Utility/Collections/TArray.h"

/** @brief A collection of useful mathematical operations and types. */
class Maths
{
public:
	/** @brief Value close to 0 (1.4e^-45) */
	static float const EPSILON;
	/** @brief Euler's number, ~2.718. */
	static float const E;
	/** @brief Standard PI constant. */
	static float const PI;
	/** @brief The standard Tau constant equal to 2PIr. */
	static float const TAU;
	/** @brief A floating point value equal to the highest possible float value. */
	static float const POSITIVE_INFINITY;
	/** @brief A floating point value equal to the lowest possible float value. */
	static float const NEGATIVE_INFINITY;

public:
	/**
	 * @brief A wrapper for the std::abs function.
	 * @see https://en.cppreference.com/cpp/numeric/math/sin
	 */
	static float Sin(float v);

	/**
	 * @brief A wrapper for the std::cos function.
	 * @see https://en.cppreference.com/cpp/numeric/math/cos
	 */
	static float Cos(float v);

	/**
	 * @brief A wrapper for the std::tan function.
	 * @see https://en.cppreference.com/cpp/numeric/math/tan
	 */
	static float Tan(float v);

	/**
	 * @brief A wrapper for the std::asin function.
	 * @see https://en.cppreference.com/cpp/numeric/math/asin
	 */
	static float ASin(float v);

	/**
	 * @brief A wrapper for the std::acos function.
	 * @see https://en.cppreference.com/cpp/numeric/math/acos
	 */
	static float ACos(float v);

	/**
	 * @brief A wrapper for the std::atan function.
	 * @see https://en.cppreference.com/cpp/numeric/math/atan
	 */
	static float ATan(float v);

	/**
	 * @brief A wrapper for the std::atan2 function.
	 * @see https://en.cppreference.com/cpp/numeric/math/atan2
	 */
	static float ATan2(float y, float x);

	/**
	 * @brief A wrapper for the std::sqrt function.
	 * @see https://en.cppreference.com/cpp/numeric/math/sqrt
	 */
	static float Sqrt(float v);

	/**
	 * @brief A wrapper for the std::abs function.
	 * @see https://en.cppreference.com/cpp/numeric/math/abs
	 */
	static float Abs(float v);

	/**
	 * @brief A wrapper for the std::abs function. (Integer variant)
	 * @see https://en.cppreference.com/cpp/numeric/math/abs
	 */
	static int Abs(int v);

	/**
	 * @brief A wrapper for the std::min function. (Integer variant)
	 * @see https://en.cppreference.com/cpp/numeric/algorithm/min
	 */
	static float Min(float a, float b);

	/**
	 * @brief A wrapper for the std::min function. (Integer variant)
	 * @see https://en.cppreference.com/cpp/numeric/algorithm/min
	 */
	static int Min(int a, int b);

	/**
	 * @brief A wrapper for the std::min function which takes a variable number of inputs.
	 * @param count The number of values in @c values.
	 * @param values The list of values being compared.
	 * @see https://en.cppreference.com/cpp/numeric/algorithm/min
	 */
	template<typename FIRST, typename... ARGS>
	requires(std::is_same_v<FIRST, ARGS> && ...)
	static FIRST Min(int count, ARGS... values);

	/** @brief A wrapper for the std::max function. */
	static float Max(float a, float b);

	/**
	 * @brief A wrapper for the std::max function. (Integer variant)
	 * @see https://en.cppreference.com/cpp/numeric/algorithm/max
	 */
	static int Max(int a, int b);

	/**
	 * @brief A wrapper for the std::max function which takes a variable number of inputs.
	 * @param count The number of values in @c values.
	 * @param values The list of values being compared.
	 * @see https://en.cppreference.com/cpp/numeric/algorithm/max
	 */
	template<typename FIRST, typename... ARGS>
	requires(std::is_same_v<FIRST, ARGS> && ...)
	static FIRST Max(int count, ARGS... values);

	/**
	 * @brief A wrapper for the std::pow function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/pow
	 */
	static float Pow(float v, float p);

	/**
	 * @brief A wrapper for the std::exp function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/exp
	 */
	static float Exp(float v);

	/**
	 * @brief A wrapper for the std::log function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/log
	 */
	static float Log(float v);

	/**
	 * @brief A wrapper for the std::log10 function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/log10
	 */
	static float Log10(float v);

	/**
	 * @brief A wrapper for the std::ceil function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/ceil
	 */
	static float Ceil(float v);

	/**
	 * @brief A wrapper for the std::ceil function, returning a truncated integer.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/ceil
	 */
	static int CeilToInt(float v);

	/**
	 * @brief A wrapper for the std::ceil function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/floor
	 */
	static float Floor(float v);

	/**
	 * @brief A wrapper for the std::floor function, returning a truncated integer.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/floor
	 */
	static int FloorToInt(float v);

	/**
	 * @brief A wrapper for the std::round function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/round
	 */
	static float Round(float v);

	/**
	 * @brief A wrapper for the std::round function, returning a truncated integer.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/round
	 */
	static int RoundToInt(float v);

	/**
	 * @brief Determines the sign of a value.
	 * @return Returns @c -1 if the value is below @c 0 @c +1 otherwise.
	 */
	static float Sign(float v);

	/**
	 * @brief Compares two floats to see if they are close enough to be considered equal.
	 * @param a The first value to compare.
	 * @param b The second value to compare
	 * @param e The leeway that the comparison uses.
	 * @return @c true if the values are nearly identical and within @c e, @c false otherwise.
	 */
	static bool Approx(float a, float b, float e = EPSILON);

	/**
	 * @brief Checks if @c v is as close to @c 0 as possible.
	 * @param v The value to compare.
	 * @param e The leeway that the comparison uses.
	 * @return @c true if the @c v is nearly at zero within @c e leeway, @c false otherwise.
	 */
	static bool IsNearZero(float v, float e = EPSILON);

	/**
	 * @brief A wrapper for the std::round function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/isnan
	 */
	static bool IsNaN(float v);

	/**
	 * @brief Checks if @c v is greater than or equal to @c min and less than or equal to @c max.
	 * @param v The value to check.
	 * @param min The lower bounds.
	 * @param max The upper bounds.
	 */
	static bool IsInRange(float v, float min, float max);

	/**
	 * @brief A wrapper for the std::fmod function.
	 * @see @ref https://en.cppreference.com/cpp/numeric/math/fmod
	 */
	static float FMod(float a, float b);

	/** @brief Ensures @c v is within the bounds of @c min and @c max. */
	static float Clamp(float v, float min, float max);

	/** @brief Ensures @c v is within the bounds of @c min and @c max. */
	static int Clamp(int v, int min, int max);

	/** @brief Ensures @c v is within the bounds of @c 0 and @c 1. */
	static float Clamp01(float v);

	/**
	 * @brief Linearly interpolates between @c a and @c b over the course of @c t.
	 * @param a The lower bounds of the lerp.
	 * @param b The upper bounds of the lerp.
	 * @param t The 'T' value. This value is clamped between @c 0 - @c 1.
	 */
	static float Lerp(float a, float b, float t);

	/**
	 * @brief Linearly interpolates an angle between @c a and @c b over the course of @c t.
	 * @param a The lower bounds of the lerp.
	 * @param b The upper bounds of the lerp.
	 * @param t The 'T' value. This value is clamped between @c 0 - @c 1.
	 */
	static float LerpAngle(float a, float b, float t);

	/**
	 * @brief Linearly interpolates between @c a and @c b over the course of @c t.
	 * @param a The lower bounds of the lerp.
	 * @param b The upper bounds of the lerp.
	 * @param t The 'T' value. This value is not clamped and can extend past @c b.
	 */
	static float LerpUnclamped(float a, float b, float t);

	/** @brief Converts the incoming @c deg value into radians. */
	static float Radians(float deg);

	/** @brief Converts the incoming @c rad value into degrees. */
	static float Degrees(float rad);

	/** @brief Smoothly moves a value @c t from @c from to @c to in a cubic form.  */
	static float SmoothStep(float from, float to, float t);

	/**
	 * @brief Returns a value between @c 0 and @c length based on the @c time. 
	 * When the value exceeds @c length, it moves it resets to @c 0.
	 * @param time The value to scale along.
	 * @param length The maximum value.
	 */
	static float Repeat(float time, float length);

	/**
	 * @brief Returns a value between @c 0 and @c length based on the @c time. 
	 * When the value exceeds @c length, it moves it in reverse.
	 * @param time The value to scale along.
	 * @param length The maximum value.
	 */
	static float PingPong(float time, float length);

	/** @brief Calculates the difference between two angles. */
	static float DeltaAngle(float current, float target);

};

template <typename FIRST, typename... ARGS>
requires(std::is_same_v<FIRST, ARGS> && ...)
FIRST Maths::Min(const int count, ARGS... values)
{
	float value = POSITIVE_INFINITY;
	TArray<FIRST, sizeof...(ARGS)> arr{ std::forward<ARGS>(values)... };

	for (int i = 0; i < count; ++i)
	{
		value = std::min(value, arr[i]);
	}

	return value;
}

template <typename FIRST, typename... ARGS>
requires(std::is_same_v<FIRST, ARGS> && ...)
FIRST Maths::Max(const int count, ARGS... values)
{
	float value = NEGATIVE_INFINITY;
	TArray<FIRST, sizeof...(ARGS)> arr{ std::forward<ARGS>(values)... };

	for (int i = 0; i < count; ++i)
	{
		value = std::max(value, arr[i]);
	}

	return value;
}
