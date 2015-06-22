varying vec3 vCol;
uniform vec3 UaColor;
uniform vec3 UdColor;
uniform vec3 UsColor;
uniform float Ushine;
uniform int uShadeModel;
uniform vec3 uLightPos;
uniform int uN;

varying vec3 vPos;
varying vec3 vNor;

void main()
{	
	if (uShadeModel == 0 || uN == 1)
		gl_FragColor = vec4(vCol,1.0);
	else {
		vec3 lightVector = normalize(uLightPos - vPos);
		vec3 lightRefl = normalize(reflect(-lightVector,vNor));  

		float diffuse = max(dot(normalize(vNor), lightVector), 0.0);
		float specular = pow(max(dot(lightRefl, normalize(-vPos)), 0.0), Ushine);

		gl_FragColor = vec4(UdColor * diffuse + UsColor * specular + UaColor, 1.0);
	}
}
