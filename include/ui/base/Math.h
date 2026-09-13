#pragma once

#include <ui/base/Header.h>
#include <type_traits>

namespace ui {

using byte = unsigned char;
template <typename ColourType, int AlphaDefault, bool PremultipliedAlpha>
class Colour;
using Colourb = Colour<byte, 255, false>;
using ColourbPremultiplied = Colour<byte, 255, true>;
template <typename Type>
class Vector2;
using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
template <typename Type>
class Rectangle;
using Rectanglef = Rectangle<float>;

namespace Math {

	constexpr float UI_PI = 3.141592653f;

	template <typename Type>
	Type Max(Type a, Type b)
	{
		return (a > b) ? a : b;
	}

	template <typename Type>
	Type Min(Type a, Type b)
	{
		return (a < b) ? a : b;
	}

	template <typename Type>
	Type Clamp(Type value, Type min, Type max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}

	template <typename Type>
	Type Lerp(float t, Type v0, Type v1)
	{
		static_assert(!std::is_same<Type, Colourb>::value, "Lerp currently cannot be used with Colourb. Use RoundedLerp instead.");
		return v0 * (1.0f - t) + v1 * t;
	}

	/// Element-wise maximum.
	template <>
	UI_CORE_API Vector2f Max<Vector2f>(Vector2f a, Vector2f b);
	template <>
	UI_CORE_API Vector2i Max<Vector2i>(Vector2i a, Vector2i b);
	/// Element-wise minimum.
	template <>
	UI_CORE_API Vector2f Min<Vector2f>(Vector2f a, Vector2f b);
	template <>
	UI_CORE_API Vector2i Min<Vector2i>(Vector2i a, Vector2i b);
	/// Element-wise clamp.
	template <>
	UI_CORE_API Vector2f Clamp<Vector2f>(Vector2f value, Vector2f min, Vector2f max);
	template <>
	UI_CORE_API Vector2i Clamp<Vector2i>(Vector2i value, Vector2i min, Vector2i max);

	/// Color interpolation.
	UI_CORE_API ColourbPremultiplied RoundedLerp(float t, ColourbPremultiplied c0, ColourbPremultiplied c1);

	/// Evaluates if a number is zero or close to it.
	/// @param[in] value The number to compare to zero.
	/// @return True if the number is zero or close to it, false otherwise.
	UI_CORE_API bool IsCloseToZero(float value);

	/// Calculates the absolute value of a number.
	/// @param[in] value The number to get the absolute value of.
	/// @return The absolute value of the number.
	UI_CORE_API float Absolute(float value);
	/// Calculates the absolute value of a number.
	/// @param[in] value The number to get the absolute value of.
	/// @return The absolute value of the number.
	UI_CORE_API int Absolute(int value);
	/// Calculates the component-wise absolute value of a vector.
	/// @param[in] value The vector to get the absolute value of.
	/// @return The absolute value of the vector.
	UI_CORE_API Vector2f Absolute(Vector2f value);

	/// Calculates the cosine of an angle.
	/// @param[in] angle The angle to calculate the cosine of, in radians.
	/// @return The cosine of the angle.
	UI_CORE_API float Cos(float angle);
	/// Calculates the arc-cosine of a value.
	/// @param[in] value The value to calculate the arc-cosine of.
	/// @return The angle, in radians.
	UI_CORE_API float ACos(float value);
	/// Calculates the sine of an angle.
	/// @param[in] angle The angle to calculate the sine of, in radians.
	/// @return The sine of the angle.
	UI_CORE_API float Sin(float angle);
	/// Calculates the arc-sine of a value.
	/// @param[in] value The value to calculate the arc-sine of.
	/// @return The angle, in radians.
	UI_CORE_API float ASin(float value);
	/// Calculates the tangent of an angle.
	/// @param[in] angle The angle to calculate the tangent of, in radians.
	/// @return The tangent of the angle.
	UI_CORE_API float Tan(float angle);
	/// Calculates the angle of a two-dimensional line.
	/// @param[in] y The y-component of the line.
	/// @param[in] x The x-component of the line.
	/// @return The angle of the line in radians.
	UI_CORE_API float ATan2(float y, float x);
	/// Evaluates the natural exponential function on a value.
	/// @param[in] value The value
	/// @return e^(value)
	UI_CORE_API float Exp(float value);
	/// Evaluates the base-2 logarithm of an integer.
	/// @param[in] value The value
	/// @return log2(value)
	UI_CORE_API int Log2(int value);

