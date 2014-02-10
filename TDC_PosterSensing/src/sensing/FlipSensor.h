//
//  FlipSensor.h
//  TDC_PosterSensing
//
//  Created by Brett Renfer on 2/7/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "ofxNI2.h"
#include "ofxCv.h"
#include "ofxSurf.h"

class TrackingImage : public ofImage
{
public:
    void loadAndScale( string path, float scale = .5 ){
        ofImage tempRgb;
        tempRgb.loadImage(path);
        ofxCv::convertColor(tempRgb, *this, CV_RGB2GRAY);
        resize(width * scale, height * scale);
    }
};

class FlipSensor
{
public:
    
    void setup();
    void update();
    void draw();
    void setNear( float near );
    void setFar( float far );
    
    void startDetecting(){
        bTrack = true;
    }
    
    void mouse( int x, int y ){
        mouseX = x;
        mouseY = y;
    }
    
protected:
    
    bool bTrack;
    float near, far;
    int mouseX, mouseY;
    
    ofPixels            scaledPixels;
    ofImage             toDraw, colorSmall;
    
    ofxCv::ContourFinder contourFinder;
    ofxNI2::Device      * device;
    ofxNI2::ColorStream color;
    ofxNI2::IrStream    ir;
    ofxNI2::DepthStream depth;
    
    vector<ofPtr<ofxSurf> > surfers;
    vector<TrackingImage>   images;
    
};

