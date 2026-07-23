#include "Maths/Color.h"

#include <cassert>
#include <cstring>
#include <format>
#include <unordered_map>

#include "Maths/Maths.h"

static std::unordered_map<char, uint8> asciiToHex =
{
	{ '0', static_cast<uint8>(0x00) },
	{ '1', static_cast<uint8>(0x01) },
	{ '2', static_cast<uint8>(0x02) },
	{ '3', static_cast<uint8>(0x03) },
	{ '4', static_cast<uint8>(0x04) },
	{ '5', static_cast<uint8>(0x05) },
	{ '6', static_cast<uint8>(0x06) },
	{ '7', static_cast<uint8>(0x07) },
	{ '8', static_cast<uint8>(0x08) },
	{ '9', static_cast<uint8>(0x09) },
	{ 'a', static_cast<uint8>(0x0A) },
	{ 'b', static_cast<uint8>(0x0B) },
	{ 'c', static_cast<uint8>(0x0C) },
	{ 'd', static_cast<uint8>(0x0D) },
	{ 'e', static_cast<uint8>(0x0E) },
	{ 'f', static_cast<uint8>(0x0F) }
};

const Color Color::BLACK = Color{ 0x000000FF };
const Color Color::WHITE = Color{ 0xFFFFFFFF };
const Color Color::BLANK = Color{ 0x00000000 };
const Color Color::RED = Color{ 0xFF0000FF };
const Color Color::GREEN = Color{ 0x00FF00FF };
const Color Color::BLUE = Color{ 0x0000FFFF };
const Color Color::YELLOW = Color{ 0xFFFF00FF };
const Color Color::BROWN = Color{ 0x964800FF };
const Color Color::ORANGE = Color{ 0xFF8000FF };
const Color Color::PINK = Color{ 0xFFC0CBFF };
const Color Color::PURPLE = Color{ 0x4B0082FF };
const Color Color::MAGENTA = Color{ 0xFF00FFFF };

Color Color::Lerp(const Color& a, const Color& b, const float t)
{
	return a + (b - a) * Maths::Clamp01(t);
}

Color Color::LerpUnclamped(const Color& a, const Color& b, const float t)
{
	return a + (b - a) * t;
}

Color Color::FromHex(const char* hex)
{
	// if the first value is a hash, offset the hex string by 1
	if (hex[0] == '#')
	{
		hex = &hex[1];
	}

	const size_t hexLen = strlen(hex);
	assert((hexLen == 6 || hexLen == 8) && "Hex must be 6 or 8 characters long");

	const uint8 r = asciiToHex[static_cast<char>(std::tolower(hex[0]))] |
		static_cast<uint8>(asciiToHex[static_cast<char>(std::tolower(hex[1]))] << 4);

	const uint8 g = asciiToHex[static_cast<char>(std::tolower(hex[2]))] |
		static_cast<uint8>(asciiToHex[static_cast<char>(std::tolower(hex[3]))] << 4);

	const uint8 b = asciiToHex[static_cast<char>(std::tolower(hex[4]))] |
		static_cast<uint8>(asciiToHex[static_cast<char>(std::tolower(hex[5]))] << 4);

	uint8 a = 255;
	if (hexLen == 8)
	{
		a = asciiToHex[static_cast<char>(std::tolower(hex[6]))] | 
			static_cast<uint8>(asciiToHex[static_cast<char>(std::tolower(hex[7]))] << 4);
	}

	// Return the constructed color
	return Color { r, g, b, a };
}

void Color::RGBToHSV(const Color& rgb, float& h, float& s, float& v)
{
	// Normalise each channel
	const float normR = rgb.r;
	const float normG = rgb.g;
	const float normB = rgb.b;

	// Find the minimum and maximum channel and difference between them
	const float cMin = Maths::Min<float>(3, normR, normG, normB);
	const float cMax = Maths::Max<float>(3, normR, normG, normB);
	const float cDiff = cMax - cMin;

	// Value is always the max
	v = cMax;

	// Saturation is 0 if the max is 0 to prevent division by 0
	s = Maths::IsNearZero(cMax) ? 0.f : cDiff / cMax;

	// If the max was the r channel
	if (Maths::Approx(cMax, normR))
	{
		h = Maths::FMod(60.f * ((normG - normB) / cDiff), 360.f);
	}
	
	// If the max was the green channel
	if (Maths::Approx(cMax, normG))
	{
		h = 60.f * ((normB - normR) / cDiff + 2.f);
	}
	
	// If the max was the red channel
	if (Maths::Approx(cMax, normB))
	{
		h = 60.f * ((normR - normG) / cDiff + 4.f);
	}

	// If hue was less than 0, add 360 to shift it into the 0-360 range
	if (h < 0.f)
	{
		h += 360.f;
	}
}

