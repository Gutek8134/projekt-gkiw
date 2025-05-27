#version 330

//Uniform variables
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//World space
uniform vec4 lightSource=vec4(3.f, 6, 0.5f, 1);

//Attributes
layout (location=0) in vec4 vertex; //vertex coordinates in model space
layout (location=1) in vec4 normal; //vertex normal vector in model space


//World space
out vec4 lightDir;
out vec4 i_normal;
out vec4 viewPosition;

void main(void) {
    gl_Position=P*V*M*vertex;

    lightDir = lightSource-M*vertex;
    i_normal = M*normal;
    viewPosition = normalize(vec4(0,0,0,1)-V*M*vertex);
}
