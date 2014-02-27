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
#include "ColorTracker.h"

static void getQuadSubImage(ofImage& inputImage, ofImage& outputImage, vector <ofPoint>& quad, ofImageType imageType) {
    if ( quad.size() < 4 ){
        ofLog( OF_LOG_ERROR, "You must pass a vector of four points to this function");
        return;
    } // weird thing that could happen...
    
    static unsigned char * inpix;
    static unsigned char * outpix;
    inpix   = inputImage.getPixels();
    outpix  = outputImage.getPixels();
    
    int inW, inH, outW, outH;
    inW = inputImage.width;
    inH = inputImage.height;
    outW = outputImage.width;
    outH = outputImage.height;
    
    int bpp = 1;
    if (imageType == OF_IMAGE_COLOR){
        bpp = 3;
    } else if (imageType == OF_IMAGE_COLOR_ALPHA){
        bpp = 4;
    }
    
    int xinput =0;
    int yinput = 0;
    int inIndex = 0;
    int outIndex = 0;
    
    float xlrp = 0.0;
    float ylrp = 0.0;
    
    ofPoint p1, p2, p3, p4;
    p1 = quad[0];
    p2 = quad[1];
    p3 = quad[2];
    p4 = quad[3];
    
    for(int x=0;x<outW;x++) {
        for(int y=0;y<outH;y++) {
            xlrp = x/(float)outW;
            ylrp = y/(float)outH;
            xinput = (p1.x*(1-xlrp)+p2.x*xlrp)*(1-ylrp) + (p4.x*(1-xlrp)+p3.x*xlrp)*ylrp;
            yinput = ((p1.y*(1-ylrp))+(p4.y*ylrp))*(1-xlrp) + (p2.y*(1-ylrp)+p3.y*ylrp)*xlrp;
            inIndex = (xinput + (yinput*inW))*bpp;
            outIndex = (x+y*outW)*bpp;
            memcpy((outpix+outIndex),(inpix+inIndex),sizeof(unsigned char)*bpp);
        }
    }
    outputImage.setFromPixels(outpix, outW, outH, imageType);
}


class CameraManager {
public:
    
    CameraManager(){
        
    }
    
    ~CameraManager(){
        kinect.close();
    }
    
    void setup(){
        near = 800;
        far  = 1000;
        
        kinect.setRegistration(true);
        kinect.init();
        kinect.open();
        kinect.setDepthClipping( near, far );
        
        bFlipHoriz = false;
        bFlipVert = false;
        bFlipAxes = false;
        
        quad.push_back(ofPoint(0,0));
        quad.push_back(ofPoint(640,0));
        quad.push_back(ofPoint(640,480));
        quad.push_back(ofPoint(0,480));
        
        ofAddListener( ofEvents().update, this, &CameraManager::update );
        tracker.setup();
    }
    
    void update( ofEventArgs & e ){
        kinect.setDepthClipping( near, far );
        kinect.update();
        if ( kinect.isFrameNew() ){
            toDraw.setFromPixels(kinect.getDepthPixelsRef());
            toDraw.mirror(bFlipVert, bFlipHoriz);
            if ( bFlipAxes ) toDraw.rotate90(1);
            
            getQuadSubImage(toDraw, toDraw, quad, toDraw.getPixelsRef().getImageType());
            
            ofxCv::threshold(toDraw, thresh);
            toDraw.update();
            
            colorPixels.setFromPixels( kinect.getPixels(), kinect.getWidth(), kinect.getHeight(), kinect.getPixelsRef().getNumChannels());
            tracker.updateCamera( kinect.getPixelsRef() );
            
            double minVal = 0, maxVal = 0;
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
            
            ofPoint args;
            args.x = mouse.x;
            args.y = mouse.y;
            
            ofNotifyEvent(cameraMove, args );
            lastPoint.set(args.x, args.y);
            
        }
    }
    
    void draw( int x = 0, int y = 0, int width=160, int height=120){
        if ( toDraw.isAllocated()){
            toDraw.draw(x,y,width,height);
            
            ofVec2f mouse;
            mouse.x = ofMap( maxLoc.x, 0, toDraw.width, 0, (float) ofGetWidth());
            mouse.y = ofMap( maxLoc.y, 0, toDraw.height, 0, (float) ofGetHeight());
            ofEllipse(mouse, 20, 20);
        }
        
    }
    
    ofImage & getImage(){
        return toDraw;
    }
    
    ofxKinect & getKinect(){
        return kinect;
    }
    
    ofPixels colorPixels;
    int near, far;
    ColorTracker tracker;
    
    bool bFlipHoriz, bFlipVert, bFlipAxes;
    
    vector<ofPoint> quad;
    float thresh;
    ofEvent<ofPoint> cameraMove;
    
protected:
    cv::Point minLoc, maxLoc;
    
    ofImage toDraw;
    ofPixels scaledPixels;
    ofxKinect kinect;
    
    ofVec2f lastPoint;
};