//#version 110

uniform float time;
uniform float A;
uniform float omega;

void main()
{
	vec4 vertex = gl_Vertex;
	vertex.y = A*sin(omega*time+5.0*vertex.x)*sin(omega*time+5.0*vertex.z);
	gl_Position = gl_ModelViewProjectionMatrix * vertex;

	gl_FrontColor = gl_Color;
}