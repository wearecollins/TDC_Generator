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
    
    int bpp = inputImage.getPixelsRef().getNumChannels();
    
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
        cameraSkew = 0.0; // -1 to 1
        
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
        
        blobTracker.setMinAreaRadius(25);
        blobTracker.setMaxAreaRadius(200);
        blobTracker.getTracker().setPersistence(120);
        
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
            
            
            if ( cameraSkew != 0.0 ){
                for ( int x = 0; x <toDraw.width; x++){
                    for ( int y = 0; y <toDraw.height; y++){
                        ofColor c = toDraw.getPixelsRef().getColor(x,y);
                        float m = 0;
                        if ( cameraSkew > 0 ){
                            float s = ofMap(y, 0, toDraw.height, 0, 1.0);
                            m = s * cameraSkew;
                        } else {
                            float s = ofMap(y, 0, toDraw.height, 1.0, 0.0);
                            m = s * -cameraSkew;
                        }
                        c.r *= m;
                        c.g *= m;
                        c.b *= m;
                        toDraw.getPixelsRef().setColor(x,y,c);
                    }
                }
            }
            toDraw.update();
            
            //getQuadSubImage(toDraw, toDraw, quad, toDraw.getPixelsRef().getImageType());
            toDraw.update();
            
            ofxCv::threshold(toDraw, thresh);
            toDraw.update();
            blobTracker.findContours(toDraw);
            
            colorPixels.setFromPixels( kinect.getPixels(), kinect.getWidth(), kinect.getHeight(), kinect.getPixelsRef().getNumChannels());
            tracker.updateCamera( kinect.getPixelsRef() );
            
            /*double minVal = 0, maxVal = 0;
            static cv::Rect rect;
            rect.x      = 0;
            rect.y      = 0;
            rect.width  = toDraw.width;
            rect.height = toDraw.height;
            cv::Mat cameraMat = ofxCv::toCv(toDraw);
            cv::Mat roiMat(cameraMat, rect);
            cv::minMaxLoc( roiMat, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
            */
            
            if ( blobTracker.size() > 0 ){
                
                ofVec2f mouse;
                cv::Rect r = blobTracker.getBoundingRect(0);
                mouse.y = ofMap( bFlipHoriz ? r.x + r.width : r.x, 0, toDraw.width, quad[0].y, quad[1].y);
                mouse.x = ofMap( r.y, 0, toDraw.height, quad[0].x, quad[1].x);
                
                if ( lastPoint.distance(mouse) == 0) return;
                
                ofPoint args;
                args.x = mouse.x;
                args.y = mouse.y;
                
                ofNotifyEvent(cameraMove, args );
                lastPoint.set(args.x, args.y);
            }
            
        }
    }
    
    void draw( int x = 0, int y = 0, int width=160, int height=120){
        if ( toDraw.isAllocated()){
            toDraw.draw(x,y,width,height);
            
            //ofEllipse(lastPoint, 20, 20);
            ofPushMatrix();
            ofTranslate(x,y);
            ofScale( (float) width/toDraw.width, (float) height/toDraw.height);
            blobTracker.draw();
            
            ofPushStyle();
            if ( blobTracker.size() > 0 ){
                ofNoFill();
                ofSetColor(255,0,0);
                cv::Rect r = blobTracker.getBoundingRect(0);
                ofRect(r.x,r.y,r.width,r.height);
            }
            ofPopStyle();
            ofPopMatrix();
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
    
    float cameraSkew;
    ofVec2f lastPoint;
    float xMap, yMap;
    
protected:
    ofxCv::ContourFinder blobTracker;
    cv::Point minLoc, maxLoc;
    
    ofImage toDraw;
    ofPixels scaledPixels;
    ofxKinect kinect;
    
};