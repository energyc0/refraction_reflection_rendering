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
layout(location = 2) in vec3 norm;

layout(location = 0) out vec3 geometryColor;
layout(location = 1) out vec3 geomPos;
layout(location = 2) out vec3 geomNorm;

void main(){
    geomPos = vec3(ubo.transformData.model * vec4(position,1.0));
    geomNorm = norm;
    gl_Position =  ubo.transformData.perspective * ubo.transformData.view * vec4(geomPos,1.0);
    geometryColor = color;
}