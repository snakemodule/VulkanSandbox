#pragma once
#include <vector>
#include <math.h>

#include "Skeleton.h"
#include "glm/gtx/compatibility.hpp"
//#include "Util.h"
#include "SkeletonAnimation.h"


struct AnimationLayer
{
	std::vector<SkeletonAnimation> blendAnimations;
	std::vector<float> jointOpacity = std::vector<float>(0);
	glm::quat* blendedRotation = nullptr;
	glm::vec3* blendedPosition = nullptr;
};


class SkeletalAnimationComponent
{
	struct plotV {
		std::vector<float> x;
		std::vector<float> y;
		std::vector<float> z;
		float* p_x;
		float* p_y;
		float* p_z;
		int L = 0;

		void update(float x, float y, float z)
		{
			this->x.push_back(x);
			this->y.push_back(y);
			this->z.push_back(z);
			p_x = this->x.data();
			p_y = this->y.data();
			p_z = this->z.data();
			L++;
		}

		void clear() {
			x.clear();
			y.clear();
			z.clear();
		}
	} livePlot, liveSamp;
	int L = 0;

public:
	//[layer][blending animations]
	std::vector<AnimationLayer> layers;
	Skeleton * skeleton = nullptr;
	uint8_t jointCount = 0;

	//std::vector<uint8_t> parentIndex;
	
	void init(Skeleton * skeleton, std::vector<AnimationLayer>&& layers)
	{
		this->skeleton = skeleton;
		this->jointCount = (uint8_t)skeleton->offsetMatrix.size();
		this->transformations.resize(jointCount);
		this->layers = std::move(layers);
	}

	void evaluate(float deltaTime, std::vector<float> blendFactors) {

		auto layerCount = layers.size();
		for (size_t i = 0; i < layerCount; i++)
		{
			auto & layer = layers[i];
			auto  highestAnimationIndex = layers[i].blendAnimations.size() - 1;
			float playbackSpeed = 1.0f;
			if (layers[i].blendAnimations.size() > 1)
			{
				//find index of two animations to interpolate between
				float span = 1.0f / highestAnimationIndex;
				int index;
				double factor = blendFactors[i];
				if (factor < 1.0f)
					index = std::floor(factor / span);
				else
					index = highestAnimationIndex - 1;
				auto & blendAnim0 = layer.blendAnimations[index];
				auto & blendAnim1 = layer.blendAnimations[index + 1];
				

				//advance animation time depending on duration of the interpolated animations todo(???)
				auto dur0 = blendAnim0.getAnimationDuration();
				auto dur1 = blendAnim1.getAnimationDuration();
				float targetDuration = glm::lerp(dur0, dur1, factor);
				for (size_t j = 0; j < layer.blendAnimations.size(); j++) {
					playbackSpeed = layer.blendAnimations[j].getAnimationDuration() / targetDuration;
					layer.blendAnimations[j].updateEvaluators(deltaTime, playbackSpeed);
				}
				//evaluate and interpolate
				blendAnim0.evaluate();
				blendAnim1.evaluate();
				float a = std::fmod(factor, span) / span;
				for (size_t j = 0; j < jointCount; j++) {
					blendAnim0.localRotation[j] = glm::slerp(blendAnim0.localRotation[j],
							blendAnim1.localRotation[j], a);
				}
				for (size_t j = 0; j < jointCount; j++) {
					blendAnim0.localPosition[j] = glm::lerp(blendAnim0.localPosition[j],
							blendAnim1.localPosition[j], a);
				}
				layer.blendedRotation = blendAnim0.localRotation.data();
				layer.blendedPosition = blendAnim0.localPosition.data();
			}
			else
			{
				layer.blendAnimations[0].updateEvaluators(deltaTime);
				layer.blendAnimations[0].evaluate();
				layer.blendedRotation = layer.blendAnimations[0].localRotation.data();
				layer.blendedPosition = layer.blendAnimations[0].localPosition.data();
			}
			
		}
		/*
		for (size_t i = 0; i < layerCount; i++)
		{
			auto & layer = layers[i];
			auto  animationCount = layers[i].blendAnimations.size();
			if (animationCount > 1)
			{
				float span = 1.0f / (animationCount - 1);
				int index;
				auto & factor = blendFactors[i];
				if (factor < 1.0f)
					index = std::floor(factor / span);
				else
					index = (animationCount - 1);
				float remainder = std::fmod(factor, span);

				auto & blendAnim0 = layer.blendAnimations[index];
				auto & blendAnim1 = layer.blendAnimations[index + 1];
				blendAnim0.evaluate(blendAnim1.getAnimationDuration());
				blendAnim1.evaluate(blendAnim1.getAnimationDuration());
				auto & lr0 = blendAnim0.localRotation;
				auto & lr1 = blendAnim1.localRotation;
				auto & lp0 = blendAnim0.localPosition;
				auto & lp1 = blendAnim1.localPosition;
				for (size_t j = 0; j < jointCount; j++)
					lr0[j] = glm::slerp(lr0[j], lr1[j], remainder / span);
				for (size_t j = 0; j < jointCount; j++)
					lp0[j] = glm::lerp(lp0[j], lp1[j], remainder / span);
				layer.blendedRotation = lr0.data();
				layer.blendedPosition = lp0.data();
			}
			else
			{
				layer.blendAnimations[0].evaluate();
				layer.blendedRotation = layer.blendAnimations[0].localRotation.data();
				layer.blendedPosition = layer.blendAnimations[0].localPosition.data();
			}

		}

		*/
		//blend all layers into base layer
		auto & accumulatorRotations = layers[0].blendedRotation;
		auto & accumulatorPositions = layers[0].blendedPosition;
		for (size_t i = 1; i < layerCount; i++)
			for (size_t joint = 0; joint < jointCount; joint++) {
				auto & currentLayerRotations = layers[i].blendedRotation;
				accumulatorRotations[joint] = glm::slerp(accumulatorRotations[joint], currentLayerRotations[joint], layers[i].jointOpacity[joint]);
			}

		for (size_t i = 1; i < layerCount; i++)
			for (size_t joint = 0; joint < jointCount; joint++)	{
				auto & currentLayerPositions = layers[i].blendedPosition;
				accumulatorPositions[joint] = glm::lerp(accumulatorPositions[joint], currentLayerPositions[joint], layers[i].jointOpacity[joint]);
			}
		//base layer -> local transformation matrices
		for (size_t i = 0; i < jointCount; i++)
			transformations[i] = glm::translate(glm::mat4(1.0f), accumulatorPositions[i]) * glm::toMat4(accumulatorRotations[i]);
		//local matrices -> global matrices
		for (size_t i = 1; i < jointCount; i++)
			transformations[i] = transformations[skeleton->hierarchy[i]] * transformations[i];
		//apply inverse bind pose
		for (size_t i = 0; i < jointCount; i++)
			transformations[i] = transformations[i] * skeleton->offsetMatrix[i];
	}



	std::vector<glm::mat4> transformations;
};

