#include "spline.h"


float CubicPolynomial::eval(float t)
{
	float t_squared = t * t;
	float t_cubed = t * t * t;
	return c0 + (c1 * t) + (c2 * t_squared) + (c3 * t_cubed);
}

void CubicPolynomial::initCubicPoly(float p_0, float p_1, float dp_0, float dp_1)
{
	c0 = p_0;
	c1 = dp_0;
	c2 = (-3.0f * p_0) + (3 * p_1) - (2 * dp_0) - dp_1;
	c3 = (2 * p_0) - (2 * p_1) + dp_0 + dp_1;
}

void CubicPolynomial::initNonuniformCatmullRom(float x0, float x1, float x2, float x3, float dt0, float dt1, float dt2)
{
	// compute tangents when parameterized in [t1,t2]
	float t1 = (x1 - x0) / dt0 - (x2 - x0) / (dt0 + dt1) + (x2 - x1) / dt1;
	float t2 = (x2 - x1) / dt1 - (x3 - x1) / (dt1 + dt2) + (x3 - x2) / dt2;

	// rescale tangents for parametrization in [0,1]
	t1 *= dt1;
	t2 *= dt1;

	initCubicPoly(x1, x2, t1, t2);
}

glm::vec3 Vector3Interpolation::eval(float time)
{
	assert(t2 - t1 != 0);
	float t = (time - t1) / (t2 - t1); //0-1
	assert(t >= 0 && t <= 1);
	return glm::vec3((float)x.eval(t), (float)y.eval(t), (float)z.eval(t));
}

Vector3Interpolation::Vector3Interpolation(const glm::vec3& p0, const glm::vec3& p1,
	const glm::vec3& p2, const glm::vec3& p3, float t1, float t2)
{
	this->t1 = t1;
	this->t2 = t2;

	auto distSquared = [](const glm::vec3& p, const glm::vec3& q) {
		float dx = q.x - p.x;
		float dy = q.y - p.y;
		float dz = q.z - p.z;
		return dx * dx + dy * dy + dz * dz;
	};
	float dt0 = pow(distSquared(p0, p1), 0.25f);
	float dt1 = pow(distSquared(p1, p2), 0.25f);
	float dt2 = pow(distSquared(p2, p3), 0.25f);

	// safety check for repeated points
	if (dt1 < 1e-4f)    dt1 = 1.0f;
	if (dt0 < 1e-4f)    dt0 = dt1;
	if (dt2 < 1e-4f)    dt2 = dt1;

	x.initNonuniformCatmullRom(p0.x, p1.x, p2.x, p3.x, dt0, dt1, dt2);
	y.initNonuniformCatmullRom(p0.y, p1.y, p2.y, p3.y, dt0, dt1, dt2);
	z.initNonuniformCatmullRom(p0.z, p1.z, p2.z, p3.z, dt0, dt1, dt2);
}

Vector3Interpolation::Vector3Interpolation()
{

}

glm::quat QuaternionInterpolation::eval(float time)
{
	assert(t2 - t1 != 0);
	float t = (time - t1) / (t2 - t1); //0-1
	assert(t >= 0 && t <= 1);
	return glm::quat((float)w.eval(t), (float)x.eval(t), (float)y.eval(t), (float)z.eval(t));
	//return glm::normalize(glm::quat((float)w.eval(t), (float)x.eval(t), (float)y.eval(t), (float)z.eval(t)));
}

QuaternionInterpolation::QuaternionInterpolation(const glm::quat& p0, const glm::quat& p1,
	const glm::quat& p2, const glm::quat& p3, float t1, float t2)
{
	this->t1 = t1;
	this->t2 = t2;

	auto distSquared = [](const glm::quat& p, const glm::quat& q) {
		float dx = q.x - p.x;
		float dy = q.y - p.y;
		float dz = q.z - p.z;
		float dw = q.w - p.w;
		return dx * dx + dy * dy + dz * dz + dw * dw;
	};
	float dt0 = pow(distSquared(p0, p1), 0.25f);
	float dt1 = pow(distSquared(p1, p2), 0.25f);
	float dt2 = pow(distSquared(p2, p3), 0.25f);

	// safety check for repeated points
	if (dt1 < 1e-4f)
		dt1 = 1.0f;
	if (dt0 < 1e-4f)
		dt0 = dt1;
	if (dt2 < 1e-4f)
		dt2 = dt1;

	x.initNonuniformCatmullRom(p0.x, p1.x, p2.x, p3.x, dt0, dt1, dt2);
	y.initNonuniformCatmullRom(p0.y, p1.y, p2.y, p3.y, dt0, dt1, dt2);
	z.initNonuniformCatmullRom(p0.z, p1.z, p2.z, p3.z, dt0, dt1, dt2);
	w.initNonuniformCatmullRom(p0.w, p1.w, p2.w, p3.w, dt0, dt1, dt2);
}

QuaternionInterpolation::QuaternionInterpolation() 
{

	this->t1 = 0;
	this->t2 = 1;
	x.initNonuniformCatmullRom(0,0,0,0,0,0,0);
	y.initNonuniformCatmullRom(0,0,0,0,0,0,0);
	z.initNonuniformCatmullRom(0,0,0,0,0,0,0);
	w.initNonuniformCatmullRom(0,0,0,0,0,0,0);
}