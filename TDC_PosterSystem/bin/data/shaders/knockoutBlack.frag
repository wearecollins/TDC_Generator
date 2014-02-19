#version 120

uniform sampler2DRect	tex;

void main(){
	vec2 v2 = gl_TexCoord[0].st;
	vec4 col = gl_Color;
	vec4 texColor = texture2DRect(tex, v2);
    
    if( texColor.r > 0.0 ){
		gl_FragColor = texColor * gl_Color;
    }else{
		gl_FragColor.rgb = texColor.rgb * gl_Color.rgb;
		gl_FragColor.a = 0.0;//dist * .1;
	}
	
}