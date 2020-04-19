#include "SkeletonAnimation.h"

#include "AnimationStuff.h"

void SkeletonAnimation::plot(Skeleton s)
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
		int L =0;

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
	for (float t = 0; t  < animationData.animationDuration; t += 0.01)
	{
		L++;

		float TicksPerSecond = 30.0f;
		float TimeInTicks = t * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, animationData.animationDuration * TicksPerSecond);


		updateEvaluators(0.01);
		evaluate(s, t);
		for (size_t i = 0; i < s.jointCount; i++)
		{
			
			qplots1[i].update(localRotation[i].w, localRotation[i].x,
				localRotation[i].y, localRotation[i].z);
			//vplots1[i].update(localPosition[i].x,
			//	localPosition[i].y, localPosition[i].z);

			auto lr = localRotation[i];

			auto q = AnimationStuff::makeAnimationMatrix(s.assimpAnimChannel[i], AnimationTime).first; //local animation transformation
			
			if (q.w < 0) { q = { -q.w, -q.x, -q.y, -q.z }; }
			
			/*double p = q.w * lr.w
				+ q.x * lr.x
				+ q.y * lr.y
				+ q.z * lr.z;
			if (std::abs(std::fmod(p, 1.0f)) < 0.0010) { p = std::round(p); }
			double d_theta = std::acos(2 * p*p - 1);
			assert(d_theta == d_theta); //NaN check
			*/

			if (i == 44)
			{
				qplots2[i].update(q.w, q.x, q.y, q.z);

			}
			else
			{
				qplots2[i].update(q.w, q.x, q.y, q.z);
			}
			//vplots2[i].update(lt.second.x, lt.second.y, lt.second.z);


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

void SkeletonAnimation::evaluate(Skeleton s, double t)
{

	float TicksPerSecond = 30.0f;
	float TimeInTicks = t * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, animationData.animationDuration * TicksPerSecond);

	for (size_t i = 0; i < rotInterpolators.size(); i++)
	{
		//44
		auto q = AnimationStuff::makeAnimationMatrix(s.assimpAnimChannel[i], AnimationTime).first; //local animation transformation
		if (q.w < 0) { q = { -q.w, -q.x, -q.y, -q.z }; }

		localRotation[i] = rotInterpolators[i].eval(animationTime);
		auto lr = localRotation[i];
		/*
		if (i == 48)
		{
			livePlot.update(lr.w, lr.x, lr.y, lr.z);
			liveSamp.update(q.w, q.x, q.y, q.z);
			L++;
			/*
			double p = q.w * lr.w
				+ q.x * lr.x
				+ q.y * lr.y
				+ q.z * lr.z;
			if (std::abs(std::fmod(p, 1.0f)) < 0.0010) { p = std::round(p); }
			double d_theta = std::acos(2 * p*p - 1);
			assert(d_theta == d_theta); //NaN check
			assert(d_theta < 0.5);
			
		
		}
		*/

		//localRotation[i] = glm::quat(q.w, q.x, q.y, q.z);
		


		//assert(d_theta < 0.5);
		//localRotation[i] = rotInterpolators[i].eval(animationTime); //todo 0-1
		//localRotation[i] = glm::quat(lt.first.w, lt.first.x, lt.first.y, lt.first.z);

	}

	for (size_t i = 0; i < posInterpolators.size(); i++)
	{
		localPosition[i] = posInterpolators[i].eval(animationTime);
		auto lt = localPosition[i];
		auto v = AnimationStuff::makeAnimationMatrix(s.assimpAnimChannel[i], AnimationTime).second; //local animation transformation
		//localPosition[i] = glm::vec3(lt.second.x, lt.second.y, lt.second.z);

		if (v.x != lt.x)
		{
			localPosition[i] = posInterpolators[i].eval(animationTime);
		}

		if (i == 1)
		{
			livePlot.update(lt.x, lt.y, lt.z);
			liveSamp.update(v.x, v.y, v.z);
			L++;
		}
	}

	/*
	for (size_t i = 0; i < rotInterpolators.size(); i++)
	{
		localRotation[i] = rotInterpolators[i].eval(animationTime); //todo 0-1
	}

	for (size_t i = 0; i < posInterpolators.size(); i++)
	{
		localPosition[i] = posInterpolators[i].eval(animationTime);			
	}
	*/
}


