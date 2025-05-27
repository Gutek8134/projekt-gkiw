#version 330

uniform vec4 surfaceColor=vec4(0,0,0,1);
uniform vec4 lightColor = vec4(1,0,0,1);

out vec4 pixelColor; //Output variable of the fragment shader. (Almost) final pixel color.

//Varying variables
in vec4 lightDir;
in vec4 i_normal;
in vec4 viewPosition;

void main(void) {
	pixelColor=surfaceColor
    +lightColor
    *clamp(dot(
        normalize(lightDir),
        normalize(i_normal)
    ),0,1);
}