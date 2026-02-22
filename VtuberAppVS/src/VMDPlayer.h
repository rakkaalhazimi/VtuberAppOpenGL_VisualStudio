#pragma once

#include "PMXModel.h"
#include "VMDFile.h"


class VMDPlayer
{
	private:
		PMXModel &model;
		VMDFile& vfile;

	public:
		VMDPlayer(PMXModel &model, VMDFile &vfile);
		void Play();
		void Pause();
		void Stop();
		~VMDPlayer() {};
};
