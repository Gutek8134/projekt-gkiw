#version 330


out vec4 pixelColor; //Output variable. Almost final pixel color.
in vec4 color;
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
	pixelColor=color*round((clamp((n_light, n_eyeNormal), 0, 1))*3)/3+round(pow(clamp(dot(reflection, n_viewPosition),0,1),f_phongExponent)*3)/3;
}
