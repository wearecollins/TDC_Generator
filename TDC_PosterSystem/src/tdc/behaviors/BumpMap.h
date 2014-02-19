//
//  BumpMap.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/13/14.
//
//

#pragma once

#include "Behavior.h"

// this should be namespaced like woah

class BumpMap : public Behavior {
public:
    
    BumpMap() : Behavior(){
        name = "bumpmap";
        camera = NULL;
    }
    
    void setup( ofxLabFlexParticleSystem::Container * particles ){
        
        shader.load("shaders/bumpmap.vert","");
        renderFBO.allocate(ofGetWidth(), ofGetHeight());
    }
    
    void update( TypeParticle * p ){
        (*p) += ofVec2f( ofSignedNoise(p->getSeedPosition().x + p->getOffsetX() + (ofGetElapsedTimeMillis() * timeFactor)) * intensity.x, ofSignedNoise(p->getSeedPosition().y + p->getOffsetY() + (ofGetElapsedTimeMillis() * timeFactor)) * intensity.y);
    }
    
    void beginDraw(){
                
        //renderFBO.begin();
        //ofClear(0,0,0,0);
        shader.begin();
        shader.setUniform2f("depthDims", 320.0f, 240.0f);
        shader.setUniform2f("screenDims", (float) ofGetWidth(), (float) ofGetHeight());
        shader.setUniform1f("extrusion", mix * 1000.0f);
        if ( camera != NULL ){
            shader.setUniformTexture("depthTex", camera->getImage().getTextureReference(), 0);
        } else {
            cout << "CAMERA IS NULL!"<<endl;
        }
    }
    
    void endDraw(){
        shader.end();
        ofPopMatrix();
        //renderFBO.end();
    }
    
    void draw(){
        ofSetColor(255);
        //renderFBO.draw(0,0);
        camera->draw();
    }
    
protected:
    
    ofShader shader;
    ofFbo renderFBO;
    ofFbo lastRender;
};

