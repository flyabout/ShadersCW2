#version 430

in vec3 vertPos;			// Vertex position
in vec3 N;					// Surface normal
in vec3 lightPos;			// Light position
in vec3 SpotLightworld;		// Spotlight Position in eye coordinates

uniform vec3 Kd;            // Diffuse reflectivity
uniform vec3 Ld;            // Diffuse light intensity
uniform vec3 Ka;			// Ambient reflectivity
uniform vec3 La;			// Ambient light intensity
uniform vec3 Ks;			// Specular reflectivity
uniform vec3 Ls;			// Specular light intensity
uniform vec3 cameraPos;		// Camera position
uniform float n;			// Specular intensity
uniform float Attenuation;	// Attenuation value
uniform float lightCutoff;	// Spotlights radius
uniform float OuterlightCutoff; // Spotlights outer radius
uniform float SpotlightAttenuation;	// Attenuation value for the spotlight
uniform vec3 SpotlightDir; //Spotlight vector direction

// complete to a full phong shading
layout( location = 0 ) out vec4 FragColour;

vec4 Specular(vec3 L, float attenuation) //Calculates specular lighting using a given attenuation and a given light direction
{
	vec3 V = normalize(cameraPos - vertPos);
	vec3 R = reflect(-L, N);
	vec4 result = (vec4(Ks*Ls, 1.0) * pow(max(dot(V, R), 0.0), n)) * attenuation;
	result = clamp(result, 0.0, 1.0);
	return result;
}

vec4 Diffusion(vec3 L, float attenuation) //Calculates Diffusion lighting using a given attenuation and a given light direction
{
	vec4 result = (vec4(Kd*Ld, 1.0) * max(dot(N, L), 0.0)) * attenuation;
	result = clamp(result, 0.0, 1.0);
	return result;
}

vec4 Ambient(float attenuation) // Calculates ambient lighting using a given attenuation value
{
	vec4 result = (vec4(Ka*La, 1.0)) * attenuation;
	result = clamp(result, 0.0, 1.0);
	return result;
}

vec4 Light()
{
	vec3 L = normalize(lightPos - vertPos);
	float lightDistance = length(lightPos - vertPos); // Get light distance to vertex position
	float attenuation = (1.0 / (Attenuation + (0.007 * lightDistance) + (0.0002 * (lightDistance * lightDistance)))); // Calculate attenuation 
	FragColour = clamp(vec4(Specular(L, attenuation) + Ambient(attenuation) + Diffusion(L, attenuation)), 0.0, 1.0); //Apply shaders to frag colour from light source
	return FragColour;
}

vec4 Spotlight() // Calculates a spotlight light source
{
	vec3 directionSpotlight = normalize(SpotLightworld - vertPos); // Get light direction
	float theta = dot(directionSpotlight, normalize(-SpotlightDir)); // Calculate angle between directionSpotlight and SpotlightDir
	float SpotlightDistance = length(SpotLightworld - vertPos); // Get distance between Spotlight and vertex position
	float Attenuation = (1.0 / (SpotlightAttenuation + (0.007 * SpotlightDistance) + (0.0002 * (SpotlightDistance * SpotlightDistance)))); // Calculate attenuation values for spotlight
	float CutoffDifference = (lightCutoff - OuterlightCutoff); // Calculate cosine difference between the inner and outer cone of light
	float intensity = clamp((theta - OuterlightCutoff) / CutoffDifference, 0.0, 1.0); //Get light intensity for each fragment
	FragColour = clamp(vec4((Specular(directionSpotlight, Attenuation) * intensity) + (Ambient(Attenuation) * intensity) + (Diffusion(directionSpotlight, Attenuation) * intensity)), 0.0, 1.0);
	return FragColour;
}

void main() 
{
	FragColour = clamp(vec4(Spotlight() + Light()), 0.0, 1.0); // Combine the different lights
}