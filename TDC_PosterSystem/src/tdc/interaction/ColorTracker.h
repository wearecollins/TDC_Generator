//
//  ColorTracker.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/20/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "CameraManager.h"
#include "ofxCv.h"

class ColorTracker : public ofThread {
public:
    
    ColorTracker(){}
    
    void setup(){
        ofAddListener( ofEvents().mouseMoved, this, &ColorTracker::mouseMoved );
        ofAddListener( ofEvents().mousePressed, this, &ColorTracker::mousePressed );
        
        // add 3x color trackers, 3 colors
        trackingColors.push_back( ofColor(0,0,255));
        trackingColors.push_back( ofColor(0,255,0));
        trackingColors.push_back( ofColor(255,0,255));
        
        colorTrackers.push_back( ofxCv::ContourFinder() );
        colorTrackers.push_back( ofxCv::ContourFinder() );
        colorTrackers.push_back( ofxCv::ContourFinder() );
        
        threshold = 50;
        currentColor = 0;
        bDraw = false;
        bChoose = false;
        
        ofxXmlSettings settings;
        if ( settings.loadFile("settings/colortracker.xml")){
            for ( int i=0; i<colorTrackers.size(); i++){
                settings.pushTag("color", i );
                trackingColors[i].r = settings.getValue("r", trackingColors[i].r);
                trackingColors[i].g = settings.getValue("g", trackingColors[i].g);
                trackingColors[i].b = settings.getValue("b", trackingColors[i].b);
                settings.popTag();
            }
        } else {
            saveSettings();
        }
        
        for ( int i=0; i<colorTrackers.size(); i++){
            colorTrackers[i].setTargetColor( trackingColors[i], ofxCv::TRACK_COLOR_RGB );
            colorTrackers[i].setMinAreaRadius(10);
            colorTrackers[i].setMaxAreaRadius(200);
            numBlobsBuff.push_back(0);
        }
        
        minRad = 10;
        maxRad = 200;
        
        //trackingImage.setUseTexture(true);
        
        cout <<"START"<<endl;
        startThread();
        roi.x = 0;
        roi.y = 0;
        roi.width = 640;
        roi.height = 480;
    }
    
    
    void threadedFunction(){
        while( isThreadRunning()){
            if ( trackingImage.isAllocated()){
                lock();
                trackingCv = ofxCv::toCv(trackingImage);
                trackingCvBuff = cv::Mat(trackingCv, roi);
//                trackingCv.setRoi();
                for ( int i=0; i<colorTrackers.size(); i++){
                    colorTrackers[i].setTargetColor( trackingColors[i], ofxCv::TRACK_COLOR_RGB );
                    colorTrackers[i].setMinAreaRadius(minRad);
                    colorTrackers[i].setMaxAreaRadius(maxRad);
                    colorTrackers[i].setThreshold(threshold);
                    colorTrackers[i].findContours(trackingCvBuff);
                    numBlobsBuff[i] = colorTrackers[i].size();
                }
                numBlobs = numBlobsBuff;
                unlock();
            }
            sleep( 16.667 );
        }
    }
    
    void updateCamera( ofPixelsRef & pix ){
        trackingImage.setFromPixels( pix );
        trialColor = trackingImage.getPixelsRef().getColor(mouse.x, mouse.y);
        
        roi.x = rectXY.x;
        roi.y = rectXY.y;
        roi.width = rectWH.x;
        roi.height = rectWH.y;
        
        // check for crash...
        if ( !isThreadRunning() ){
            startThread();
        }
    }
    
    void saveSettings(){
        
        ofxXmlSettings settings;
        settings.addTag("color");
        settings.addTag("color");
        settings.addTag("color");
        
        for ( int i=0; i<colorTrackers.size(); i++){
            settings.pushTag("color", i );
            settings.addValue("r", trackingColors[i].r);
            settings.addValue("g", trackingColors[i].g);
            settings.addValue("b", trackingColors[i].b);
            settings.popTag();
        }
        
        settings.saveFile("settings/colortracker.xml");
    }
    
    // debug draw for picking colors
    void draw(){
        if ( !bDraw || !trackingImage.isAllocated() ) return;
        ofPushStyle();
        trackingImage.draw(0,0);
        ofNoFill();
        ofRect(roi.x, roi.y, roi.width, roi.height);
        ofFill();
        for ( int i=0; i<colorTrackers.size(); i++){
            colorTrackers[i].draw();
        }
        ofSetColor(trialColor);
        int x = 640;
        int y = 0;
        ofRect(x,y,20,20);
        
        y+= 20;
        
        for ( int i=0; i<colorTrackers.size(); i++){
            ofSetColor(trackingColors[i]);
            ofRect(x,y,20,20);
            y+= 20;
        }
        ofPopStyle();
    }
    
    void mouseMoved( ofMouseEventArgs & m ){
        if ( !bDraw || !bChoose ) return;
        if (m.x < trackingImage.width && m.y < trackingImage.height){
            mouse.set(m.x,m.y);
        }
    }
    
    void mousePressed( ofMouseEventArgs & m ){
        if ( !bDraw || !bChoose ) return;
        if (m.x < trackingImage.width && m.y < trackingImage.height){
            trackingColors[currentColor] = trialColor;
            saveSettings();
        }
    }
    
    int getNumberByColor( int n ){
        if ( numBlobs.size() <= n ){
            return 0;
        }
        return numBlobs[n];
    }
    
    // add ofEvent to broadcast on found color?
    
    vector<ofColor> trackingColors;
    int currentColor;
    ofColor trialColor;
    
    ofPoint mouse;
    
    float minRad, maxRad;
    
    bool bDraw, bChoose;
    
    vector<ofxCv::ContourFinder> colorTrackers;
	float threshold;
	ofxCv::TrackingColorMode trackingColorMode;
    ofImage trackingImage;
    cv::Mat trackingCv, trackingCvBuff;
    cv::Rect roi; // TO-DO
    ofPoint rectXY;
    ofPoint rectWH;
    
protected:
    vector<int> numBlobs, numBlobsBuff;
};
