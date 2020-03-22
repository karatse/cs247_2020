// textures
uniform sampler1D transferfunction;

// parametes
uniform vec4 params;


void main()
{
	//debug output: a single color
	gl_FragColor = params;
	
	//TODO: to iso surface raycasting here
}
