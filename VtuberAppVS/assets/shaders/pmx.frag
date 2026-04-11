#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;

uniform vec4 diffuseColor;	// From PMX Diffuse color
uniform vec3 ambientColor;  // From PMX Ambient color
uniform vec3 specularColor; // From PMX Specular color
uniform float shininess;    // From PMX Specularity (Specular strength)

uniform sampler2D myTexture;

uniform sampler2D envTexture;
uniform int envMode;

uniform vec4 lightColor;
uniform vec3 lightPos;

uniform vec3 camPos;


void main()
{
	// ambient lighting 
	vec3 ambient = ambientColor * 0.20f;

	// diffuse lighting
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightPos - crntPos);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	// specular lighting
	// float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), shininess);
	vec3 specular = specAmount * specularColor;

	vec3 lighting = (ambient + diffuse) * lightColor.rgb;

	// Base Texture
	vec4 baseColor = texture(myTexture, texCoord) * diffuseColor;

	if(baseColor.a < 0.1) {
        discard;
    }

	// Env Texture
	vec2 envCoord = Normal.xy * 0.5 + 0.5;
    vec3 envColor = texture(envTexture, envCoord).rgb;

	if (envMode == 1) { 
        // Multiply (Common for SPH)
        baseColor.rgb *= envColor;
    } 
    else if (envMode == 2) { 
        // Add (Common for SPA - shiny/metallic)
        baseColor.rgb += envColor;
    }

	FragColor = baseColor;
	// FragColor = baseColor  + vec4(specular * lightColor.rgb, 0.0);
	// FragColor = vec4(lighting, 1.0) * baseColor;
	// FragColor = vec4(lighting, 1.0) * baseColor  + vec4(specular * lightColor.rgb, 0.0);
	// FragColor = texture(myTexture, texCoord) * lightColor * (diffuseColor * diffuse);
	// FragColor = texture(myTexture, texCoord) * diffuseColor;
}
