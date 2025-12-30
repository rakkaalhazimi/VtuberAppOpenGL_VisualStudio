#include "VMDFile.h"


VMDFile::VMDFile(std::string filepath)
{
	std::ifstream file = Utils::openFile(filepath);
	//std::cout << "Opened file: " << filepath << std::endl;

	// File Size (check the end of pos then go back to initial pos)
	std::streampos originalPos = file.tellg();
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(originalPos);

	// Header and Model Name
	Utils::readBinary(file, m_header);
	std::string header = Utils::sjis_to_utf8(m_header.m_header);
	std::string modelName = Utils::sjis_to_utf8(m_header.m_modelName);
	std::cout << "Header: " << header << std::endl;
	std::cout << "Model Name: " << modelName << std::endl;

	// Motion
	uint32_t motionCount = 0;
	Utils::readBinary(file, motionCount);
	m_motions.resize(motionCount);
	for (auto& motion: m_motions)
	{
		Utils::readBinary(file, motion.m_boneName);
		Utils::readBinary(file, motion.m_frame);
		Utils::readBinary(file, motion.m_translate);
		Utils::readBinary(file, motion.m_quaternion);
		Utils::readBinary(file, motion.m_interpolation);
	}

	// Blend Shape
	if (file.tellg() < fileSize)
	{
		uint32_t blendShapeCount = 0;
		Utils::readBinary(file, blendShapeCount);
		m_morphs.resize(blendShapeCount);
		for (auto& morph : m_morphs)
		{
			Utils::readBinary(file, morph.m_blendShapeName);
			Utils::readBinary(file, morph.m_frame);
			Utils::readBinary(file, morph.m_weight);
		}
	}

	// Camera
	if (file.tellg() < fileSize)
	{
		uint32_t cameraCount = 0;
		Utils::readBinary(file, cameraCount);
		m_cameras.resize(cameraCount);
		for (auto& camera : m_cameras)
		{
			Utils::readBinary(file, camera.m_frame);
			Utils::readBinary(file, camera.m_distance);
			Utils::readBinary(file, camera.m_interest);
			Utils::readBinary(file, camera.m_rotate);
			Utils::readBinary(file, camera.m_interpolation);
			Utils::readBinary(file, camera.m_viewAngle);
			Utils::readBinary(file, camera.m_isPerspective);
		}
	}

	// Light
	if (file.tellg() < fileSize)
	{
		uint32_t lightCount = 0;
		Utils::readBinary(file, lightCount);
		m_lights.resize(lightCount);
		for (auto& light : m_lights)
		{
			Utils::readBinary(file, light.m_frame);
			Utils::readBinary(file, light.m_color);
			Utils::readBinary(file, light.m_position);
		}
	}

	// Shadow
	uint32_t shadowCount = 0;
	Utils::readBinary(file, shadowCount);
	m_shadows.resize(shadowCount);
	for (auto& shadow : m_shadows)
	{
		Utils::readBinary(file, shadow.m_frame);
		Utils::readBinary(file, shadow.m_shadowType);
		Utils::readBinary(file, shadow.m_distance);
	}

	//Utils::readBinary(file, m_motions);

}