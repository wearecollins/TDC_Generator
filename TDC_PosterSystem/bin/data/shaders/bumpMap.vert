#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect depthTex;

uniform vec2 depthDims;
uniform vec2 screenDims;
uniform float extrusion;

float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool bClamp) {
    float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
    
    if( bClamp ){
        if(outputMax < outputMin){
            if( outVal < outputMax )outVal = outputMax;
            else if( outVal > outputMin )outVal = outputMin;
        }else{
            if( outVal > outputMax )outVal = outputMax;
            else if( outVal < outputMin )outVal = outputMin;
        }
    }
    return outVal;
}

void main() {
    // look up color based on mapped coordinate
	vec4 pos_gl = gl_Vertex;//gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
    
    vec2 coord = vec2(ofMap(pos_gl.x, 0., screenDims.x, 0., depthDims.x, true), ofMap(pos_gl.y, 0., screenDims.y, 0., depthDims.y, true));
    
    vec4 color = texture2DRect( depthTex, coord );
    
    vec4 pos = pos_gl;
    vec4 pos_og = pos_gl;
    
    //pos.z = (1.0-color.r) * extrusion;
    pos.z = ((color.r) * extrusion);
    
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * pos;
    gl_FrontColor =  gl_Color;
}