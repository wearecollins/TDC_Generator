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
    kinect.setRegistration();
    kinect.init();
    kinect.open();
    
	/*if (ir.setup(*device)) // only for xtion device (OpenNI2-FreenectDriver issue)
	{
		ir.setSize(640, 480);
		ir.setFps(30);
		ir.start();
	}*/
    
    near = 50;
    far  = 10000;
	
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
    kinect.update();
    // scale my pix
    if ( kinect.isFrameNew()  ){
        colorSmall.setFromPixels( kinect.getPixelsRef() );
        colorSmall.resize(kinect.getWidth() * .5, kinect.getHeight() * .5);
    }
}

//--------------------------------------------------------------
void FlipSensor::draw()
{
    ofPushMatrix();
    if ( colorSmall.getWidth() > 0 ){
        colorSmall.draw(640,0);
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