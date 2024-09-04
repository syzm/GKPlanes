#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;  

uniform sampler2D texture_diffuse1;

uniform vec3 lightDirection; // Directional light direction
uniform vec3 lightColor;     // Directional light color
uniform vec3 viewPos;        // Camera position

// Spotlight 1
uniform vec3 spotLight1Pos;
uniform vec3 spotLight1Dir;
uniform vec3 spotLightColor;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;

// Spotlight 2
uniform vec3 spotLight2Pos;
uniform vec3 spotLight2Dir;

// Plane Spotlight (Reflector)
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

uniform vec3 fogColor;
uniform float fogIntensity;

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

// Function to calculate spotlight effect
vec3 CalculateSpotlight(vec3 lightPos, vec3 lightDir, vec3 lightColor, vec3 normal, vec3 fragPos, vec3 viewDir, float cutOff, float outerCutOff, float constant, float linear, float quadratic)
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
    // Normalize vectors
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);

    // Directional light calculations
    vec3 ambient = 0.1 * lightColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * lightColor;

    // Combine directional lighting
    vec3 lighting = ambient + diffuse + specular;

    // Calculate spotlights
    vec3 spot1 = CalculateSpotlight(spotLight1Pos, spotLight1Dir, spotLightColor, norm, FragPos, viewDir, spotLightCutOff, spotLightOuterCutOff, constant, linear, quadratic);
    vec3 spot2 = CalculateSpotlight(spotLight2Pos, spotLight2Dir, spotLightColor, norm, FragPos, viewDir, spotLightCutOff, spotLightOuterCutOff, constant, linear, quadratic);
    vec3 planeSpot = CalculateSpotlight(planeSpotLightPos, planeSpotLightDir, planeSpotLightColor, norm, FragPos, viewDir, planeSpotLightCutOff, planeSpotLightOuterCutOff, constant, linear, quadratic);
    vec3 underPlaneSpot = CalculateSpotlight(underPlaneSpotLightPos, underPlaneSpotLightDir, underPlaneSpotLightColor, norm, FragPos, viewDir, underPlaneSpotLightCutOff, underPlaneSpotLightOuterCutOff, constant, linear, quadratic);

    // Add spotlight effects to the lighting
    lighting += spot1 + spot2 + planeSpot + underPlaneSpot;

    // Apply lighting to the texture color
    vec3 textureColor = vec3(texture(texture_diffuse1, TexCoords));
    vec3 result = lighting * textureColor;

    // Calculate the distance from the fragment to the camera
    float distance = length(viewPos - FragPos);

    // Calculate the fog factor using the custom function
    float fogFactor = CalculateFog(distance, fogIntensity);

    // Blend the object color with the fog color using the calculated fog factor
    vec3 finalColor = mix(fogColor, result, fogFactor);

    // Set the final fragment color
    FragColor = vec4(finalColor, 1.0);
}
