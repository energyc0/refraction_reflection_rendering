#version 460

layout(push_constant) uniform PushConstant{
	bool isWireframeShown;
}push;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 barycoords;

layout(location = 0) out vec4 outColor;

float edgeFactor(float thickness)
{
	vec3 a3 = smoothstep( vec3(0.0, 0.0, 0.0), fwidth(barycoords) * thickness, barycoords);
	return min( min( a3.x, a3.y ), a3.z );
}

void main()
{
	if(push.isWireframeShown){
		outColor = vec4(edgeFactor(1.0) * fragColor, 1.0 );
	}else{
		outColor = vec4( fragColor, 1.0 );
	}
}