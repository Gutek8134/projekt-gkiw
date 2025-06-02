#version 330

//Uniform variables
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;


uniform vec4 lightPosition;
uniform vec4 redLightSource;

//Attributes
layout (location=0) in vec4 vertex; //vertex coordinates in model space
layout (location=1) in vec4 normal; //vertex normal vector in model space
layout (location=2) in vec2 texCoord; //texturing coordinates


//varying variables
out vec2 i_tc;
out vec4 i_normal;
out vec4 halfway;
out vec4 red_halfway;
out vec4 light;
out vec4 red_light;

void main(void) {
    gl_Position=P*V*M*vertex;

    vec4 viewer = inverse(V)*(vec4(0,0,0,1)-V*M*vertex);
    vec4 lightDir = lightPosition - M*vertex;
    vec4 redLightDir = redLightSource - M*vertex;

    mat4 G=mat4(inverse(transpose(mat3(M))));
    i_normal = G*normal;
    halfway = (lightDir+viewer)/length(lightDir+viewer);
    red_halfway = (redLightDir+viewer)/length(lightDir+viewer);

    light = lightDir;
    red_light = redLightDir;
    
    i_tc=texCoord;
}
