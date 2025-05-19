#version 330

//Uniform variables
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 lightColor = vec4(1, 1, 1, 1);
// World space
uniform vec4 lightPosition = vec4(0, 0, -6, 1);
uniform int phongExponent=10;

//Attributes
in vec4 vertex; //Vertex coordinates in model space
in vec4 colors;
// Model space
in vec4 normals;

out vec4 color;
out vec4 light;
out vec4 eyeNormal;
out vec4 viewPosition;
out vec4 f_lightColor;
out float f_phongExponent;

void main(void) {
    gl_Position=P*V*M*vertex;
    // Eye space
    light = normalize(V*lightPosition - V*M*vertex);
    eyeNormal = normalize(V*M*normals);
    viewPosition = normalize(vec4(0,0,0,1)-V*M*vertex);
    color = colors;
    f_lightColor = lightColor;
    f_phongExponent = phongExponent;
}
