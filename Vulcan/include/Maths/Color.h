#pragma once

#include <ostream>
#include <string>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vulkan/vulkan.h>

#include "Object.h"
#include "Maths/Alias.h"

using std::ostream;
using std::string;

using glm::vec3;
using glm::vec4;

/** 
 * @brief A custom colour structure that allows for various common functionality.
 */
struct Color
{
public:
	/** @brief #000000FF (0, 0, 0, 255) */
	static const Color BLACK;
	/** @brief #FFFFFFFF (255, 255, 255, 255) */
	static const Color WHITE;
	/** @brief #00000000 (0, 0, 0, 255) */
	static const Color BLANK;
	/** @brief #FF0000FF (255, 0, 0, 255) */
	static const Color RED;
	/** @brief #00FF00FF (0, 255, 0, 255) */
	static const Color GREEN;
	/** @brief #0000FFFF (0, 0, 255, 255) */
	static const Color BLUE;
	/** @brief #FFFF00FF (255, 255, 0, 255) */
	static const Color YELLOW;
	/** @brief #964800FF (150, 72, 0, 255) */
	static const Color BROWN;
	/** @brief #FF8000FF (255, 128, 255, 255) */
	static const Color ORANGE;
	/** @brief #FFC0CBFF (255, 192, 203, 255) */
	static const Color PINK;
	/** @brief #4B0082FF (75, 0, 130, 255) */
	static const Color PURPLE;
	/** @brief #FF00FFFF (255, 0, 255, 255) */
	static const Color MAGENTA;

public:
	/** @brief Linearly interpolates between two values with a clamped t value. */
	static Color Lerp(const Color& a, const Color& b, float t);

	/** @brief Linearly interpolates between two values with an unclamped t value. */
	static Color LerpUnclamped(const Color& a, const Color& b, float t);

	/**
	 * @brief Creates a colour from a hexadecimal style string.
	 * @param hex The hexadecimal string.
	 */
	static Color FromHex(const char* hex);

	/** 
	 * @brief Gets the HSV components of the passed RGB color. 
	 * @param rgb The colour to get the HSV components of.
	 * @param h The hue component. (0-360)
	 * @param s The saturation component. (0-1)
	 * @param v The value component. (0-1)
	 */
	static void RGBToHSV(const Color& rgb, float& h, float& s, float& v);

	/** 
	 * @brief Creates an HDR colour in RGB format using HSV values. 
	 * @param h The hue component. (0-360)
	 * @param s The saturation component. (0-1)
	 * @param v The value component. (0-1)
	 * @param a
	 */
	static Color HSVToRGB(float h, float s, float v, float a);

	/** 
	 * @brief Creates a colour in RGB format using HSV values. 
	 * @param h The hue component. (0-360)
	 * @param s The saturation component. (0-1)
	 * @param v The value component. (0-1)
	 * @param hdr Whether the colour is in the High Definition Range.
	 * @param a
	 */
	static Color HSVToRGB(float h, float s, float v, bool hdr, float a);

	/** @brief Returns the smaller of two inputs. */
	static Color Min(const Color& a, const Color& b);

	/** @brief Returns the larger of two inputs. */
	static Color Max(const Color& a, const Color& b);

public:
	/** @brief The red channel of the color. */
	float r;
	/** @brief The green channel of the color. */
	float g;
	/** @brief The blue channel of the color. */
	float b;
	/** @brief The alpha channel of the color. */
	float a;

public:
	/** @brief The default constructor which sets all values to 0. */
	Color();

	/** @brief Creates a colour from a hexadecimal value. */
	explicit Color(uint32 hex);

	/** 
	 * @brief Creates a colour from 3 unsigned integers, alpha value maximised. 
	 * @param r The red channel. (0-255)
	 * @param g The green channel. (0-255)
	 * @param b The blue channel. (0-255)
	 */
	Color(uint8 r, uint8 g, uint8 b);

	/** 
	 * @brief Creates a colour from 4 unsigned integers. 
	 * @param r The red channel. (0-255)
	 * @param g The green channel. (0-255)
	 * @param b The blue channel. (0-255)
	 * @param a The alpha (transparency) channel. (0-255)
	 */
	Color(uint8 r, uint8 g, uint8 b, uint8 a);

	/**
	 * @brief Creates a colour from 3 floats, alpha value maximised.
	 * @param r The red channel. (0-1)
	 * @param g The green channel. (0-1)
	 * @param b The blue channel. (0-1)
	 */
	Color(float r, float g, float b);

