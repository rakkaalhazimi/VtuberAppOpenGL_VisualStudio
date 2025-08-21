#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;
layout (location = 3) in ivec4 boneIndices;
layout (location = 4) in vec4 boneWeights;



out vec2 texCoord;

uniform mat4 camMatrix;


void main()
{
  texCoord = aTex;
  gl_Position = camMatrix * vec4(aPos, 1.0);
}