//
//  BumpMap.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/13/14.
//
//

#pragma once

#include "Behavior.h"

// for now
#ifdef USE_OPENNI
//#include "ofxNI2.h"
#else
#include "ofxKinect.h"
#endif
#include "DepthRemapToRange.h"

// this should be namespaced like woah

class BumpMap : public Behavior {
public:
    
    BumpMap() : Behavior(){
        name = "bumpmap";
    }
    
    void setup( ofxLabFlexParticleSystem::Container * particles ){
        near = 800;
        far  = 1000;
        
#ifdef USE_OPENNI
        // setup device
        device = new ofxNI2::Device();
        device->setup();
        
        if (depth.setup(*device))
        {
            depth.setSize(320, 240);
            depth.setFps(30);
            depth.start();
            
            ofPtr<ofxNI2::Grayscale> shader = depth.getShader<ofxNI2::Grayscale>();
            shader->setNear( 50 );
            shader->setFar( 2000 );
        }
#else
        kinect.setRegistration(true);
        kinect.init();
        kinect.open();
        kinect.setDepthClipping( near, far );
#endif
        
        shader.load("shaders/bumpmap.vert","");
        renderFBO.allocate(ofGetWidth(), ofGetHeight());
    }
    
    void update( TypeParticle * p ){
        (*p) += ofVec2f( ofSignedNoise(p->getSeedPosition().x + p->getOffsetX() + (ofGetElapsedTimeMillis() * timeFactor)) * intensity.x, ofSignedNoise(p->getSeedPosition().y + p->getOffsetY() + (ofGetElapsedTimeMillis() * timeFactor)) * intensity.y);
    }
    
    void beginDraw(){
#ifdef USE_OPENNI
        depth.updateTextureIfNeeded();
        
        ofxNI2::depthRemapToRange(depth.getPixelsRef(), scaledPixels, near, far, true);
        toDraw.setFromPixels(scaledPixels);
#else
        kinect.update();
        toDraw.setFromPixels(kinect.getDepthPixelsRef());
#endif
        
        toDraw.mirror(false, true);
        
        renderFBO.begin();
        ofPushMatrix();
        ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
        ofScale(scale, scale);
        ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
        
        ofClear(0,0,0,0);
        shader.begin();
        shader.setUniform2f("depthDims", 320.0f, 240.0f);
        shader.setUniform2f("screenDims", (float) ofGetWidth(), (float) ofGetHeight());
        shader.setUniform1f("extrusion", mix * 1000.0f);
        shader.setUniformTexture("depthTex", toDraw.getTextureReference(), 0);
    }
    
    void endDraw(){
        shader.end();
        ofPopMatrix();
        renderFBO.end();
    }
    
    void draw(){
        ofSetColor(255);
        renderFBO.draw(0,0);
        toDraw.draw(0,0,160,120);
    }
    
protected:
    
    ofShader shader;
    ofFbo renderFBO;
    ofFbo lastRender;
    
    ofImage toDraw;
    ofPixels scaledPixels;
    int near, far;
    
#ifdef USE_OPENNI
    ofxNI2::Device * device;
    ofxNI2::DepthStream depth;
#else
    ofxKinect kinect;
#endif
};

