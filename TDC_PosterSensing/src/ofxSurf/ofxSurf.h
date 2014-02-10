//
//  ofxSurf.h
//  Dependencies: ofxCv
//
//  Created by Brett Renfer on 2/7/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxCv.h"

//#define USE_GPU
#ifdef USE_GPU
#include "opencv2/gpu/gpu.hpp"
#endif

class ofxSurf : protected ofThread {
public:
    
    ofxSurf();
    ~ofxSurf();
    
// TODO: add constructor for other-than-default params
    
    // this sucks, had to make it this to play nice with ofxNI2
    void setSource( ofPixelsRef & source );
    void detect( ofPixelsRef & scene );
    
    void draw( int x = 0, int y = 0, int w = -1, int h = -1);
    
    vector<cv::DMatch> getCVMatches();
    vector<ofVec2f> getMatches();
    
protected:
    
    ofPixels targetPixelsSource, targetPixelsScene;
    ofPixels bufferPixelsSource, bufferPixelsScene;
    
    // debug drawing of current matches
    cv::Mat img_matches;
    
    // current matching points
    std::vector< cv::DMatch > currentMatches, matchesBuffer;
    
    void threadedFunction();
#ifndef USE_GPU
    cv::SurfFeatureDetector detector;
#endif
    
    cv::SurfDescriptorExtractor extractor;
    
private:
    cv::Mat src_mat;
    cv::Mat scn_mat;
     
    std::vector<cv::KeyPoint> keypoints_object;
    std::vector<cv::KeyPoint> keypoints_scene, keypoints_scene_buffer;
    cv::Mat descriptors_object;
    cv::Mat descriptors_scene;
    
    ofImage toOfImage;
    
    bool bNeedToDetect;
    bool bNeedToUpdateSource;
};