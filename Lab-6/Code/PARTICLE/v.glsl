uniform float time;
attribute  float vx, vy, vz;
const float a = -0.000001;

void main()
{
    
    vec4 t = gl_Vertex;
    t.y = gl_Vertex.y + vy*time + 0.5*a*time*time;
    t.x = gl_Vertex.x + vx*time;
	t.z = gl_Vertex.z + vz*time;
	
	gl_PointSize = max(t.z*t.y,0.1);
    gl_Position = gl_ModelViewProjectionMatrix*t;
    gl_FrontColor =  gl_Color;
}