#version 410 core
out vec4 FragColor;

flat in vec3 FinalColor;

void main()
{
    FragColor = vec4(FinalColor, 1.0);
}
