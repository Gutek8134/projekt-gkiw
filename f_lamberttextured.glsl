#version 330


uniform sampler2D tex;
uniform sampler2D rough;

out vec4 pixelColor; //Output variable of the fragment shader. (Almost) final pixel color.

//Varying variables
in vec2 i_tc;
in vec4 i_normal;
in vec4 halfway;
in vec4 light;

void main(void) {
    vec4 n_normal = normalize(i_normal);
    vec4 n_halfway = normalize(halfway);
    vec4 n_light = normalize(light);
    vec4 color=texture(tex,i_tc);

	pixelColor=
    vec4(color.rgb*clamp(dot(n_light, n_normal),0,1), color.a)
    + vec4(color.rgb*pow(clamp(dot(n_halfway, n_normal),0,1), 30), 0)
    * texture(rough, i_tc)
    ;
}