	/// Converts an angle from radians to degrees.
	/// @param[in] angle The angle, in radians.
	/// @return The angle converted to degrees.
	UI_CORE_API float RadiansToDegrees(float angle);
	/// Converts an angle from degrees to radians.
	/// @param[in] angle The angle, in degrees.
	/// @return The angle converted to radians.
	UI_CORE_API float DegreesToRadians(float angle);
	/// Normalises an angle in radians to [0, 2pi).
	/// @param[in] angle The angle, in radians.
	/// @return The normalised angle.
	UI_CORE_API float NormaliseAngle(float angle);

	/// Calculates the square root of a value.
	/// @param[in] value The value to calculate the square root of. This must be above zero.
	/// @return The square root of the value.
	UI_CORE_API float SquareRoot(float value);

	/// Rounds a floating-point value to the nearest integer.
	/// @param[in] value The value to round.
	/// @return The rounded integer as float.
	UI_CORE_API float Round(float value);
	/// Rounds a floating-point value to the nearest integer.
	/// @param[in] value The value to round.
	/// @return The rounded integer as double.
	UI_CORE_API double Round(double value);
	/// Rounds a floating-point value up to the nearest integer.
	/// @param[in] value The value to round.
	/// @return The rounded integer as float.
	UI_CORE_API float RoundUp(float value);
	/// Rounds a floating-point value down to the nearest integer.
	/// @param[in] value The value to round.
	/// @return The rounded integer as float.
	UI_CORE_API float RoundDown(float value);
	/// Rounds a floating-point value to the nearest integer.
	/// @param[in] value The value to round.
	/// @return The rounded integer.
	UI_CORE_API int RoundToInteger(float value);
	/// Rounds a floating-point value up to the nearest integer.
	/// @param[in] value The value to round.
	/// @return The rounded integer.
	UI_CORE_API int RoundUpToInteger(float value);
	/// Rounds a floating-point value down to the nearest integer.
	/// @param[in] value The value to round.
	/// @return The rounded integer.
	UI_CORE_API int RoundDownToInteger(float value);

	/// Decomposes a floating-point value to its fractional and integral parts.
	/// @param[in] value The value to decompose.
	/// @param[out] integral The integral part of the value.
	/// @return The fractional part of the value.
	UI_CORE_API float DecomposeFractionalIntegral(float value, float* integral);

	/// Round the position and width of a line segment to the pixel grid while minimizing movement of the edges.
	/// @param[inout] x The position, which will use normal rounding.
	/// @param[inout] width The width, which is rounded such that movement of the right edge is minimized.
	UI_CORE_API void SnapToPixelGrid(float& x, float& width);
	/// Round the position and size of a rectangle to the pixel grid while minimizing movement of the edges.
	/// @param[inout] position The position, which will use normal rounding.
	/// @param[inout] size The size, which is rounded such that movement of the right and bottom edges is minimized.
	UI_CORE_API void SnapToPixelGrid(Vector2f& position, Vector2f& size);
	/// Round the rectangle to the pixel grid while minimizing movement of the edges.
	/// @param[inout] rectangle The rectangle to round.
	UI_CORE_API void SnapToPixelGrid(Rectanglef& rectangle);
	/// Round the position and size of a rectangle to the pixel grid such that it fully covers the original rectangle.
	/// @param[inout] position The position, which will be rounded down.
	/// @param[inout] size The size, which is rounded such that the right and bottom edges are rounded up.
	UI_CORE_API void ExpandToPixelGrid(Vector2f& position, Vector2f& size);
	/// Round the rectangle to the pixel grid such that it fully covers the original rectangle.
	/// @param[inout] rectangle The rectangle to round.
	UI_CORE_API void ExpandToPixelGrid(Rectanglef& rectangle);

	/// Converts a number to the nearest power of two, rounding up if necessary.
	/// @param[in] value The value to convert to a power-of-two.
	/// @return The smallest power of two that is as least as big as the input value.
	UI_CORE_API int ToPowerOfTwo(int value);

	/// Converts from the ASCII representation of a hexadecimal digit to decimal.
	/// @param[in] hex_digit The hexadecimal digit to convert to decimal.
	/// @return The digit in decimal.
	UI_CORE_API int HexToDecimal(char hex_digit);

	/// Generates a random floating-point value between 0 and a user-specified value.
	/// @param[in] max_value The limit to random value. The generated value will be guaranteed to be below this limit.
	/// @return The random value.
	UI_CORE_API float RandomReal(float max_value);
	/// Generates a random integer value between 0 and a user-specified value.
	/// @param[in] max_value The limit to random value. The generated value will be guaranteed to be below this limit.
	/// @return The random value.
	UI_CORE_API int RandomInteger(int max_value);
	/// Generates a random boolean value, with an equal chance of true or false.
	/// @return The random value.
	UI_CORE_API bool RandomBool();

} // namespace Math
} // namespace ui
