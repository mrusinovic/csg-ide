varying vec4 vertex;
varying vec3 normal, vNormal, lightDir, eyeVec;
varying float att;
varying vec4 vcol;

void main()
{
    normal = normalize(gl_Normal);
    vertex = gl_Vertex;
    vNormal = gl_NormalMatrix * gl_Normal;
    vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
    lightDir = vec3(gl_LightSource[0].position.xyz - vVertex);
    eyeVec = -vVertex;
    float d = length(lightDir); // /8 je (privremeno) da malo smanji opadanje svitlosti
    att = 1.0 / ( gl_LightSource[0].constantAttenuation +
                 (gl_LightSource[0].linearAttenuation * d) +
                 (gl_LightSource[0].quadraticAttenuation * d * d));
           
	vcol = gl_Color;
    gl_Position = ftransform();
    
}
