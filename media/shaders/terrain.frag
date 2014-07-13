uniform sampler2D secondMap;
uniform sampler2D baseMap;
varying vec4 vertex,vcol;
varying vec3 normal, vNormal, lightDir, eyeVec;
varying float att;
void main(void)
{
	// tri-planar texture bending factor
	vec3 blending = (abs( normal ) - 0.2) * 7.0;
	blending = normalize(max(blending, 0));      // Force weights to sum to 1.0 (very important!)
	blending /= (blending.x + blending.y + blending.z ).xxx;
	//vec3 blending = abs( normal );
	//vec3 signedBlending = sign(normal) * blending;
	// texture coords
	vec4 coords = vertex * 0.3;
	vec4 col1 = texture2D( baseMap, coords.yz );
	vec4 col2 = texture2D( baseMap, coords.zx );
	vec4 col3 = texture2D( baseMap, coords.xy );
	//vec3 nor1 = texture2D( secondMap, coords.yz ).zxy - 0.5;
	//vec3 nor2 = texture2D( secondMap, coords.zx ).yzx - 0.5;
   // vec3 nor3 = texture2D( secondMap, coords.xy ).xyz - 0.5;
	// Finally, blend the results of the 3 planar projections.
	vec4 colBlended = col1 * blending.x + col2 * blending.y + col3 * blending.z;

	if(att > 0.1) {
	
		//vec3 norBlended = nor1 * signedBlending.x + nor2 * signedBlending.y + nor3 * signedBlending.z;
		vec3 N = normalize(gl_NormalMatrix * normal);//norBlended - replaced with vertex normal
		vec3 L = normalize(lightDir);
		float lambertTerm = max(0.32, min(dot(N,L),vcol.x)); //0.22 govori koliko ce bit tamne sjene//min(dot(N,L),vcol.x) takoda u sjeni ne sjaji
		lambertTerm = min(lambertTerm, vcol.w); ///w je alpha
		
		colBlended = colBlended * lambertTerm * att;
		//vec3 E = normalize(eyeVec);
		//vec3 R = reflect(-L, N);
		//float specular = 0.25 * pow( max(dot(R, E), 0.0), 32 ) * att;
		//colBlended += specular * att;		
	}else{
		colBlended = vec4(0,0,0,0);
	}
/*
	const float LOG2 = 1.442695;
	float z = (gl_FragCoord.z / gl_FragCoord.w)/100;
	float fogFactor = exp2( -gl_Fog.density * gl_Fog.density * z * z * LOG2 );

	fogFactor = clamp(fogFactor, 0.0, 1.0);
	gl_FragColor = mix(gl_Fog.color,colBlended,fogFactor);	
*/
	gl_FragColor = colBlended;//*vcol;	

}