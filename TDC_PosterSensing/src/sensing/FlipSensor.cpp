//
//  FlipSensor.cpp
//  TDC_PosterSensing
//
//  Created by Brett Renfer on 2/7/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "FlipSensor.h"
#include "utils/DepthRemapToRange.h"

//--------------------------------------------------------------
void FlipSensor::setup()
{
    ofxNI2::init();
    device = new ofxNI2::Device();
    device->setup();
    
    if (depth.setup(*device))
	{
		depth.setSize(640, 480);
		depth.setFps(30);
		depth.start();
        
        ofPtr<ofxNI2::Grayscale> shader = depth.getShader<ofxNI2::Grayscale>();
        shader->setNear( 50 );
        shader->setFar( 2000 );
	}
    
	/*if (ir.setup(*device)) // only for xtion device (OpenNI2-FreenectDriver issue)
	{
		ir.setSize(640, 480);
		ir.setFps(30);
		ir.start();
	}*/
    
    near = 50;
    far  = 10000;
	
   if (color.setup(*device)) // only for kinect device
   {
        color.setSize(640, 480);
        color.setFps(60);
        color.start();
   }
    
    // load images
    ofDirectory dir;
    int n = dir.listDir("posters");
    for (int i=0; i<n; i++){
        surfers.push_back(ofPtr<ofxSurf>(new ofxSurf()) );
        images.push_back(TrackingImage());
        images[i].loadAndScale(dir.getPath(i));
        surfers[i]->setSource(images[i].getPixelsRef());
    }
    
    bTrack = false;
}

//--------------------------------------------------------------
void FlipSensor::update()
{
    // scale my pix
    if ( depth.getWidth() > 0 ){
        ofxNI2::depthRemapToRange(depth.getPixelsRef(), scaledPixels, near, far, false);
        toDraw.setFromPixels(scaledPixels);
        
        colorSmall.setFromPixels( color.getPixelsRef() );
        colorSmall.resize(color.getWidth() * .5, color.getHeight() * .5);
    }
}

//--------------------------------------------------------------
void FlipSensor::draw()
{
    ofPushMatrix();
    
    if ( depth.getWidth() > 0 ){
        depth.draw();
        //toDraw.setFromPixels(depth.getPixelsRef(near, far));
        //toDraw.update();
        toDraw.draw(0, 480);
    }
    if ( color.getWidth() > 0 ){
        color.draw(640,0);
        // uh
        if ( bTrack ){
            ofPushMatrix();
            for (int i=0; i<images.size(); i++){
                surfers[i]->detect(colorSmall.getPixelsRef());
                surfers[i]->draw(640,480 + colorSmall.height * i);
//                cout << surfers[i]->getMatches().size() << " : "<< i << endl;
                // i need to figure this out!
                if ( surfers[i]->getMatches().size() < 50 ){
                    cout << "we're on poster "<< i + 1 << endl;
                }
            }
            ofPopMatrix();
        }
    }
    if ( ir.getWidth() > 0 ){
        ir.draw(640,0);
    }
    ofPopMatrix();
    
    /*
    if ( mouseX > 0 && mouseX < depth.getWidth() && mouseY > 0 && mouseY < depth.getHeight() ){
        string val = ofToString( depth.getPixelsRef().getColor( mouseX, mouseY) );
        ofDrawBitmapString(val, mouseX, mouseY);
    }
    */
    
}

//--------------------------------------------------------------
void FlipSensor::setNear( float _near )
{
    near = _near;
//    ofPtr<ofxNI2::Grayscale> shader = depth.getShader<ofxNI2::Grayscale>();
//    shader->setNear(near);
}

//--------------------------------------------------------------
void FlipSensor::setFar( float _far )
{
    far = _far;
//    ofPtr<ofxNI2::Grayscale> shader = depth.getShader<ofxNI2::Grayscale>();
//    shader->setFar(far);
}