uniform sampler2D difuseMap;
uniform sampler2D lightMap;
uniform sampler2D normalMap;

void main(void)
{

// Extract the normal from the normal map  
vec3 normal = normalize(texture2D(normalMap, gl_TexCoord[0].st).rgb * 2.0 - 1.0);  
  
// Determine where the light is positioned (this can be set however you like)  
vec3 light_pos = normalize(vec3(0, -15, 0));  
  
// Calculate the lighting diffuse value  
float diffuse = max(dot(normal, light_pos), 0.0);  
  
vec3 color = diffuse * texture2D(difuseMap, gl_TexCoord[0].st).rgb * texture2D(lightMap, gl_TexCoord[1].st).rgb;

// Set the output color of our current pixel  
gl_FragColor = vec4(color, 1.0);  

}