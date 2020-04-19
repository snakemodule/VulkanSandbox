#include "UncompressedAnimation.h"

#include "AnimationStuff.h"

void UncompressedAnimation::plot(Skeleton s)
{
	struct plotQ {
		std::vector<float> w;
		std::vector<float> x;
		std::vector<float> y;
		std::vector<float> z;
		float* p_w;
		float* p_x;
		float* p_y;
		float* p_z;
		int L = 0;

		void update(float w, float x, float y, float z)
		{
			this->w.push_back(w);
			this->x.push_back(x);
			this->y.push_back(y);
			this->z.push_back(z);
			p_w = this->w.data();
			p_x = this->x.data();
			p_y = this->y.data();
			p_z = this->z.data();
			L++;
		}

		void clear() {
			w.clear();
			x.clear();
			y.clear();
			z.clear();
		}
	};

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
	};

	std::vector<plotQ> qplots1 = std::vector<plotQ>(s.jointCount);
	std::vector<plotV> vplots1 = std::vector<plotV>(s.jointCount);

	std::vector<plotQ> qplots2 = std::vector<plotQ>(s.jointCount);
	std::vector<plotV> vplots2 = std::vector<plotV>(s.jointCount);

	int L = 0;
	for (float t = 0; t < animationData.animationDuration; t += 0.01)
	{
		L++;

		updateEvaluators(0.01);
		evaluate(s, t);

		float TicksPerSecond = 30.0f;
		float TimeInTicks = t * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, animationData.animationDuration * TicksPerSecond);
		for (size_t i = 0; i < s.jointCount; i++)
		{
			qplots1[i].update(localRotation[i].w, localRotation[i].x,
				localRotation[i].y, localRotation[i].z);
			vplots1[i].update(localPosition[i].x,
				localPosition[i].y, localPosition[i].z);
			
			auto lt = AnimationStuff::makeAnimationMatrix(s.assimpAnimChannel[i], AnimationTime); //local animation transformation
			qplots2[i].update(lt.first.w, lt.first.x, lt.first.y, lt.first.z);
			vplots2[i].update(lt.second.x, lt.second.y, lt.second.z);

			auto lr = localRotation[i];
			double innerProd = lr.w * lt.first.w
				+ lr.x * lt.first.x
				+ lr.y * lt.first.y
				+ lr.z * lt.first.z;

			double d = 1 - std::pow(innerProd, 2);
			//assert(d < 0.05);
			if (d>0.05)
			{
				evaluate(s, t);
			}

		}
		
		/*
		Skeleton& s = mymodel->skeleton;

		// the root has no parent
		s.localTransform[0] = AnimationStuff::makeAnimationMatrix(s.animationChannel[0], AnimationTime); //local animation transformation
		s.globalTransform[0] = s.localTransform[0];					//for root node local=global
		s.finalTransformation[0] = s.globalTransform[0] * s.offsetMatrix[0];


		for (unsigned int i = 1; i < mymodel->jointIndex.size(); ++i)
		{
			const uint16_t parentJointIndex = s.hierarchy[i];
			s.localTransform[i] = AnimationStuff::makeAnimationMatrix(s.animationChannel[i], AnimationTime); //local animation transformation
			s.globalTransform[i] = s.globalTransform[parentJointIndex] * s.localTransform[i]; //animation transform in space of the parent
			s.finalTransformation[i] = s.globalTransform[i] * s.offsetMatrix[i];
		}
		*/
	}
}


void UncompressedAnimation::evaluate(Skeleton s, double t)
{

	float TicksPerSecond = 30.0f;
	float TimeInTicks = t * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, animationData.animationDuration * TicksPerSecond);

	for (size_t i = 0; i < rotInterpolators.size(); i++)
	{
		//44
		auto lt = AnimationStuff::makeAnimationMatrix(s.assimpAnimChannel[i], AnimationTime); //local animation transformation
		
		localRotation[i] = rotInterpolators[i].eval(animationTime); //todo 0-1
		
		auto lr = localRotation[i];
		double innerProd = lr.w * lt.first.w 
			+ lr.x * lt.first.x
			+ lr.y * lt.first.y 
			+ lr.z * lt.first.z;

		double d = 1 - std::pow(innerProd, 2);
		assert(d < 0.05);
		//localRotation[i] = rotInterpolators[i].eval(animationTime); //todo 0-1
		//localRotation[i] = glm::quat(lt.first.w, lt.first.x, lt.first.y, lt.first.z);
		
	}

	for (size_t i = 0; i < posInterpolators.size(); i++)
	{
		localPosition[i] = posInterpolators[i].eval(animationTime);
		//auto lt = AnimationStuff::makeAnimationMatrix(s.assimpAnimChannel[i], AnimationTime); //local animation transformation
		//localPosition[i] = glm::vec3(lt.second.x, lt.second.y, lt.second.z);
	}
}

