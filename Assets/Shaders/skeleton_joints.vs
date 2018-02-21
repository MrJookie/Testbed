#version 330

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 64;
uniform mat4 boneTransformation[MAX_BONES];

void main() {
	gl_PointSize = 6.0;
		 	
	gl_Position = projection * view * model * boneTransformation[gl_VertexID] * vec4(0.0, 0.0, 0.0, 1.0);
}