Color Color::HSVToRGB(const float h, const float s, const float v, const float a)
{
	return HSVToRGB(h, s, v, true, a);
}

Color Color::HSVToRGB(const float h, const float s, float v, const bool hdr, float a)
{
	// HDR's value can exceed 1, so if we don't want HDR, clamp the value
	v = !hdr ? Maths::Clamp01(v) : v;

	// Calculate the c, normalised hue and m terms
	const float c = v * s;
	const float normH = h / 60.f;
	const float m = v - c;

	// Calculate the x term: (C x (1 - |H' mod 2 - 1|)
	const float x = c * (1.f - Maths::Abs(Maths::FMod(normH, 2.f) - 1.f));

	float normR = 0.f, normG = 0.f, normB = 0.f;
	if (Maths::IsInRange(h, 0.f, 1.f))
	{
		normR = c;
		normG = x;
	}
	else if (Maths::IsInRange(h, 1.f, 2.f))
	{
		normR = x;
		normG = c;
	}
	else if (Maths::IsInRange(h, 2.f, 3.f))
	{
		normG = c;
		normB = x;
	}
	else if (Maths::IsInRange(h, 3.f, 4.f))
	{
		normG = x;
		normB = c;
	}
	else if (Maths::IsInRange(h, 4.f, 5.f))
	{
		normR = x;
		normB = c;
	}
	else if (Maths::IsInRange(h, 5.f, 6.f))
	{
		normR = c;
		normB = x;
	}

	// Modify the normalised values by the m term
	return { normR + m, normG + m, normB + m, a };
}

Color Color::Min(const Color& a, const Color& b)
{
	return a.Luminance() < b.Luminance() ? a : b;
}

Color Color::Max(const Color& a, const Color& b)
{
	return a.Luminance() > b.Luminance() ? a : b;
}

Color::Color()
	: r{ 0.f }, g{ 0.f }, b{ 0.f }, a{ 0.f }
{}

Color::Color(const uint32 hex)
	: Color{ static_cast<uint8>(hex >> 24), static_cast<uint8>(hex >> 16), static_cast<uint8>(hex >> 8), static_cast<uint8>(hex >> 0) }
{

}

Color::Color(const uint8 r, const uint8 g, const uint8 b)
	: r{ static_cast<float>(r) / 255.f }, g{ static_cast<float>(g) / 255.f },
	b{ static_cast<float>(b) / 255.f }, a{ 1.f }
{

}

Color::Color(const uint8 r, const uint8 g, const uint8 b, const uint8 a)
	: r{ static_cast<float>(r) / 255.f }, g{ static_cast<float>(g) / 255.f },
	b{ static_cast<float>(b) / 255.f }, a{ static_cast<float>(a) / 255.f }
{}

Color::Color(const float r, const float g, const float b)
	: r{ r }, g{ g }, b{ b }, a{ 1.f }
{}

Color::Color(const float r, const float g, const float b, const float a)
	: r{ r }, g{ g }, b{ b }, a{ a }
{}

Color::Color(const vec3& rgb)
	: r{ rgb.r }, g{ rgb.g }, b{ rgb.b }, a{ 1.f }
{
	
}

Color::Color(const vec4& rgba)
	: r{ rgba.r }, g{ rgba.g }, b{ rgba.b }, a{ rgba.a }
{}

Color::Color(const Color& rhs) = default;

Color Color::Linear() const
{
	constexpr float p = 2.2f;

	return Color
	{
		Maths::Pow(r, p),
		Maths::Pow(g, p),
		Maths::Pow(b, p),
		a
	};
}

void Color::ToLinear()
{
	constexpr float p = 2.2f;

	r = Maths::Pow(r, p);
	g = Maths::Pow(g, p);
	b = Maths::Pow(b, p);
}

Color Color::Gamma() const
{
	constexpr float p = 1.f / 2.2f;

	return Color
	{  
		Maths::Pow(r, p),
		Maths::Pow(g, p),
		Maths::Pow(b, p),
		a
	};
}

