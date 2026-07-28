#pragma once

#include <algorithm>
#include <cstdint>

enum class PostEffectType : uint32_t
{
	None,
	Grayscale,
	Sepia,
	Vignette,
	BoxFilter3x3,
	BoxFilter5x5,
	GaussianBlur,
	RadialBlur,
	LuminanceOutline,
	DepthOutline,
	Random,
	Dissolve,
	Count
};

struct PostEffectSettings
{
	PostEffectType type = PostEffectType::None;
	float threshold = 1.0f;
	float noiseStrength = 0.8f;
	float vignettePower = 0.8f;
};

class PostEffectController
{
public:

	void SetType(PostEffectType type)
	{
		settings_.type = type;
	}

	PostEffectType GetType() const
	{
		return settings_.type;
	}

	void SetThreshold(float threshold)
	{
		settings_.threshold = std::clamp(threshold, 0.0f, 1.0f);
	}

	PostEffectSettings& GetSettings()
	{
		return settings_;
	}

	const PostEffectSettings& GetSettings() const
	{
		return settings_;
	}

private:

	PostEffectSettings settings_;
};
