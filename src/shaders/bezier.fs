#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;  

uniform vec3 objectColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 viewPos;

// Spotlight 1
uniform vec3 spotLight1Pos;
uniform vec3 spotLight1Dir;
uniform vec3 spotLightColor;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;

// Spotlight 2
uniform vec3 spotLight2Pos;
uniform vec3 spotLight2Dir;

// Plane Spotlight
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
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    // Calculate the distance between the light source and the fragment
    float distance = length(lightPos - fragPos);

    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

    vec3 ambient = 0.05 * lightColor;
    float diff = max(dot(normal, lightVec), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 reflectDir = reflect(-lightVec, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * lightColor;
    
    return (ambient + intensity * (diffuse + specular)) * attenuation;
}

void main()
{
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

    lighting += spot1 + spot2 + planeSpot + underPlaneSpot;

    vec3 result = lighting * objectColor;
    float distance = length(viewPos - FragPos);
    float fogFactor = CalculateFog(distance, fogIntensity);
    vec3 finalColor = mix(fogColor, result, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}
