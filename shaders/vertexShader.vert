#version 450

struct TransformData{
    mat4 model;
    mat4 view;
    mat4 perspective;
};

layout(binding = 0) uniform UniformBuffer{
    TransformData transformData;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 geometryColor;

void main(){
    gl_Position =  ubo.transformData.perspective * ubo.transformData.view * ubo.transformData.model * vec4(position,1.0);
    geometryColor = color;
}