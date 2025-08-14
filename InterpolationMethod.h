#pragma once

enum class InterpolationMethod {
	LINEAR,                // Linear interpolation
	CUBIC_SPLINE,         // Cubic spline interpolation
	AKIMA_SPLINE,         // Akima spline interpolation
	MONOTONIC_CUBIC_SPLINE       // Monotonic cubic spline interpolation
};
