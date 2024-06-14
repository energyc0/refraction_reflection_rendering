#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 colors[];
layout(location = 1) in vec3 geomPos[];
layout(location = 2) in vec3 geomNorm[];

layout (location=0) out vec3 fragColor;
layout (location=1) out vec3 barycoords;
layout (location=2) out vec3 fragPos;
layout(location = 3) out vec3 fragNorm;
const vec3 bc[3] = vec3[]
	(
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, 0.0, 1.0)
	);

void main(){
    for(int i = 0; i < gl_in.length();i++){
        gl_Position = gl_in[i].gl_Position;
        fragColor = colors[i];
        barycoords = bc[i];
        fragPos = geomPos[i];
        fragNorm = geomNorm[i];
        EmitVertex();
    }
    EndPrimitive();
}