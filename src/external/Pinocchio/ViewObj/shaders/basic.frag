#version 120

varying vec2 st;
varying vec3 n, p;
uniform sampler2D dm;

void main () {
	// shading darker based on angle to eye
	vec3 n_eye = normalize (n);
	vec3 p_to_eye = normalize (-p);
	float dp = dot (n_eye, p_to_eye);

	vec4 texel = texture2D (dm, st);

	gl_FragColor = vec4 (texel.rgb * dp, 1.0);
}
