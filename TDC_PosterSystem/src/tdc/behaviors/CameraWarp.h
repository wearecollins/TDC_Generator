//
//  CameraWarp.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/19/14.
//
//

#pragma once

#include "Behavior.h"
#include "ofxCv.h"

class CameraWarp : public Behavior {
public:
    
    void setup( ofxLabFlexParticleSystem::Container * c ){
        // load shaders
        knockBlack.load("","shaders/knockoutBlack.frag");
        bumpShader.load("shaders/bumpMap.vert","");
        // create accumulation buffers
        accum = 0.0;
    };
    
    void update( TypeParticle * p ){
        (*p) += ofVec2f( ofSignedNoise(p->getSeedPosition().x + p->getOffsetX() + (ofGetElapsedTimeMillis() * timeFactor)) * intensity.x, ofSignedNoise(p->getSeedPosition().y + p->getOffsetY() + (ofGetElapsedTimeMillis() * timeFactor)) * intensity.y);
    }
    
//    virtual void update( TypeParticle * p ){};
    void updateAll( ofxLabFlexParticleSystem::Container * c ){
        if ( !last.isAllocated()) return;
        ofxLabFlexParticleSystem::Iterator it = c->begin();
        ofVec3f lmp(lastMaxLoc.x, lastMaxLoc.y, 0);
        ofVec3f mp(maxLoc.x, maxLoc.y);
        mp.x = ofMap( mp.x, 0, last.width, 0, (float) ofGetWidth());
        mp.y = ofMap( mp.y, 0, last.height, 0, (float) ofGetHeight());
        lmp.x = ofMap( lmp.x, 0, last.width, 0, (float) ofGetWidth());
        lmp.y = ofMap( lmp.y, 0, last.height, 0, (float) ofGetHeight());
        
        float mouseMass = 100.0f;
        if ( lmp.distance(mp) < 10 ) return;
        
        for( it = c->begin(); it != c->end(); ++it )
        {
            TypeParticle * p = (TypeParticle *) it->second;
            float distance = ofDist(mp.x, mp.y, p->x, p->y);
            
            if( distance > 100 ) {
                continue;
            }
            
            // ofVec2f - ofVec2f
            ofVec3f diff = mp -( *p );
            
            diff.normalize();
            
            // http://www.grantchronicles.com/astro09.htm
            // this is a really lose interpretation.. like not very close
            float force = mouseMass * p->mass / MAX(1, distance);
            float accel = force / p->mass;
            
            p->acceleration += diff * accel;
        }
        lastMinLoc = minLoc;
        lastMaxLoc = maxLoc;
    };
//    virtual void copyMesh( ofMesh * m ){};
    
    void beginDraw(){
        static ofImage diffImage;
        diffImage = camera->getImage();
        
        //ofxCv::blur(diffImage, diffImage, 77 );
        diffImage.update();
        
        if ( !currentDiff.isAllocated()){
            currentDiff = diffImage;
            accumulation.allocate( currentDiff.width, currentDiff.height );
            
            accumulation.begin();
            ofClear(0,0,0,0);
            accumulation.end();
        }
        if ( last.isAllocated()){
            
            // hm OK
            //ofxCv::absdiff(diffImage, last, currentDiff);
            currentDiff = diffImage;
            currentDiff.update();
            
            static cv::Mat cameraMat;
            cameraMat = ofxCv::toCv(currentDiff);
            
            cv::Point minLocTemp, maxLocTemp;
            static cv::Rect rect;
            rect.x      = 0;
            rect.y      = 0;
            rect.width  = currentDiff.width;
            rect.height = currentDiff.height;
            cv::Mat roiMat(cameraMat, rect);
            cv::minMaxLoc( roiMat, &minVal, &maxVal, &minLocTemp, &maxLocTemp, cv::Mat());
            
            
            //float last_depthval = lastDepth[maxLoc.y * camera->getKinect().getWidth() + maxLoc.x];
            
            //float depthval = camera->getKinect().getRawDepthPixelsRef()[maxLoc.y * camera->getKinect().getWidth() + maxLoc.x];
            float last_color = last.getColor(maxLoc.x, maxLoc.y).r;
            float color = currentDiff.getColor(maxLoc.x, maxLoc.y).r;
            
            if ( lastMaxVal > 50 ){
                //ofxCv::threshold( currentDiff, color * .9);
                
                //ofxCv::subtract(currentDiff, 150, currentDiff);
                //ofxCv::threshold( currentDiff, 150 );//maxVal > 100 ? maxVal : 100 );
                //currentDiff.update();
                
                accumulation.begin();
                ofPushStyle();
                ofEnableAlphaBlending();
                //knockBlack.begin();
                //knockBlack.setUniformTexture("tex", currentDiff.getTextureReference(), 0);
                //cout << (maxLoc.x - maxLocTemp.x) <<":"<<maxLoc.x <<":"<<maxLoc.y << endl;
                
                ofSetColor(lastMaxVal,10);
                ofEllipse( maxLoc.x, maxLoc.y, ofMap(color, 0, 255, 25,100), ofMap(color, 0, 255, 25,100) );
                
                //knockBlack.end();
                ofPopStyle();
                accumulation.end();
                accum = 0.0;
            } else {
                accumulation.begin();
                ofPushStyle();
                ofSetColor(0,0,0,accum);
                ofRect(0,0,accumulation.getWidth(), accumulation.getHeight());
                ofPopStyle();
                accumulation.end();
                accum += .001;
            }
            minLoc = minLoc * .9 + minLocTemp * .1;
            maxLoc = maxLoc * .9 + maxLocTemp * .1;
        }
        
        last = diffImage;
        lastDepth = camera->getKinect().getRawDepthPixelsRef();
        lastMaxVal = lastMaxVal * .75 + maxVal * .25;
        
        bumpShader.begin();
        bumpShader.setUniform2f("depthDims", 320.0f, 240.0f);
        bumpShader.setUniform2f("screenDims", (float) ofGetWidth(), (float) ofGetHeight());
        bumpShader.setUniform1f("extrusion", mix * 1000.0f);
        if ( camera != NULL ){
            bumpShader.setUniformTexture("depthTex", accumulation.getTextureReference(), 0);
        } else {
            cout << "CAMERA IS NULL!"<<endl;
        }
    }
    void endDraw(){
        bumpShader.end();
    }
    void draw(){
        currentDiff.draw(100,ofGetHeight()/2, 160, 120);
        accumulation.draw(100,ofGetHeight()/2 + 120, 160, 120);
//        ofEllipse( maxLoc.x, maxLoc.y, 5, 5 );
    }
    
protected:
    
    ofImage currentDiff;
    ofImage last;
    ofFbo   accumulation;
    ofShortPixels lastDepth;
    cv::Point minLoc, maxLoc;
    cv::Point lastMinLoc, lastMaxLoc;
    double minVal, maxVal;
    double lastMaxVal;
    float accum;
    
    ofShader knockBlack;
    ofShader bumpShader;
};
