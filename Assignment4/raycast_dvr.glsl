// textures
uniform sampler1D transferfunction;
uniform sampler3D vol_texture;
uniform sampler2D front_texture;
uniform sampler2D back_texture;

// parametes
uniform float brightness;
uniform float step_size;
uniform bool enable_lighting;

float base_step_size = 0.0025f;
float shininess = 32.0f;
float delta = 0.01;
float termination_threshold = 0.95;

struct PointLight {
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
};
PointLight L = PointLight(vec3(0.9f, 0.5f, 0.3f), vec3(0.4f, 0.2f, 0.1f), vec3(0.8f, 0.8f, 0.8f));

void main()
{
	// Direct Volume Rendering
	// Initialize accumulated color and opacity
	vec4 dst = vec4(0.0);
	// Determine volume entry position
	vec3 position = vec3(texture2D(front_texture, gl_TexCoord[0].st));
	// Compute ray direction
	vec3 direction = vec3(texture2D(back_texture, gl_TexCoord[0].st)) - position;
	// Normalized ray direction
	direction = normalize(direction);
	vec3 pos = position;
	bool hit = false;

	// Loop for ray traversal
	for (int i = 0; i < 512; i++)	// Some large number
	{
		// Data access to scalar value in 3D valume texture
		vec4 value = texture3D(vol_texture, position);
		// Opacity correction
		float opacity = 1 - pow(1 - value.a, step_size / base_step_size);
		if (opacity > 0.15 && !hit)
		{
			hit = true;
			pos = position;
		}
		// Apply transfer function
		vec4 src = texture1D(transferfunction, opacity);
		// Front-to-back compositing
		dst += (1.0-dst.a) * src;
		// Early ray Termination
		if (dst.a > termination_threshold) break;
		// Advance ray position along ray direction
		position += direction * step_size;
	}

	vec4 light = vec4(1.0, 1.0, 1.0, 1.0);
	if (enable_lighting) {
		vec3 ambient = brightness * L.ambient;
		vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
		vec3 specular = vec3(0.0f, 0.0f, 0.0f);
		// Shading
		vec4 result = texture1D(transferfunction, texture3D(vol_texture, pos).a);
		if (result.a > 0.1) {
			vec3 norm;
			norm.x = texture3D(vol_texture, pos + vec3(delta, 0.0, 0.0)).x - texture3D(vol_texture, pos - vec3(delta, 0.0, 0.0)).x;
			norm.y = texture3D(vol_texture, pos + vec3(0.0, delta, 0.0)).x - texture3D(vol_texture, pos - vec3(0.0, delta, 0.0)).x;
			norm.z = texture3D(vol_texture, pos + vec3(0.0, 0.0, delta)).x - texture3D(vol_texture, pos - vec3(0.0, 0.0, delta)).x;
			norm = normalize(norm);

			float diff = max(dot(norm, direction), 0.0);
			diffuse = diff * L.diffuse;

			vec3 viewDir = normalize(-pos);
			vec3 reflectDir = reflect(-direction, norm);
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
			specular = spec * L.specular;
		}
		light = vec4(ambient + diffuse + specular, 1.0f);
	}
	gl_FragColor = light * dst;
}
