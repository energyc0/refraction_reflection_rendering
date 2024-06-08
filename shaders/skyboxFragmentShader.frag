#version 450

layout(binding = 1) uniform samplerCube cubemap;

layout(location = 0) in vec3 texCoord;
layout(location = 0) out vec4 fragColor;

void main(){
    fragColor = texture(cubemap, texCoord);
}