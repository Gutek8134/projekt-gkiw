#version 330


uniform sampler2D tex;
uniform sampler2D rough;

uniform vec4 redLightColor = vec4(1,0,0,1);

out vec4 pixelColor; //Output variable of the fragment shader. (Almost) final pixel color.

//Varying variables
in vec2 i_tc;
in vec4 i_normal;
in vec4 halfway;
in vec4 red_halfway;
in vec4 light;
in vec4 red_light;

void main(void) {
    vec4 n_normal = normalize(i_normal);
    vec4 n_halfway = normalize(halfway);
    vec4 n_red_halfway = normalize(red_halfway);
    vec4 n_light = normalize(light);
    vec4 n_red_light = normalize(red_light);
    vec4 color=texture(tex,i_tc);

	pixelColor=
    vec4(color.rgb*clamp(dot(n_light, n_normal),0,1), color.a)
    + vec4(redLightColor.rgb*clamp(dot(n_red_light, n_normal),0,1), 0)
    + (vec4(color.rgb*pow(clamp(dot(n_halfway, n_normal),0,1), 30), 0)
    + vec4(redLightColor.rgb*pow(clamp(dot(n_red_halfway, n_normal),0,1), 30)*0.8/(length(red_light)-0.5), 0))*texture(rough, i_tc)
    ;
}
