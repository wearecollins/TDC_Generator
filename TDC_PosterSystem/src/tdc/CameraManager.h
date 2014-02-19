//
//  CameraManager.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/19/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxKinect.h"

class CameraManager {
public:
    
    CameraManager(){
        
    }
    
    void setup(){
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
        
        ofAddListener( ofEvents().update, this, &CameraManager::update );
    }
    
    void update( ofEventArgs & e ){
        
#ifdef USE_OPENNI
        depth.updateTextureIfNeeded();
        
        ofxNI2::depthRemapToRange(depth.getPixelsRef(), scaledPixels, near, far, true);
        toDraw.setFromPixels(scaledPixels);
#else
        kinect.update();
        toDraw.setFromPixels(kinect.getDepthPixelsRef());
#endif
    }
    
    void draw( int width=160, int height=120){
        toDraw.mirror(false, true);
        toDraw.draw(0,0,width,height);
    }
    
    ofImage & getImage(){
        return toDraw;
    }
    
protected:
    
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