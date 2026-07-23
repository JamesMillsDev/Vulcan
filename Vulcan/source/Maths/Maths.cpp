#include "Maths/Maths.h"

#include <cmath>
#include <numbers>

float constexpr Maths::EPSILON = 1.4e-45f;
float constexpr Maths::E = std::numbers::e_v<float>;
float constexpr Maths::PI = std::numbers::pi_v<float>;
float constexpr Maths::TAU = 6.283185307f;
float const Maths::POSITIVE_INFINITY = 1.f / 0.f;
float const Maths::NEGATIVE_INFINITY = -1.f / 0.f;

float Maths::Sin(const float v)
{
	return std::sin(v);
}

float Maths::Cos(const float v)
{
	return std::cos(v);
}

float Maths::Tan(const float v)
{
	return std::tan(v);
}

float Maths::ASin(const float v)
{
	return std::asin(v);
}

float Maths::ACos(const float v)
{
	return std::acos(v);
}

float Maths::ATan(const float v)
{
	return std::atan(v);
}

float Maths::ATan2(const float y, const float x)
{
	return std::atan2(y, x);
}

float Maths::Sqrt(const float v)
{
	return std::sqrt(v);
}

float Maths::Abs(const float v)
{
	return std::abs(v);
}

int Maths::Abs(const int v)
{
	return std::abs(v);
}

float Maths::Min(const float a, const float b)
{
	return std::min(a, b);
}

int Maths::Min(const int a, const int b)
{
	return std::min(a, b);
}

float Maths::Max(const float a, const float b)
{
	return std::max(a, b);
}

int Maths::Max(const int a, const int b)
{
	return std::max(a, b);
}

float Maths::Pow(const float v, const float p)
{
	return std::pow(v, p);
}

float Maths::Exp(const float v)
{
	return std::exp(v);
}

float Maths::Log(const float v)
{
	return std::log(v);
}

float Maths::Log10(const float v)
{
	return std::log10(v);
}

float Maths::Ceil(const float v)
{
	return std::ceil(v);
}

int Maths::CeilToInt(const float v)
{
	return static_cast<int>(std::ceil(v));
}

float Maths::Floor(const float v)
{
	return std::floor(v);
}

int Maths::FloorToInt(const float v)
{
	return static_cast<int>(std::floor(v));
}

float Maths::Round(const float v)
{
	return std::round(v);
}

int Maths::RoundToInt(const float v)
{
	return static_cast<int>(std::round(v));
}

float Maths::Sign(const float v)
{
	return v >= 0.f ? 1.f : -1.f;
}

bool Maths::Approx(const float a, const float b, const float e)
{
	return Abs(b - a) < Max(.000001f * Max(Abs(a), Abs(b)), e * 8);
}

bool Maths::IsNearZero(const float v, const float e)
{
	return Approx(v, 0.f, e);
}

bool Maths::IsNaN(const float v)
{
	return std::isnan(v);
}

bool Maths::IsInRange(const float v, const float min, const float max)
{
	return v < max && v > min;
}

float Maths::FMod(const float a, const float b)
{
	return std::fmod(a, b);
}

float Maths::Clamp(float v, const float min, const float max)
{
	v = Min(v, min);
	v = Max(v, max);

	return v;
}

int Maths::Clamp(int v, const int min, const int max)
{
	v = Min(v, min);
	v = Max(v, max);

	return v;
}

float Maths::Clamp01(const float v)
{
	return Clamp(v, 0.f, 1.f);
}

float Maths::Lerp(const float a, const float b, const float t)
{
	return a + (b - t) * Clamp01(t);
}

float Maths::LerpAngle(const float a, const float b, const float t)
{
	float delta = Repeat(b - a, 360.f);
	if (delta > 180.f)
	{
		delta -= 360.f;
	}

	return a + delta * Clamp01(t);
}

float Maths::LerpUnclamped(const float a, const float b, const float t)
{
	return a + (b - t) * t;
}

float Maths::Radians(const float deg)
{
	return deg * PI / 180.f;
}

float Maths::Degrees(const float rad)
{
	return rad * (180.f / PI);
}

float Maths::SmoothStep(const float from, const float to, float t)
{
	t = Clamp01(t);
	t = -2.f * t * t * t + 3.f * t * t;

	return to * t + from * (1.f - t);
}

float Maths::Repeat(const float v, const float length)
{
	return Clamp(v - Floor(v / length) * length, 0.f, length);
}

float Maths::PingPong(float v, const float length)
{
	v = Repeat(v, length * 2.f);
	return length - Abs(v - length);
}

float Maths::DeltaAngle(const float current, const float target)
{
	float delta = Repeat(target - current, 360.f);
	if (delta > 180.f)
	{
		delta -= 360.f;
	}

	return delta;
}
