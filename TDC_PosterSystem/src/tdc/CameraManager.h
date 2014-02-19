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
#include "ofxCv.h"

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
            
            double minVal = 0, maxVal = 0;
            cv::Point minLoc, maxLoc;
            static cv::Rect rect;
            rect.x      = 0;
            rect.y      = 0;
            rect.width  = toDraw.width;
            rect.height = toDraw.height;
            cv::Mat cameraMat = ofxCv::toCv(toDraw);
            cv::Mat roiMat(cameraMat, rect);
            cv::minMaxLoc( roiMat, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
            
            ofVec2f mouse;
            mouse.x = ofMap( maxLoc.x, 0, toDraw.width, 0, (float) ofGetWidth());
            mouse.y = ofMap( maxLoc.y, 0, toDraw.height, 0, (float) ofGetHeight());
            
            if ( lastPoint.distance(mouse) == 0) return;
            
            ofMouseEventArgs args;
            args.x = mouse.x;
            args.y = mouse.y;
            
            ofNotifyEvent(ofEvents().mouseMoved, args );
            lastPoint.set(args.x, args.y);
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
    
    ofVec2f lastPoint;
};