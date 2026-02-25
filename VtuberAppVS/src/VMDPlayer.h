#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "PMXModel.h"
#include "VMDFile.h"


class VMDPlayer
{
	private:
		PMXModel &model;
		VMDFile &vfile;
		int index = 0;
		std::vector<VMDMotion> testVMD;

	public:
		uint32_t globalMaxFrame = 0;
		std::unordered_map<
			std::string, std::vector<VMDMotion>> boneNameToMotion;
		std::unordered_map<
			std::string, std::vector<VMDMotion>> boneNameToFrames;

		VMDPlayer(PMXModel &model, VMDFile &vfile);
		void Play();
		void Pause();
		void Stop();
		~VMDPlayer() {};
};
