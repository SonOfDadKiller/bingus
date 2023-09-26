#include "bingus.h"

double wrapMax(double x, double max)
{
	return fmod(max + fmod(x, max), max);
}

double wrapMinMax(double x, double min, double max)
{
	return min + wrapMax(x - min, max - min);
}

float wrapMax(float x, float max)
{
	return fmod(max + fmod(x, max), max);
}

float wrapMinMax(float x, float min, float max)
{
	return min + wrapMax(x - min, max - min);
}

vec4 hsv(vec4 hsv)
{
	hsv.x *= 360.f;

	vec4 rgb;
	float H = hsv.x, S = hsv.y, V = hsv.z,
		P, Q, T,
		fract;

	(H == 360.f) ? (H = 0.f) : (H /= 60.f);
	fract = H - floor(H);

	P = V * (1.f - S);
	Q = V * (1.f - S * fract);
	T = V * (1.f - S * (1.f - fract));

	if (0.f <= H && H < 1.f)
		rgb = vec4(V, T, P, hsv.w);
	else if (1.f <= H && H < 2.f)
		rgb = vec4(Q, V, P, hsv.w);
	else if (2.f <= H && H < 3.f)
		rgb = vec4(P, V, T, hsv.w);
	else if (3.f <= H && H < 4.f)
		rgb = vec4(P, Q, V, hsv.w);
	else if (4.f <= H && H < 5.f)
		rgb = vec4(T, P, V, hsv.w);
	else if (5.f <= H && H < 6.f)
		rgb = vec4(V, P, Q, hsv.w);
	else
		rgb = vec4(0, 0, 0, hsv.w);

	return rgb;
}

vec2 PixelToWorld(vec2 pixelCoord)
{
	pixelCoord = (pixelCoord / GetWindowSize()) * 2.f - 1.f;
	vec4 origin = cameraViewProjInverse * vec4(pixelCoord, 0.f, 1.f);
	origin.w = 1.0f / origin.w;
	return vec2(origin.x, origin.y) * origin.w;
}

vec3 PixelToWorld(vec3 pixelCoord)
{
	pixelCoord = (pixelCoord / vec3(GetWindowSize(), 0)) * 2.f - 1.f;
	vec4 origin = cameraViewProjInverse * vec4(pixelCoord.x, pixelCoord.y, 0.f, 1.f);
	origin.w = 1.0f / origin.w;
	return vec3(origin.x, origin.y, origin.z) * origin.w;
}

vec2 WorldToPixel(vec2 worldCoord)
{
	vec4 origin = cameraViewProj * vec4(worldCoord, 0.f, 1.f);
	return vec2(origin.x, -origin.y) * GetWindowSize() / 2.f;
}

vec2 PixelToNDC(vec2 pixelCoord)
{
	return (pixelCoord / (GetWindowSize() / 2.f)) - vec2(1);
}

vec3 PixelToNDC(vec3 pixelCoord)
{
	return (pixelCoord / (vec3(GetWindowSize().x, GetWindowSize().y, 1.f) / 2.f)) - vec3(1);
}

float ParseFloat(std::string input, float oldValue)
{
	std::string::size_type sz;

	try 
	{
		return std::stof(input, &sz);
	}
	catch (...)
	{
		return oldValue;
	}
}