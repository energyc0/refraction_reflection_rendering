#version 460

layout(push_constant) uniform PushConstant{
	bool isWireframeShown;
	bool isReflectionEnabled;
	bool isRefractionEnabled;
	vec3 cameraPos;
}push;
layout(binding = 1) uniform samplerCube cubemap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 barycoords;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 fragNorm;

layout(location = 0) out vec4 outColor;

float edgeFactor(float thickness)
{
	vec3 a3 = smoothstep( vec3(0.0, 0.0, 0.0), fwidth(barycoords) * thickness, barycoords);
	return min( min( a3.x, a3.y ), a3.z );
}

void main()
{
	if(push.isRefractionEnabled){
		float ratio = 1.0 / 1.52; // mesh is made of glass
		vec3 dist = normalize(fragPos - push.cameraPos);
		vec3 refractioned = refract(dist, fragNorm, ratio);
		outColor = vec4(texture(cubemap, refractioned).rgb,1.0);
	}
	else if(push.isReflectionEnabled){
		vec3 dist = normalize(fragPos - push.cameraPos);
		vec3 reflected = reflect(dist,fragNorm);
		//reflected.y*=-1.0;
		outColor = vec4(texture(cubemap, reflected).rgb,1.0);	
	}
	else if(push.isWireframeShown){
		outColor = vec4(edgeFactor(1.0) * fragColor, 1.0 );
	}else{
		outColor = vec4( fragColor, 1.0 );
	}
}