// textures
uniform sampler1D transferfunction;

// parametes
uniform vec4 params;


void main()
{

	//debug output: tranfer function color is displayed for current fragment's x coordinate
	gl_FragColor = texture1D ( transferfunction, gl_TexCoord[0].x );
	
	//TODO: implement DVR here
}
