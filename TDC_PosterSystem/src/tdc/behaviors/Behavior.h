//
//  Behavior.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/12/14.
//
//

#pragma once

#include "ofMain.h"
#include "TDCConstants.h"
#include "ofxLabFlexParticleSystem.h"
#include "CameraManager.h"

// PING PONG from shader particle ex
struct pingPongBuffer {
public:
    void allocate( int _width, int _height, int _internalformat = GL_RGBA, float _dissipation = 1.0f){
        // Allocate
        for(int i = 0; i < 2; i++){
            FBOs[i].allocate(_width,_height, _internalformat );
            FBOs[i].getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
        }
        
        // Clean
        clear();
        
        // Set everything to 0
        flag = 0;
        swap();
        flag = 0;
    }
    
    void swap(){
        src = &(FBOs[(flag)%2]);
        dst = &(FBOs[++(flag)%2]);
    }
    
    void clear(){
        for(int i = 0; i < 2; i++){
            FBOs[i].begin();
            ofClear(0,255);
            FBOs[i].end();
        }
    }
    
    ofFbo& operator[]( int n ){ return FBOs[n];}
    
    ofFbo   *src;       // Source       ->  Ping
    ofFbo   *dst;       // Destination  ->  Pong
private:
    ofFbo   FBOs[2];    // Real addresses of ping/pong FBO´s
    int     flag;       // Integer for making a quick swap
};

class TypeParticle;
class TypeParticleSystem;

class Behavior {
    
public:
    
    Behavior(){
        name = "undefined";
    }
    
    string getName(){
        return name;
    }
    
    void copySettings( Behavior * from ){
        intensity = from->intensity;
        timeFactor = from->timeFactor;
        mix = from->mix;
        scale = from->scale;
    }
    
    virtual void setMouse( ofVec2f mousePosition ){ mouse = mousePosition; };
    virtual void setup( lab::ParticleSystem::Container * c ){};
    
    virtual void update( TypeParticle * p ){};
    virtual void updateAll( lab::ParticleSystem::Container * c ){};
    virtual void copyMesh( ofMesh * m ){};
    
    // call before/after draw (duh)
    virtual void beginDraw(){}
    virtual void endDraw(){}
    virtual void draw(){}
    virtual void reload(){};
    
    // settings
    ofVec3f intensity;
    double   timeFactor;
    float    mix;
    float   scale;
    
    // yeesh
    ofMatrix4x4 homography;
    CameraManager * camera;
    float pointSize;
    float pointRandomization;
    ofVec2f screen;
    
protected:
    
    string name;
    ofVec2f mouse;
};