	/**
	 * @brief Creates a colour from 4 floats.
	 * @param r The red channel. (0-1)
	 * @param g The green channel. (0-1)
	 * @param b The blue channel. (0-1)
	 * @param a The alpha (transparency) channel. (0-1)
	 */
	Color(float r, float g, float b, float a);

	/**
	 * @brief Converts a GLM/Vulkan formatted colour into this type.
	 * @param rgb The GLM/Vulkan RGB color.
	 */
	explicit Color(const vec3& rgb);

	/**
	 * @brief Converts a GLM/Vulkan formatted colour into this type.
	 * @param rgba The GLM/Vulkan RGBA color.
	 */
	explicit Color(const vec4& rgba);

	/** @brief Copies the right hand side value into this one. */
	Color(const Color& rhs);

	/** @brief Gets the colour in linear space. */
	[[nodiscard]] Color Linear() const;

	/** @brief Converts this colour into linear space. */
	void ToLinear();

	/** @brief Gets the colour in gamma space. */
	[[nodiscard]] Color Gamma() const;

	/** @brief Converts this colour into gamma space. */
	void ToGamma();

	/** @brief Gets the perceived luminance of the colour. */
	[[nodiscard]] float Luminance() const;

	/** @brief Gets the hue of the colour */
	[[nodiscard]] float Hue() const;

	/** @brief Gets the saturation of the colour */
	[[nodiscard]] float Saturation() const;

	/** @brief Gets the value of the colour */
	[[nodiscard]] float Value() const;

	/** 
	 * @brief Creates a new colour with an overridden red component. 
	 * @param newR The new red component. (0-255)
	 */
	[[nodiscard]] Color WithRed(uint8 newR) const;

	/**
	 * @brief Creates a new colour with an overridden red component.
	 * @param newR The new red component. (0-1)
	 */
	[[nodiscard]] Color WithRed(float newR) const;

	/**
	 * @brief Creates a new colour with an overridden green component.
	 * @param newG The new green component. (0-255)
	 */
	[[nodiscard]] Color WithGreen(uint8 newG) const;

	/**
	 * @brief Creates a new colour with an overridden green component.
	 * @param newG The new green component. (0-1)
	 */
	[[nodiscard]] Color WithGreen(float newG) const;

	/**
	 * @brief Creates a new colour with an overridden blue component.
	 * @param newB The new blue component. (0-255)
	 */
	[[nodiscard]] Color WithBlue(uint8 newB) const;

	/**
	 * @brief Creates a new colour with an overridden blue component.
	 * @param newB The new blue component. (0-1)
	 */
	[[nodiscard]] Color WithBlue(float newB) const;

	/**
	 * @brief Creates a new colour with an overridden alpha component.
	 * @param newA The new alpha component. (0-255)
	 */
	[[nodiscard]] Color WithAlpha(uint8 newA) const;

	/**
	 * @brief Creates a new colour with an overridden alpha component.
	 * @param newA The new alpha component. (0-1)
	 */
	[[nodiscard]] Color WithAlpha(float newA) const;

	/** @brief Returns a colour with the RGB channels inverted (255.f - CHANNEL) */
	[[nodiscard]] Color Inverted() const;

public:
	/** @brief Converts this Color to a GLM @c vec3 with components in [0.0, 1.0]. */
	explicit operator vec3() const;

	/** @brief Converts this Color to a GLM @c vec4 with components in [0.0, 1.0]. */
	explicit operator vec4() const;

	explicit operator VkClearColorValue() const;

	/** @brief Returns @c true if both colors have identical RGBA values. */
	bool operator==(const Color& rhs) const;

	/** @brief Returns @c false if the colors differ in any channels. */
	bool operator!=(const Color& rhs) const;

	Color operator+(const Color& rhs) const;

	Color operator+(float rhs) const;

	Color operator-(const Color& rhs) const;

	Color operator*(const Color& rhs) const;

	Color operator*(float rhs) const;

	Color operator/(float rhs) const;

	float& operator[](int index);

	const float& operator[](int index) const;

	/**
	 * @brief Assigns a GLM @c vec3 to this Color.
	 * @param rhs A vector with components in the range [0.0, 1.0], ordered (r, g, b).
	 */
	Color& operator=(const vec3& rhs);

	/**
	 * @brief Assigns a GLM @c vec4 to this Color.
	 * @param rhs A vector with components in the range [0.0, 1.0], ordered (r, g, b, a).
	 */
	Color& operator=(const vec4& rhs);

	/** @brief Copy-assignment operator. */
	Color& operator=(const Color& rhs);

	/** @brief Writes the color to a stream as @c "(r, g, b, a)". */
	friend ostream& operator<<(ostream& stream, const Color& color);

};