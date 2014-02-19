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
        
        kinect.setRegistration(true);
        kinect.init();
        kinect.open();
        kinect.setDepthClipping( near, far );
        
        ofAddListener( ofEvents().update, this, &CameraManager::update );
    }
    
    void update( ofEventArgs & e ){
        kinect.update();
        if ( kinect.isFrameNew() ){
            toDraw.setFromPixels(kinect.getDepthPixelsRef());
            toDraw.mirror(false, true);
        }
    }
    
    void draw( int width=160, int height=120){
        toDraw.draw(0,0,width,height);
    }
    
    ofImage & getImage(){
        return toDraw;
    }
    
    ofxKinect & getKinect(){
        return kinect;
    }
    
protected:
    
    ofImage toDraw;
    ofPixels scaledPixels;
    int near, far;
    ofxKinect kinect;
    
};