SkeletonAnimation::quatKey SkeletonAnimation::bitsToKey(AnimationKeys::qbits bits)
{
	quatKey key;
	glm::quat & q = key.rot;
	float & t = key.time;

	t = (static_cast<float>(bits.time) / 0xFFFFF) * animationData.animationDuration;

	float component_range = 1 / std::sqrt(2.0f);
	auto dequantize20bcomponent = [](uint32_t comp, float component_range)->float {
		return (static_cast<float>(comp) / 0xFFFFF) * component_range;
	};
	float c0, c1, c2, restoredcomponent;
	c0 = dequantize20bcomponent(bits.x, component_range)
		* std::pow(-1.0f, static_cast<float>(bits.c0sign));
	c1 = dequantize20bcomponent((bits.y0 << 10) | bits.y1, component_range)
		* std::pow(-1.0f, static_cast<float>(bits.c1sign));
	c2 = dequantize20bcomponent(bits.z, component_range)
		* std::pow(-1.0f, static_cast<float>(bits.c2sign));
	restoredcomponent = std::sqrt(1 - c0 * c0 - c1 * c1 - c2 * c2)
		* std::pow(-1.0f, static_cast<float>(bits.nullsign));

	uint8_t nullcomponent = bits.nullc;
	if (nullcomponent == 0b00)
		q = glm::quat(restoredcomponent, c0, c1, c2);
	else if (nullcomponent == 0b01)
		q = glm::quat(c0, restoredcomponent, c1, c2);
	else if (nullcomponent == 0b10)
		q = glm::quat(c0, c1, restoredcomponent, c2);
	else {
		assert(nullcomponent == 0b11);
		q = glm::quat(c0, c1, c2, restoredcomponent);
	}
	key.channel = bits.channel;
	return key;
}

float SkeletonAnimation::dequantizeTime20bits(unsigned int time)
{
	return (static_cast<float>(time) / 0xFFFFF) * animationData.animationDuration;
}

SkeletonAnimation::vecKey SkeletonAnimation::bitsToKey(AnimationKeys::vbits bits)
{
	vecKey key;
	glm::vec3& v = key.pos;
	float & t = key.time;

	t = dequantizeTime20bits(bits.time);
	
	float component_range = 300.0f;
	auto dequantize21bcomponent = [](uint32_t comp, float component_range)->float {
		return (static_cast<float>(comp) / 0x1FFFFF) * component_range;
	};
	float c0, c1, c2;
	c0 = dequantize21bcomponent(bits.x, component_range)
		* std::pow(-1.0f, static_cast<float>(bits.c0sign));
	c1 = dequantize21bcomponent((bits.y0 << 11) | bits.y1, component_range)
		* std::pow(-1.0f, static_cast<float>(bits.c1sign));
	c2 = dequantize21bcomponent(bits.z, component_range)
		* std::pow(-1.0f, static_cast<float>(bits.c2sign));
	v = glm::vec3(c0, c1, c2);
	//v = glm::vec3(0, 0, 0);
	key.channel = bits.channel;
	return key;
}

AnimationKeys::qbits SkeletonAnimation::getRotData()
{
	latestRotData = animationData.rotData[rotDataIndex];
	return latestRotData;
}

AnimationKeys::qbits SkeletonAnimation::getNextRotData()
{
	++rotDataIndex;
	return getRotData();
}

AnimationKeys::vbits SkeletonAnimation::getPosData()
{
	latestPosData = animationData.posData[posDataIndex];
	return latestPosData;
}

AnimationKeys::vbits SkeletonAnimation::getNextPosData()
{
	++posDataIndex;
	return getPosData();
}


void SkeletonAnimation::updateEvaluators(float deltaTime)
{
	if (animationTime + deltaTime >= (float)animationData.animationDuration)
	{
		initiateEvaluators();

	}
	animationTime = std::fmod((animationTime + deltaTime), (float)animationData.animationDuration);
	   	 			
	auto channel = latestPosData.channel;
	while (posKeys[channel][posKeyRingIndex[channel] + 2].time < animationTime && posDataIndex < animationData.posKeyCount) {
		
		// insert new key
		posKeys[channel][posKeyRingIndex[channel] + 0] = bitsToKey(latestPosData);
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
	//auto & rotKeys[channel] = rotKeys[channel];
	//ring = rotKeyRingIndex[channel];
	
	while (rotKeys[channel][rotKeyRingIndex[channel] + 2].time < animationTime && rotDataIndex < animationData.rotKeyCount) {
		// insert new key
	
		rotKeys[channel][rotKeyRingIndex[channel] + 0] = bitsToKey(latestRotData);
		rotKeys[channel][rotKeyRingIndex[channel] + 0].rotKeyID = rotDataIndex;
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

void SkeletonAnimation::initiateEvaluators()
{
	posDataIndex = 0;
	rotDataIndex = 0;
	for (unsigned int i = 0; i < rotKeys.size(); i++)
	{
		auto & ring = rotKeyRingIndex[i];
		auto & rk = rotKeys[i];
		for (size_t j = 0; j < 4; j++)
		{	
			rk[ring + j] = bitsToKey(getRotData());
			rk[ring + j].rotKeyID = 0;
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
			pk[ring + j] = bitsToKey(getPosData());
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

SkeletonAnimation::SkeletonAnimation(int jointCount, AnimationKeys& animationData)
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


SkeletonAnimation::~SkeletonAnimation()
{
}
