#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 LightingColor;  // Receive lighting color from vertex shader
in float FogFactor; 

uniform sampler2D texture_diffuse1;
uniform vec3 fogColor;

void main()
{
    vec3 textureColor = vec3(texture(texture_diffuse1, TexCoords));
    vec3 result = LightingColor * textureColor;
    vec3 finalColor = mix(fogColor, result, FogFactor);
    FragColor = vec4(finalColor, 1.0);
}
