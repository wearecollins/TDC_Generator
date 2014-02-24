//
//  ofxSurf.cpp
//
//  Created by Brett Renfer on 2/7/14.
//  Based on example from opencv docs:
//  http://docs.opencv.org/doc/tutorials/features2d/feature_homography/feature_homography.html

#include "ofxSurf.h"

//--------------------------------------------------------------
ofxSurf::ofxSurf(){
    bNeedToDetect = false;
    bNeedToUpdateSource = false;
}

//--------------------------------------------------------------
ofxSurf::~ofxSurf(){
    waitForThread();
    stopThread();
}

//--------------------------------------------------------------
void ofxSurf::setSource( ofPixelsRef & sourceImg ){
    targetPixelsSource = sourceImg;
    bNeedToUpdateSource = true;
}

//--------------------------------------------------------------
void ofxSurf::detect(ofPixelsRef & sceneImg ){
    targetPixelsScene = sceneImg;
    bNeedToDetect = true;
    
    if ( !isThreadRunning()){
#ifdef USE_GPU
#else
        detector = cv::SurfFeatureDetector(50, 3, 2, false);
#endif
        startThread();
    }
}

//--------------------------------------------------------------
void ofxSurf::threadedFunction(){
    while (isThreadRunning()){
        if ( bNeedToDetect ){
            // copy current pixels
            lock();
            bufferPixelsScene  = targetPixelsScene;
            unlock();
            
#ifdef USE_GPU
            {
                cv::gpu::SURF_GPU     detector;
                
                static cv::gpu::GpuMat src_mat;
                static cv::gpu::GpuMat scn_mat;
                
                src_mat = cv::gpu::GpuMat(bufferPixelsSource.getHeight(), bufferPixelsSource.getWidth(), ofxCv::getCvImageType(bufferPixelsSource), bufferPixelsSource.getPixels(), 0);
                scn_mat = cv::gpu::GpuMat(bufferPixelsScene.getHeight(), bufferPixelsScene.getWidth(), ofxCv::getCvImageType(bufferPixelsScene), bufferPixelsScene.getPixels(), 0);
                
                // detecting keypoints & computing descriptors
                cv::gpu::GpuMat keypoints1GPU, keypoints2GPU;
                cv::gpu::GpuMat descriptors1GPU, descriptors2GPU;
                detector(src_mat, cv::gpu::GpuMat(), keypoints1GPU, descriptors1GPU);
                detector(scn_mat, cv::gpu::GpuMat(), keypoints2GPU, descriptors2GPU);
                
                // matching descriptors
                cv::gpu::BruteForceMatcher_GPU< cv::L2<float> > matcher;
                cv::gpu::GpuMat trainIdx, distance;
                matcher.matchSingle(descriptors1GPU, descriptors2GPU, trainIdx, distance);
                
                // downloading results
                vector<cv::KeyPoint> keypoints1, keypoints2;
                vector<float> descriptors1, descriptors2;
                vector<cv::DMatch> matches;
                detector.downloadKeypoints(keypoints1GPU, keypoints1);
                detector.downloadKeypoints(keypoints2GPU, keypoints2);
                detector.downloadDescriptors(descriptors1GPU, descriptors1);
                detector.downloadDescriptors(descriptors2GPU, descriptors2);
                cv::gpu::BruteForceMatcher_GPU< cv::L2<float> >::matchDownload(trainIdx, distance, matches);
                
                // drawing the results
                lock();
                cv::drawMatches(src_mat, keypoints1, scn_mat, keypoints2, matches, img_matches);
                unlock();
            }
#else
            
            // update source if it has changed
            if ( bNeedToUpdateSource ){
                lock();
                bufferPixelsSource = targetPixelsSource;
                unlock();
                
                bNeedToUpdateSource = false;
                // source - make opencv mat
                src_mat = ofxCv::toCv(bufferPixelsSource);
                
                // source - detect key points
                detector.detect( src_mat, keypoints_object );
                
                // source - caculate descriptors
                extractor.compute( src_mat, keypoints_object, descriptors_object );
            }
            
            // clear keypoint vectors (prob not necessary?)
            
            keypoints_object.clear();
            keypoints_scene_buffer.clear();
            
            // scene - make opencv mat
            scn_mat = ofxCv::toCv(bufferPixelsScene);
            
            // scene - detect the keypoints using SURF Detector
            detector.detect( scn_mat, keypoints_scene_buffer );
            
            // scene - calculate descriptors (feature vectors)
            extractor.compute( scn_mat, keypoints_scene_buffer, descriptors_scene );
            
            //-- Step 3: Matching descriptor vectors using FLANN matcher
            //TODO: try brute force matcher?
            cv::FlannBasedMatcher matcher;
            matcher.match( descriptors_object, descriptors_scene, matchesBuffer );
            
            double max_dist = 0; double min_dist = 100;
            
            //-- Quick calculation of max and min distances between keypoints
            for( int i = 0; i < descriptors_object.rows; i++ )
            { double dist = matchesBuffer[i].distance;
                if( dist < min_dist ) min_dist = dist;
                if( dist > max_dist ) max_dist = dist;
            }
            
            ofLogVerbose()<<"-- Max dist : "<<max_dist<<"\n";
            ofLogVerbose()<<"-- Min dist : "<<min_dist<<"\n";
            
            //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
            std::vector< cv::DMatch > good_matches;
            
            for( int i = 0; i < descriptors_object.rows; i++ )
            { if( matchesBuffer[i].distance < 3*min_dist )
            { good_matches.push_back( matchesBuffer[i]); }
            }
            
            // hm...
            lock();
            cv::drawMatches( src_mat, keypoints_object, scn_mat, keypoints_scene_buffer,
                            good_matches, img_matches, cv::Scalar::all(-1), cv::Scalar::all(-1),
                            vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
            unlock();
            
            //-- Localize the object
            std::vector<cv::Point2f> obj;
            std::vector<cv::Point2f> scene;
            
            
            for( int i = 0; i < good_matches.size(); i++ )
            {
                //-- Get the keypoints from the good matches
                obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
                scene.push_back( keypoints_scene_buffer[ good_matches[i].trainIdx ].pt );
            }
            
            cv::Mat H = cv::findHomography( obj, scene, CV_RANSAC );
            
            //-- Get the corners from the image_1 ( the object to be "detected" )
            std::vector<cv::Point2f> obj_corners(4);
            obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( src_mat.cols, 0 );
            obj_corners[2] = cvPoint( src_mat.cols, src_mat.rows ); obj_corners[3] = cvPoint( 0, src_mat.rows );
            std::vector<cv::Point2f> scene_corners(4);
            
            cv::perspectiveTransform( obj_corners, scene_corners, H);
            
            //-- Draw lines between the corners (the mapped object in the scene - image_2 )
            cv::line( img_matches, scene_corners[0] + cv::Point2f( src_mat.cols, 0), scene_corners[1] + cv::Point2f( src_mat.cols, 0), cv::Scalar(0, 255, 0), 4 );
            cv::line( img_matches, scene_corners[1] + cv::Point2f( src_mat.cols, 0), scene_corners[2] + cv::Point2f( src_mat.cols, 0), cv::Scalar( 0, 255, 0), 4 );
            cv::line( img_matches, scene_corners[2] + cv::Point2f( src_mat.cols, 0), scene_corners[3] + cv::Point2f( src_mat.cols, 0), cv::Scalar( 0, 255, 0), 4 );
            cv::line( img_matches, scene_corners[3] + cv::Point2f( src_mat.cols, 0), scene_corners[0] + cv::Point2f( src_mat.cols, 0), cv::Scalar( 0, 255, 0), 4 );
            
            lock();
            std::swap(good_matches, currentMatches);
            std::swap(keypoints_scene_buffer, keypoints_scene);
            unlock();
            
#endif
            
            bNeedToDetect = false;
        }
        yield();
    }
}

//--------------------------------------------------------------
vector<cv::DMatch> ofxSurf::getCVMatches()
{
    return currentMatches;
}

//--------------------------------------------------------------
vector<ofVec2f> ofxSurf::getMatches()
{
    // quick copy of keypoints_scene (this should be double buffered)
    lock();
    std::vector<cv::KeyPoint> scenePts = keypoints_scene;
    unlock();
    
    vector<ofVec2f> sceneVec2fs;
    for( int i = 0; i < currentMatches.size(); i++ )
    {
        sceneVec2fs.push_back( ofxCv::toOf( scenePts[ currentMatches[i].trainIdx ].pt ) );
    }
    return sceneVec2fs;
}

//--------------------------------------------------------------
void ofxSurf::draw( int x, int y, int w, int h){
    lock();
    if ( img_matches.cols == 0 ){
        unlock();
        return;
    }
    ofxCv::toOf(img_matches, toOfImage);
    unlock();
    toOfImage.update();
    toOfImage.draw(x, y, w == -1 ? toOfImage.width : w, h == -1 ? toOfImage.height : h);
}
