#pragma once

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>



namespace IK
{
	struct axisAngle3D
	{
		glm::vec3 axis;
		float angle;
	};


	glm::vec3 rotate3DVector(glm::vec3 &v, glm::vec3 &axis, float angle);
	glm::vec3 rotatePointAround3D(glm::vec3 &p, glm::vec3 &pivot, glm::vec3 &axis, float &angle);
	axisAngle3D solveAxisAngle3D(glm::vec3 &joint, glm::vec3 &effector, glm::vec3 &target);
	glm::quat solveJointCCD(glm::vec3 &joint, glm::vec3 &effector, glm::vec3 &target);
	void solve3DJointCCD(std::vector<glm::vec3>& joints, glm::vec3& effector, glm::vec3& target, int iterations = 10);
}