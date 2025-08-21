#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;

uniform float uOffset;
uniform vec3 selectionColor = vec3(1.0f);
uniform sampler2D myTexture;


void main()
{
   // FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
   // FragColor = vec4(color, 1.0f);
   FragColor = texture(myTexture, texCoord + vec2(uOffset, 0)) * vec4(selectionColor, 1.0f);
}