void Color::ToGamma()
{
	constexpr float p = 1.f / 2.2f;

	r = Maths::Pow(r, p);
	g = Maths::Pow(g, p);
	b = Maths::Pow(b, p);
}

float Color::Luminance() const
{
	return .2126f * r + .7152f * g + .0722f * b;
}

float Color::Hue() const
{
	float h, s, v;
	RGBToHSV(*this, h, s, v);

	return h;
}

float Color::Saturation() const
{
	float h, s, v;
	RGBToHSV(*this, h, s, v);

	return s;
}

float Color::Value() const
{
	float h, s, v;
	RGBToHSV(*this, h, s, v);

	return v;
}

Color Color::WithRed(const uint8 newR) const
{
	return Color{ static_cast<float>(newR), g, b, a };
}

Color Color::WithRed(const float newR) const
{
	return Color{ newR, g, b, a };
}

Color Color::WithGreen(const uint8 newG) const
{
	return Color{ r, static_cast<float>(newG), b, a };
}

Color Color::WithGreen(const float newG) const
{
	return Color{ r, newG, b, a };
}

Color Color::WithBlue(const uint8 newB) const
{
	return Color{ r, g, static_cast<float>(newB), a };
}

Color Color::WithBlue(const float newB) const
{
	return Color{ r, g, newB, a };
}

Color Color::WithAlpha(const uint8 newA) const
{
	return Color{ r, g, b, static_cast<float>(newA) };
}

Color Color::WithAlpha(const float newA) const
{
	return Color{ r, g, b, newA };
}

Color Color::Inverted() const
{
	return Color{ 255.f - r, 255.f - g, 255.f - b, a };
}

Color::operator vec3() const
{
	return vec3{ r, g, b };
}

Color::operator vec4() const
{
	return vec4{ r, g, b, a };
}

Color::operator VkClearColorValue() const
{
	Color gamma = Gamma();

	return { { gamma.r / 255.f, gamma.g / 255.f, gamma.b / 255.f, gamma.a / 255.f } };
}

bool Color::operator==(const Color& rhs) const
{
	// Early exit if the memory addresses are the same.
	if (this == &rhs)
	{
		return true;
	}

	// Compare each channel individually
	return Maths::Approx(r, rhs.r) && Maths::Approx(g, rhs.g) && 
		Maths::Approx(b, rhs.b) && Maths::Approx(a, rhs.a);
}

bool Color::operator!=(const Color& rhs) const
{
	// Early exit if the memory addresses are the same.
	if (this == &rhs)
	{
		return false;
	}

	return !Maths::Approx(r, rhs.r) || !Maths::Approx(g, rhs.g) ||
		!Maths::Approx(b, rhs.b) || !Maths::Approx(a, rhs.a);
}

Color Color::operator+(const Color& rhs) const
{
	return { r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a };
}

Color Color::operator+(const float rhs) const
{
	return { r + rhs, g + rhs, b + rhs, a + rhs };
}

Color Color::operator-(const Color& rhs) const
{
	return { r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a };
}

Color Color::operator*(const Color& rhs) const
{
	return { r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a };
}

Color Color::operator*(const float rhs) const
{
	return { r * rhs, g * rhs, b * rhs, a * rhs };
}

Color Color::operator/(const float rhs) const
{
	return { r / rhs, g / rhs, b / rhs, a / rhs };
}

float& Color::operator[](const int index)
{
	assert(index >= 0 && index < 4 && "Index out of bounds!");

	return (&r)[index];
}

const float& Color::operator[](const int index) const
{
	assert(index >= 0 && index < 4 && "Index out of bounds!");

	return (&r)[index];
}

Color& Color::operator=(const vec3& rhs)
{
	r = rhs.r;
	g = rhs.g;
	b = rhs.b;
	a = 1.f;

	return *this;
}

Color& Color::operator=(const vec4& rhs)
{
	r = rhs.r;
	g = rhs.g;
	b = rhs.b;
	a = rhs.a;

	return *this;
}

Color& Color::operator=(const Color& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	r = rhs.r;
	g = rhs.g;
	b = rhs.b;
	a = rhs.a;

	return *this;
}

ostream& operator<<(ostream& stream, const Color& color)
{
	stream << std::format(
		"({:.1f}, {:.1f}, {:.1f}, {:.1f})",
		color.r, color.g, color.b, color.a
	);

	return stream;
}
