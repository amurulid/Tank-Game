attribute vec4 aPosition;
attribute vec3 aNormal;
uniform vec3 uLightPos;
uniform vec3 UaColor;
uniform vec3 UdColor;
uniform vec3 UsColor;
uniform float Ushine;
uniform int uShadeModel;
uniform int uN;
uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform int type;

varying vec3 vCol;
varying vec3 vPos;
varying vec3 vNor;


void main()
{
	vPos = vec3(uViewMatrix * uModelMatrix * aPosition);
	vNor = normalize(vec3(uViewMatrix * uModelMatrix * vec4(aNormal, 0.0)));

    vec3 modelViewVertex = vec3(uViewMatrix * uModelMatrix * aPosition);
    vec3 modelViewNormal = normalize(vec3(uViewMatrix * uModelMatrix * vec4(aNormal, 0.0)));
 
    vec3 lightVector = normalize(uLightPos - modelViewVertex);
	vec3 lightRefl = normalize(reflect(lightVector, modelViewNormal));

    float diffuse = max(dot(lightVector, modelViewNormal), 0.0);
	float specular = pow(max(dot(lightRefl, normalize(modelViewVertex)), 0.0), Ushine);
 
    vCol = UdColor * diffuse + UsColor * specular + UaColor;
	if (uN == 1) 
		vCol = normalize(aNormal);
    gl_Position = uProjMatrix * uViewMatrix * uModelMatrix * aPosition;
}