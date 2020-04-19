#pragma once
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

struct CubicPolynomial {
	float c0, c1, c2, c3;

	float eval(float t);

	void initCubicPoly(float p_0, float p_1, float dp_0, float dp_1);

	void initNonuniformCatmullRom(float x0, float x1, float x2, float x3, 
		float dt0, float dt1, float dt2);
};

struct QuaternionInterpolation {
	float t1;
	float t2;
	CubicPolynomial x;
	CubicPolynomial y;
	CubicPolynomial z;
	CubicPolynomial w;


	glm::quat eval(float t);
	
	QuaternionInterpolation(const glm::quat& p0, const glm::quat& p1, 
		const glm::quat& p2, const glm::quat& p3, float t1, float t2);

	QuaternionInterpolation();
};

struct Vector3Interpolation {
	float t1;
	float t2;
	CubicPolynomial x;
	CubicPolynomial y;
	CubicPolynomial z;
	
	glm::vec3 eval(float t);

	Vector3Interpolation(const glm::vec3& p0, const glm::vec3& p1,
		const glm::vec3& p2, const glm::vec3& p3, float t1, float t2);

	Vector3Interpolation();

};