void UncompressedAnimation::updateEvaluators(float t)
{
	if (animationTime + t >= (float)animationData.animationDuration)
	{
		initiateEvaluators();

	}
	animationTime = std::fmod((animationTime + t), (float)animationData.animationDuration);

	auto channel = latestPosData.channel;
	while (posKeys[channel][posKeyRingIndex[channel] + 2].time < animationTime && posDataIndex < animationData.posKeyCount) {

		// insert new key
		posKeys[channel][posKeyRingIndex[channel] + 0] = latestPosData;
		++posDataIndex;
		++posKeyRingIndex[channel];

		posInterpolators[channel] = Vector3Interpolation(
			posKeys[channel][posKeyRingIndex[channel] + 0].pos,
			posKeys[channel][posKeyRingIndex[channel] + 1].pos,
			posKeys[channel][posKeyRingIndex[channel] + 2].pos,
			posKeys[channel][posKeyRingIndex[channel] + 3].pos,
			posKeys[channel][posKeyRingIndex[channel] + 1].time,
			posKeys[channel][posKeyRingIndex[channel] + 2].time);

		//get next key if available
		if (posDataIndex < animationData.posKeyCount) {
			getPosData();
			channel = latestPosData.channel;
		}
	}
	
	channel = latestRotData.channel;
	
	while (rotKeys[channel][rotKeyRingIndex[channel] + 2].time < animationTime && rotDataIndex < animationData.rotKeyCount) {
		// insert new key
		rotKeys[channel][rotKeyRingIndex[channel] + 0] = latestRotData;

		++rotDataIndex;
		++rotKeyRingIndex[channel];

		rotInterpolators[channel] = QuaternionInterpolation(
			rotKeys[channel][rotKeyRingIndex[channel] + 0].rot,
			rotKeys[channel][rotKeyRingIndex[channel] + 1].rot,
			rotKeys[channel][rotKeyRingIndex[channel] + 2].rot,
			rotKeys[channel][rotKeyRingIndex[channel] + 3].rot,
			rotKeys[channel][rotKeyRingIndex[channel] + 1].time,
			rotKeys[channel][rotKeyRingIndex[channel] + 2].time);

		//get next key if available
		if (rotDataIndex < animationData.rotKeyCount) {
			getRotData();
			channel = latestRotData.channel;
		}
	}
}

void UncompressedAnimation::initiateEvaluators()
{
	posDataIndex = 0;
	rotDataIndex = 0;
	for (unsigned int i = 0; i < rotKeys.size(); i++)
	{
		auto & ring = rotKeyRingIndex[i];
		auto & rk = rotKeys[i];
		for (size_t j = 0; j < 4; j++)
		{
			rk[ring + j] = getRotData();
			rotDataIndex++;
		}
		rotInterpolators[i] = QuaternionInterpolation(rk[ring + 0].rot,
			rk[ring + 1].rot, rk[ring + 2].rot, rk[ring + 3].rot,
			rk[ring + 1].time, rk[ring + 2].time);
	}
	for (unsigned int i = 0; i < posKeys.size(); i++)
	{
		auto & ring = posKeyRingIndex[i];
		auto & pk = posKeys[i];
		for (size_t j = 0; j < 4; j++)
		{
			pk[ring + j] = getPosData();
			posDataIndex++;
		}
		posInterpolators[i] = Vector3Interpolation(pk[ring + 0].pos,
			pk[ring + 1].pos, pk[ring + 2].pos, pk[ring + 3].pos,
			pk[ring + 1].time, pk[ring + 2].time);
	}
	if (posDataIndex < animationData.posData.size())
	{
		getPosData();
	}
	if (rotDataIndex < animationData.rotData.size())
	{
		getRotData();
	}
}

UncompressedAnimation::UncompressedAnimation(int jointCount, UncompressedAnimationKeys & animationData)
	: animationData(animationData),
	rotKeyRingIndex(jointCount),
	posKeyRingIndex(jointCount),
	rotKeys(jointCount),
	posKeys(jointCount),
	rotInterpolators(jointCount),
	posInterpolators(jointCount),
	localRotation(jointCount),
	localPosition(jointCount)
{
	this->animationData = animationData;
	initiateEvaluators();
}

UncompressedAnimation::~UncompressedAnimation()
{
}

UncompressedAnimation::quatKey UncompressedAnimation::getRotData()
{
	auto & k = animationData.rotData[rotDataIndex];
	latestRotData = { glm::quat(k.w, k.x, k.y, k.z), (float)k.time/30, k.channel_id };
	return latestRotData;
}

UncompressedAnimation::quatKey UncompressedAnimation::getNextRotData()
{
	++rotDataIndex;
	return getRotData();
}

UncompressedAnimation::vecKey UncompressedAnimation::getPosData()
{
	auto & k = animationData.posData[posDataIndex];
	latestPosData = { glm::vec3(k.x, k.y, k.z), (float)k.time / 30, k.channel_id };
	return latestPosData;
}

UncompressedAnimation::vecKey UncompressedAnimation::getNextPosData()
{
	++posDataIndex;
	return getPosData();
}
