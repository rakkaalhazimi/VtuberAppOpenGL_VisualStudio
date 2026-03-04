#include "InverseKinematics.h"



glm::vec3 IK::rotate3DVector(glm::vec3& v, glm::vec3& axis, float angle)
{
	axis = glm::normalize(axis);
	return (
		v * glm::cos(angle)
		+ glm::cross(axis, v) * glm::sin(angle)
		+ axis * glm::dot(axis, v) * (1 - glm::cos(angle))
	);
}


glm::vec3 IK::rotatePointAround3D(
	glm::vec3& p, glm::vec3& pivot, glm::vec3& axis, float& angle
)
{
	glm::vec3 diff = p - pivot;
	return pivot + rotate3DVector(diff, axis, angle);
}

// Solve the angle with single axis
IK::axisAngle3D IK::solveSingleAxisAngle2D(
	glm::vec3& axis, glm::vec3& joint, glm::vec3& effector, glm::vec3& target
)
{
	glm::vec3 v1 = glm::normalize(effector - joint);
	glm::vec3 v2 = glm::normalize(target - joint);

	// Remove X component (project onto plane perpendicular to axis)
	v1 -= axis * glm::dot(v1, axis);
	v2 -= axis * glm::dot(v2, axis);

	v1 = glm::normalize(v1);
	v2 = glm::normalize(v2);

	float dot = glm::clamp(glm::dot(v1, v2), -1.0f, 1.0f);
	float angle = glm::acos(dot);
	float direction = glm::dot(glm::cross(v1, v2), axis);
	// Negative dot product means
	// The rotation is around the NEGATIVE X-axis
	if (direction < 0)
	{
		axis *= -1;
	}

	return { axis, angle };
}

IK::axisAngle3D IK::solveAxisAngle3D(
	glm::vec3& joint, glm::vec3& effector, glm::vec3& target
)
{
	glm::vec3 v1 = glm::normalize(effector - joint);
	glm::vec3 v2 = glm::normalize(target - joint);
	//std::cout << "v1: " << v1.x << std::endl;

	glm::vec3 axis = glm::cross(v1, v2);
	float axisLen = glm::length(axis);
	
	if (axisLen < 1e-6f)
	{
		return { glm::vec3(0.0f), 0.0f };
	}

	float cosine = glm::dot(v1, v2);
	// 180-degree flip case
	if (cosine < -0.9999f)
	{
		return { glm::vec3(0.0f), 0.0f };
	}

	axis /= axisLen;
	float angle = glm::acos(glm::clamp(cosine, -1.0f, 1.0f));
	//angle = glm::clamp(angle, -0.314f, 0.314f);
	return { axis, angle };
}


glm::quat IK::solveJointCCD(glm::vec3& joint, glm::vec3& effector, glm::vec3& target)
{
	axisAngle3D result = solveAxisAngle3D(joint, effector, target);
	if (glm::length(result.axis) <= 1e-6 || result.angle <= 1e-6)
	{
		return glm::quat(1, 0, 0, 0);
	}
	result.axis = glm::normalize(result.axis);
	glm::quat deltaRotation = glm::angleAxis(result.angle, result.axis);
	return deltaRotation;
}


void IK::solve3DJointCCD(
	std::vector<glm::vec3>& joints, glm::vec3& effector, glm::vec3& target, int iterations
)
{
	glm::vec3 currentEffector = effector;
	for (int i = 0; i < iterations; i++)
	{
		glm::vec3 axis;
		float angle;
		
		for (int j = joints.size() - 1; j >= 0; --j)
		{
			axisAngle3D result = solveAxisAngle3D(joints[j], currentEffector, target);
			
			std::cout << "Joint " << j << ": " << joints[j].x << " " << joints[j].y << " " << joints[j].z << std::endl;
			std::cout << "Axis length: " << glm::length(result.axis) << std::endl;
			std::cout << "Angle: " << result.angle << std::endl;

			if (glm::length(result.axis) <= 1e-6 || result.angle <= 1e-6)
			{
				continue;
			}

			for (int k = j + 1; k < joints.size(); ++k)
			{
				joints[k] = rotatePointAround3D(
					joints[k],
					joints[j],
					result.axis,
					result.angle
				);
			}

			currentEffector = rotatePointAround3D(currentEffector, joints[j], result.axis, result.angle);

		}
		std::cout << "Effector: "
			<< currentEffector.x << " "
			<< currentEffector.y << " "
			<< currentEffector.z << " "
			<< std::endl;

		std::cout << "Target: "
			<< target.x << " "
			<< target.y << " "
			<< target.z << " "
			<< std::endl;

		std::cout << std::endl;
	}
}
