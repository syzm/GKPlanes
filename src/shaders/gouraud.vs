#version 410 core
layout (location = 0) in vec3 aPos;        // Vertex position
layout (location = 1) in vec3 aNormal;     // Vertex normal
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 LightingColor;  // Send the calculated lighting color to the fragment shader
out float FogFactor;     // Send the fog factor to the fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightDirection; 
uniform vec3 lightColor;     
uniform vec3 viewPos;
uniform vec3 fogColor;
uniform float fogIntensity;

// Spotlight 1
uniform vec3 spotLight1Pos;
uniform vec3 spotLight1Dir;
uniform vec3 spotLightColor;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;

// Spotlight 2
uniform vec3 spotLight2Pos;
uniform vec3 spotLight2Dir;

// Plane Spotlight (Front)
uniform vec3 planeSpotLightPos;
uniform vec3 planeSpotLightDir;
uniform vec3 planeSpotLightColor;
uniform float planeSpotLightCutOff;
uniform float planeSpotLightOuterCutOff;

// Under-Plane Spotlight
uniform vec3 underPlaneSpotLightPos;
uniform vec3 underPlaneSpotLightDir;
uniform vec3 underPlaneSpotLightColor;
uniform float underPlaneSpotLightCutOff;
uniform float underPlaneSpotLightOuterCutOff;

// Attenuation parameters
uniform float constant;
uniform float linear;
uniform float quadratic;

// Function to calculate fog factor based on distance
float CalculateFog(float distance, float fogIntensity)
{
    float fogFactor = exp(-distance * fogIntensity / 3.0);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    return fogFactor;
}

// Function to calculate spotlight effect with attenuation
vec3 CalculateSpotlight(vec3 lightPos, vec3 lightDir, vec3 lightColor, vec3 normal, vec3 fragPos, vec3 viewDir, float cutOff, float outerCutOff)
{
    vec3 lightVec = normalize(lightPos - fragPos);
    float theta = dot(lightVec, normalize(-lightDir)); // Angle between light direction and fragment direction
    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0); // Smooth edge

    // Calculate the distance between the light source and the fragment
    float distance = length(lightPos - fragPos);

    // Calculate attenuation based on the distance
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    // Ambient, diffuse, and specular calculations for spotlight
    vec3 ambient = 0.05 * lightColor;
    float diff = max(dot(normal, lightVec), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 reflectDir = reflect(-lightVec, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * lightColor;
    
    // Apply attenuation to the spotlight effects
    return (ambient + intensity * (diffuse + specular)) * attenuation;
}

void main()
{
    TexCoords = aTexCoords;
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));  
    vec3 Normal = mat3(transpose(inverse(model))) * aNormal; 

    // directional light
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);

    vec3 ambient = 0.1 * lightColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * lightColor; 

    // Combine the main lighting effects
    vec3 lighting = ambient + diffuse + specular;

    // Calculate spotlight effects with attenuation
    vec3 spot1 = CalculateSpotlight(spotLight1Pos, spotLight1Dir, spotLightColor, norm, FragPos, viewDir, spotLightCutOff, spotLightOuterCutOff);
    vec3 spot2 = CalculateSpotlight(spotLight2Pos, spotLight2Dir, spotLightColor, norm, FragPos, viewDir, spotLightCutOff, spotLightOuterCutOff);
    vec3 planeSpot = CalculateSpotlight(planeSpotLightPos, planeSpotLightDir, planeSpotLightColor, norm, FragPos, viewDir, planeSpotLightCutOff, planeSpotLightOuterCutOff);
    vec3 underPlaneSpot = CalculateSpotlight(underPlaneSpotLightPos, underPlaneSpotLightDir, underPlaneSpotLightColor, norm, FragPos, viewDir, underPlaneSpotLightCutOff, underPlaneSpotLightOuterCutOff);

    // Combine all lighting effects
    LightingColor = lighting + spot1 + spot2 + planeSpot + underPlaneSpot;

    // Calculate the distance from the fragment to the camera
    float distance = length(viewPos - FragPos);

    // Calculate the fog factor
    FogFactor = CalculateFog(distance, fogIntensity);

    // Set the vertex position in clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
