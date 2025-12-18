#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 worldPos;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform vec2 scale;

void main()
{
    vec3 vertexWorld = worldPos 
        + cameraRight * aPos.x * scale.x
        + cameraUp * aPos.y * scale.y;
    gl_Position = projection * view * vec4(vertexWorld, 1.0);
}