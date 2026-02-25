#include "VMDPlayer.h"


VMDPlayer::VMDPlayer(PMXModel &model, VMDFile &vfile):
	model(model), vfile(vfile)
{
	// Assign each bone name with VMDMotion
	for (const auto& item : vfile.m_motions)
	{
		// Also find global max frame
		globalMaxFrame = std::max(globalMaxFrame, item.m_frame);
		boneNameToMotion[Utils::sjis_to_utf8(item.m_boneName)].push_back(item);
	}

	// Sort VMDMotion based on frame number
	for (const auto& item : vfile.m_motions)
	{
		std::sort(
			boneNameToMotion[Utils::sjis_to_utf8(item.m_boneName)].begin(),
			boneNameToMotion[Utils::sjis_to_utf8(item.m_boneName)].end(),
			[](const VMDMotion& a, const VMDMotion& b)
			{
				return a.m_frame < b.m_frame;
			}
		);
	}

	// Interpolate frames of each bone
	for (const auto& [boneName, sortedMotions] : boneNameToMotion)
	{
		int maxFrames = sortedMotions.back().m_frame;
		size_t keyIndex = 0;

		for (size_t i = 0; i < maxFrames; i++)
		{

			if (i == sortedMotions[keyIndex].m_frame)
			{
				testVMD.push_back(sortedMotions[keyIndex]);
			}
			else
			{
				const VMDMotion& start = sortedMotions[keyIndex];
				const VMDMotion& end = sortedMotions[keyIndex + 1];

				// Calculate progress (0.0 to 1.0)
				float t = (float)(i - start.m_frame) / (float)(end.m_frame - start.m_frame);

				VMDMotion interpolated;
				interpolated.m_frame = i;

				// Linear Interpolation for Translation
				interpolated.m_translate = glm::mix(start.m_translate, end.m_translate, t);

				// Spherical Interpolation for Rotation (Quaternions)
				interpolated.m_quaternion = glm::slerp(start.m_quaternion, end.m_quaternion, t);

				boneNameToFrames[boneName].push_back(interpolated);
			}

			// Advance the keyframe window if we've passed the "end" of the current one
			if 
			(
				keyIndex < sortedMotions.size() - 1 &&
				i >= sortedMotions[keyIndex + 1].m_frame - 1
			)
			{
				keyIndex++;
			}
		}
	}

	//std::cout << "Maximum frames: " << globalMaxFrame << std::endl;
}

void VMDPlayer::Play()
{

	for (auto& [boneName, motions] : boneNameToFrames)
	{
		if (model.boneNameToIndex.find(boneName) == model.boneNameToIndex.end())
		{
			continue;
		}

		if (index >= motions.size())
		{
			continue;
		}

		uint32_t boneIndex = model.boneNameToIndex[boneName];
		VMDMotion &motion = motions[index];
		model.bones[boneIndex].position = motion.m_translate;
		model.bones[boneIndex].setQuadRotation(motion.m_quaternion);
	}

	if ((index + 1) == globalMaxFrame)
	{
		index = 0;
	}
	else
	{
		index++;
	}

}
