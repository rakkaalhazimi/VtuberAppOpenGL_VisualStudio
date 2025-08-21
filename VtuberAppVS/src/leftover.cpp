
// void printBoneTree(
//   std::ofstream &file,
//   const std::vector<PMXBone> &bones,
//   const std::unordered_map<int, std::vector<int>> &boneChildren,
//   int boneIndex, int depth = 0) 
// {
//   const auto& bone = bones[boneIndex];
//   file << std::string(depth * 2, ' ') << "- " << bone.nameLocal << " (index: " << boneIndex << ")\n";

//   auto it = boneChildren.find(boneIndex);
//   if (it != boneChildren.end()) 
//   {
//     for (int childIndex : it->second) 
//     {
//       printBoneTree(file, bones, boneChildren, childIndex, depth + 1);
//     }
//   }
// }

// Force NVIDIA GPU on laptops with Optimus
// #include <minwindef.h>
// extern "C" {
//     __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
// }


// std::set<int> usedBones;
// for (const PMXVertex& v : pmxFile.vertices) 
// {
//     for (int i = 0; i < v.boneIndices.size(); ++i) 
//     {
//         usedBones.insert(v.boneIndices[i]);
//     }
// }

// std::ofstream usedBonesFile {"used-bones.txt"};
// for (int item: usedBones)
// {
//   usedBonesFile << item << std::endl;
// }

// const GLubyte* renderer = glGetString(GL_RENDERER); // e.g., "NVIDIA GeForce GTX 1660"
// const GLubyte* vendor   = glGetString(GL_VENDOR);   // e.g., "NVIDIA Corporation"
// const GLubyte* version  = glGetString(GL_VERSION);  // e.g., "4.6.0 NVIDIA 550.78"
// const GLubyte* glsl     = glGetString(GL_SHADING_LANGUAGE_VERSION);

// std::cout << "GPU Vendor:   " << vendor << std::endl;
// std::cout << "GPU Renderer: " << renderer << std::endl;
// std::cout << "OpenGL Version: " << version << std::endl;
// std::cout << "GLSL Version:   " << glsl << std::endl;

// void printHex(const std::string &str) {
//   for (unsigned char c : str) {
//     std::cout 
//       << std::hex 
//       << std::uppercase 
//       << std::setfill('0') 
//       << std::setw(2)
//       << static_cast<int>(c) << " ";
//   }
//   std::cout << std::dec << std::endl; // Reset to decimal
// }

// void traverseBones(
//   std::unordered_map<int, std::vector<int>> &boneChildren,
//   int boneIndex,
//   int depth = 0
// )
// {
//   std::cout << std::string(depth * 2, ' ') << '-' << " Bone index: " << boneIndex << std::endl;
  
//   if (boneChildren.find(boneIndex) != boneChildren.end())
//   {
//     for (int childIndex: boneChildren[boneIndex])
//     {
//       traverseBones(boneChildren, childIndex, depth + 1);
//     }
//   }
// }

// void traverseIndexBone(
//   int boneIndex,
//   std::set<int> &usedBoneIndices,
//   std::unordered_map<int, std::vector<int>> &boneChildren
// )
// {
//   usedBoneIndices.insert(boneIndex);
//   if (boneChildren.find(boneIndex) != boneChildren.end())
//   {
//     for (int childIndex: boneChildren[boneIndex])
//     {
//       traverseIndexBone(childIndex, usedBoneIndices, boneChildren);
//     }
//   }
// }


// bool isHelperBoneName(std::string name)
// {
//   static const std::vector<std::string> helpers = {
//     "操作中心", "全ての親", "センター", "グルーブ", "視線", "先",
//     "ＩＫ", "IK", "ik", "Ik", "toeIK",
//     "Parent", "親", "All Parent", "Center", "Groove", "Look", "HeadLook",
//     "ダミー", "マーカー",
//     "物理", "剛体", "joint", "Joint"
//   };
  
//   for (const auto& keyword : helpers) {
//     if (name.find(keyword) != std::string::npos)
//     {
//       return true;
//     }
//   }
  
  
//   return false;
// }