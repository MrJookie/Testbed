#version 330

in vec3 position;
in vec3 normal;
in vec2 texCoords;
in ivec4 boneID;
in vec4 weight;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//uniform mat4 mvp;

const int MAX_BONES = 64;
uniform mat4 boneTransformation[MAX_BONES];

void main() {
	vec4 PosL = vec4(0);
	
	if(weight[0] + weight[1] + weight[2] + weight[3] > 0.0f) { //or just 0.9, or 0.99 due to sum of weights = 1
		 mat4 BoneTransform   = boneTransformation[int(boneID[0])] * weight[0];
		 BoneTransform		+= boneTransformation[int(boneID[1])] * weight[1];
		 BoneTransform		+= boneTransformation[int(boneID[2])] * weight[2];
		 BoneTransform		+= boneTransformation[int(boneID[3])] * weight[3];

		 PosL = BoneTransform * vec4(position, 1.0);
		 
		 //PosL = vec4(position, 1.0);
	} else { //0.0 means no weight exists, do not use bones
		PosL = vec4(position, 1.0);
	}
	
	gl_Position = projection * view * model * PosL;
	
	FragPos = vec3(model * PosL);
	Normal = mat3(transpose(inverse(model))) * normal;
	TexCoords = texCoords;
}
