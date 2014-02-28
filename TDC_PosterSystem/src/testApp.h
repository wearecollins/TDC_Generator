#pragma once

#include "ofMain.h"
#include "WeatherColors.h"
#include "TypeParticleSystem.h"
#include "ofxUI.h"

#include "ofxSpacebrew.h"

// projection map + sensing
#include "ofxMaskEditor.h"
#include "ofxSurf.h"

class TrackingImage : public ofImage
{
public:
    void loadAndScale( ofPixelsRef & pix, float scale = 1.0 ){
        setUseTexture(false);
        ofImage tempRgbCvt;
        tempRgbCvt.setUseTexture(false);
        tempRgbCvt.setImageType( OF_IMAGE_COLOR );
        setFromPixels(pix);
        ofxCv::convertColor(tempRgbCvt, *this, pix.getNumChannels() == 3 ? CV_RGB2GRAY : CV_RGBA2GRAY );
        resize(width * scale, height * scale);
    }
    void loadAndScale( string path, float scale = 1.0 ){
        cout << "LOADING "<<path<<endl;
        ofImage tempRgb;
        tempRgb.loadImage(path);
        ofxCv::convertColor(tempRgb, *this, CV_RGB2GRAY);
        resize(width * scale, height * scale);
    }
};

class testApp : public ofBaseApp{

	public:
		void setup();
        void setupSpacebrew();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void renderBackground();
        void renderParticles( bool bHomography );
    
        bool bRandomize;
        void randomize();
    
        void onGui( ofxUIEventArgs & e );
    
        // UI canvas to render data bar
        ofxUICanvas * envGui;
        ofxUICanvas * commGui;
        ofxUICanvas * collabGui;
        void setupDataBar();
        void drawDataBar( bool bBottom = false );
    
        ofImage mask;
    
        // particle system
        TypeParticleSystem particles;
    
        int drawMode;
    
        // masking + sensing
        bool bEditingMask;
        ofxMaskEditor maskEditor;
        map<string, ofPtr<ofxSurf> > surfers;
        map<string, TrackingImage>   surfImages;
        TrackingImage camera;
    
        // image saving logic
        ofFbo toSave;
        ofFbo toSavePoster;
        ofPixels pix;
    
        // poster bg
        ofMesh posterMesh;
    
        // poster source points
        ofxSVG posterSrc;
    
        // type
        map<string, ofxSVG> type;
    
        // tweakin'
        vector<ofxUISuperCanvas *> guis;
        ofxUITabBar * gui;
    
        // interaction
        Spacebrew::Connection spacebrew;
        void onMessage( Spacebrew::Message & m );
};
