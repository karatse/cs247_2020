// textures
uniform sampler3D scalar_field_3D;
uniform sampler3D vector_field_3D;
uniform sampler2D noise_field;
uniform int L;
uniform bool scalar_toggle;

// TODO: =======================================================================
//
// Add custom uniforms and constants here
//
// =============================================================================
float kernel_value(int i) {
    return exp(-(i-L/2)*(i-L/2));
}

void main()
{

	// TODO: ===================================================================
	//
	// Implement LIC and colorcoded scalar field overlay
	//
	// =========================================================================
    const float step = 0.001;
    vec3 position = vec3(gl_TexCoord[0].st, 1);
    vec3 t = texture2D(noise_field, position);
    vec3 smoothed_value = kernel_value(L/2) * t;
    if (scalar_toggle) {
        gl_FragColor = vec4(t, 1);
        return;
    }
    float kvs = 0;
    vec3 pp = position;
    vec3 pm = position;
    for (int i=0; i<L; i++) {
        float kv = kernel_value(i);
        vec3 vp = texture3D(vector_field_3D, pp);
        pp += vp * step;
        smoothed_value += kv * texture2D(noise_field, pp);
        vec3 vm = texture3D(vector_field_3D, pm);
        pm -= vm * step;
        smoothed_value += kv * texture2D(noise_field, pm);
        kvs += kv;
    }
    smoothed_value /= kvs;

	gl_FragColor = vec4(smoothed_value, 1.0);
}