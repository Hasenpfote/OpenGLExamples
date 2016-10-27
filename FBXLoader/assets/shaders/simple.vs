#version 430
uniform mat4 matWorld;
uniform mat4 jointPalette[128];
uniform bool isSkinnedMesh;

layout(location = 0) in vec3 vsPosition;
layout(location = 1) in vec3 vsNormal;
layout(location = 2) in vec2 vsTc0;
layout(location = 3) in uvec4 vsJointIndices;
layout(location = 4) in vec4 vsJointWeights;


layout (std140) uniform CommonMatrices
{
	mat4 view;
	mat4 projection;
};


out gl_PerVertex{
    vec4 gl_Position;
	vec4 gl_FrontColor;
};

out vec2 fsTc0;
out vec4 fsColor;

void main()
{
	if(isSkinnedMesh){
		mat4 matJoint = jointPalette[vsJointIndices.x] * vsJointWeights.x;
		matJoint += jointPalette[vsJointIndices.y] * vsJointWeights.y;
		matJoint += jointPalette[vsJointIndices.z] * vsJointWeights.z;
		matJoint += jointPalette[vsJointIndices.w] * vsJointWeights.w;
		gl_Position = projection * view * matWorld * matJoint * vec4(vsPosition, 1.0);
	}
	else{
		gl_Position = projection * view * matWorld * vec4(vsPosition, 1.0);
	}

	fsTc0.x = vsTc0.x;
	fsTc0.y = 1.0 - vsTc0.y;
	fsColor.rgb = 0.5 * vsNormal.xyz + 0.5;
	fsColor.a = 1.0;
}