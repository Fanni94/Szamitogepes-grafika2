#version 130
#

struct LightInfo {
    vec3 Position; // Light positions in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// pipeline-ból bejövõ per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_normal;
in vec2 vs_out_tex0;

// kimenõ érték - a fragment színe
out vec4 fs_out_col;

// színtér tulajdonságok

uniform vec3 eyePosition;
uniform sampler2D texImage1;
uniform sampler2D texImage2;
uniform float t;

// fénytulajdonságok
uniform int fenydb;
uniform LightInfo lights[10];
uniform float lightIntensity = 70.f;

// anyagtulajdonságok

vec3 materialAmbient = vec3(0.5,0.5,0.5);
vec3 materialDiffuse = vec3(1.0,1.0,1.0);
vec3 materialSpecular = vec3(1.0,1.0,1.0);
float matSpecularSharpness = 5.0;

void main()
{
	vec3 n = vs_out_normal; 
	vec3 totalLighting = vec3(0.0f, 0.0f, 0.0f);
	for (int index = 0; index < fenydb; index++)
    {
		vec3 lightPosition = lights[index].Position;
		vec3 lightAmbient = lights[index].La;
		vec3 lightDiffuse = lights[index].Ld;
		vec3 lightSpecular = lights[index].Ls;

		vec3 toLight = normalize(lightPosition - vs_out_pos);
		vec3 toEye = normalize(eyePosition - vs_out_pos);

		float lightDistance = length(lightPosition - vs_out_pos);
		float falloff = lightIntensity/pow(lightDistance,2.0);

		//
		// ambiens szín számítása
		//
		vec3 ambient = materialAmbient * lightAmbient;

		//
		// diffúz szín számítása
		//

		vec3 diffuse = clamp(dot(-toLight, n),0.f,1.f) * lightDiffuse * materialDiffuse;

		//
		// fényfoltképzõ szín
		//

		vec3 specular = vec3(0.0, 0.0, 0.0);

		if (dot(n, toLight) < -0.05f)
		{
			float eyeReflectionAngle = pow(dot(toEye,reflect(toLight,n)),matSpecularSharpness);
			specular = clamp(eyeReflectionAngle,0.f,1.f) * lightSpecular * materialSpecular;
		}

		totalLighting = totalLighting + (ambient + diffuse + specular)*falloff;
	}
	//
	// a fragment végsõ színének meghatározása
	//
	vec4 texture_color = (1-t)*texture(texImage1, vs_out_tex0.st) + t*texture(texImage2, vs_out_tex0.st);
	vec4 surfColor = vec4(totalLighting, 1.0) * texture_color;

	// Normálvektor kirajzolása
    //fs_out_col = vec4(n*0.5 + 0.5,1.f); 

	// Megvilágított textúrázott pixel kirajzolása
	fs_out_col = surfColor;



//  vec4 ambient = La*Ka;
//
//  float di = clamp(dot(vs_out_normal, -Ldir), 0.0f, 1.0f);
//  vec4 diffuse = Ld*Kd*di;
//
//  vec3 r = reflect(Ldir, vs_out_normal);
//  vec3 c = normalize(Eye - vs_out_pos);
//  float si = pow(clamp(dot(r,c), 0.0f, 1.0f), 16);
//  vec4 specular = Ls*Ks*si;
//
//  vec4 texture_color = (1-t)*texture(texImage1, vs_out_tex0.st) + t*texture(texImage2, vs_out_tex0.st);
//	fs_out_col = (ambient+diffuse)*texture_color;
}

