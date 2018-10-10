#version 130

// VBO-ból érkezõ változók
in vec3 vs_in_pos;
in vec2 vs_in_tex0;

// a pipeline-ban tovább adandó értékek
out vec3 vs_out_pos;
out vec3 vs_out_normal;
out vec2 vs_out_tex0;
out vec3 vs_out_light_pos;

// shader külsõ paraméterei - most a három transzformációs mátrixot külön-külön vesszük át
uniform mat4 MVP;
uniform mat4 world;
uniform float eps = 0.01f;

vec3 calculatePos(float u, float v, float d) {
	return vec3(d*cos(u)*sin(v),d*sin(u)*sin(v), d*cos(v) );
}

vec3 calculateNormal(float u, float v, float d) {
	vec3 neighborU = calculatePos(u- eps,v, d);
	vec3 neighborV = calculatePos(u,v+ eps, d);

	vec3 pos = calculatePos(u,v,d);

	vec3 tangent = normalize(neighborU - pos);
	vec3 bitangent = normalize(neighborV - pos);

	return normalize(cross(tangent,bitangent));
}

vec3 calculateNormalDerivalas(float u, float v, float d) {
  float dux = -d*sin(u)*sin(v);
  float duz = 0;
  float duy = d*cos(u)*sin(v);

  float dvx = d*cos(u)*cos(v);
  float dvz = -d*sin(v);
  float dvy = d*sin(u)*cos(v);

  return normalize(cross(vec3(dvx,dvy,dvz), vec3(dux,duy,duz)));
}

void main()
{
// Legyen P(u,v,d) = (1+d)*[ cos(2pi*u)*sin(pi*v), sin(2pi*u)*sin(pi*v), cos(pi*v)] a parametrikus felület alakja (gömb).
// d=m(u,v)= -0.05 és 0.05

  float d = 1+vs_in_pos.z;
  float u = vs_in_pos.x;
  float v = vs_in_pos.y;
	u *= 2*3.1415f;
  v *= 3.1415f;

  float x = d*cos(u)*sin(v);
  float y = d*cos(v);
  float z = d*sin(u)*sin(v);

  float dux = d*-1*sin(u)*sin(v);
  float duy = 0;
  float duz = d*cos(u)*sin(v);

  float dvx = d*cos(u)*cos(v);
  float dvy = d*-1*sin(v);
  float dvz = d*sin(u)*cos(v);

  gl_Position = MVP * vec4( calculatePos(u,v,d),  1 );
  vs_out_pos = (world * vec4(calculatePos(u,v,d), 1 )).xyz;
  vs_out_tex0 = vs_in_tex0;
  //vs_out_normal = normalize(cross(vec3(dvx,dvy,dvz), vec3(dux,duy,duz)));
  vs_out_normal = calculateNormalDerivalas(u,v,d);
}
