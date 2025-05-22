#version 330

uniform sampler2D tex;

out vec4 pixelColor; //Output variable. Almost final pixel color.
in vec2 texture_coordinate;
in vec4 light;
in vec4 eyeNormal;
in vec4 viewPosition;
in vec4 f_lightColor;
in float f_phongExponent;

void main(void) {
	vec4 n_light = normalize(light);
	vec4 n_eyeNormal = normalize(eyeNormal);
	vec4 n_viewPosition = normalize(viewPosition);
	vec4 reflection = reflect(-n_light, n_eyeNormal);
	pixelColor=texture(tex, texture_coordinate)*dot(n_viewPosition, n_eyeNormal)+clamp(pow(dot(reflection, n_viewPosition),f_phongExponent),0,1);
    // pixelColor = vec4(1,1,1,1)*dot(n_viewPosition,n_eyeNormal);
}
