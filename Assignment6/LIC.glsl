// textures
uniform sampler3D scalar_field_3D;
uniform sampler3D vector_field_3D;
uniform sampler2D noise_field;

// =============================================================================
//
// Add custom uniforms and constants here
//
// =============================================================================
uniform int L;
uniform float blend;
uniform bool scalar_toggle;
const float step = 0.01;

float kernel_value(int i) {
    float x = i / float(L);
    return exp(-x*x);
}

void main()
{
    // =========================================================================
    //
    // Implement LIC and colorcoded scalar field overlay
    //
    // =========================================================================
    vec3 position = vec3(gl_TexCoord[0].st, 1);
    float kv = kernel_value(L/2);
    vec3 smoothed_value = kv * texture2D(noise_field, position);
    float kvs = kv;
    vec3 pp = position;
    vec3 pm = position;
    for (int i=0; i<L/2; i++) {
        kv = kernel_value(i);

        vec3 vp = texture3D(vector_field_3D, pp) * 2 - 1;
        pp += vp * step;
        smoothed_value += kv * texture2D(noise_field, pp);
        kvs += kv;

        vec3 vm = texture3D(vector_field_3D, pm) * 2 - 1;
        pm -= vm * step;
        smoothed_value += kv * texture2D(noise_field, pm);
        kvs += kv;
    }
    smoothed_value /= kvs;
    float color = smoothed_value.x;
    smoothed_value = vec3(color, 1-color, 0);
    if (scalar_toggle) {
        vec3 scalar_value = texture3D(scalar_field_3D, position);
        gl_FragColor = vec4(smoothed_value * blend + scalar_value * (1-blend), 1);
    } else {
        gl_FragColor = vec4(smoothed_value, 1);
    }
}