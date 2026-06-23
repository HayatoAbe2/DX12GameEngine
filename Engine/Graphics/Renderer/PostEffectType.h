#pragma once

enum class PostEffectType {
	None,
	Grayscale,
	Vignette,
	GaussianFilter3x3,
	BoxFilter5x5,
	Outline,
	RadialBlur,
	Dissolve,

	Count
};