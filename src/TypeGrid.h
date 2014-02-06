//
//  TypeGrid.h
//  TDC_PosterTest
//
//  Created by Brett Renfer on 2/4/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxSvg.h"

class TypeGrid {

public:
    
    void load( string svg_to_load ){
        lastSpeed = 0;
        path.load( svg_to_load );
        
        int inc = 2;
        
        string file = svg_to_load.substr(0, svg_to_load.length()-4) +"_mesh";
        cout << file << endl;
        
        mesh.load( file );
        
        bool bLoad = mesh.getVertices().size() > 0;
        
        
        if ( !bLoad ){
            mesh.setupIndicesAuto();
            
            for ( int x=0; x<ofGetWidth(); x += inc){
                for ( int y=0; y<ofGetHeight(); y += inc){
                    ofVec2f p = ofVec2f(x,y);
                    
                    bool bGood = false;
                    for ( int i=0; i<path.getNumPath(); i++){
                        ofPath pz = path.getPathAt(i);
                        pz.setPolyWindingMode(OF_POLY_WINDING_ODD);
                        vector<ofPolyline> lines = pz.getOutline();
                        bool bInMainShape = ofInsidePoly( p, lines[0].getVertices());
                        bool bInSubShape  = false;
                        if ( bInMainShape && lines.size() > 1 ){
                            for ( int j=1; j<lines.size(); j++){
                                if ( lines[j].size() > 0 ){
                                    bInSubShape = ofInsidePoly( p, lines[j].getVertices());
                                    if ( bInSubShape ) break;
                                }
                            }
                        }
                        if ( bInMainShape && !bInSubShape ) bGood = true;
                       
                        if (bGood) break;
                    }
                    
                    if ( bGood ){
                        mesh.addVertex(p);
                        mesh.addColor( ofColor(255));
                    }
                }
            }
            mesh.save( file, false );
        }
        
        mesh.setMode( OF_PRIMITIVE_POINTS );
        offset = ofRandom(10.0f, 50.0f);
        
        for ( int i=0; i<mesh.getVertices().size(); i++){
            sizes.push_back( ofRandom(0.1,3.0f));
            offsets.push_back(ofVec2f(0,0));
        }
        
        resetColor();
        
    }
    
    void draw(){
        
        ofMesh m = mesh;
        
        for ( int i=0; i<m.getVertices().size(); i++){
            float valX = ofSignedNoise(m.getVertices()[i].x + ofGetElapsedTimeMillis() * .0001, m.getVertices()[i].y + (ofGetElapsedTimeMillis() * .0002));
            float valY = ofSignedNoise(m.getVertices()[i].x + ofGetElapsedTimeMillis() * .0002, m.getVertices()[i].y + (ofGetElapsedTimeMillis() * .0001));
            valX *= sin(ofGetElapsedTimeMillis() * .0001) * offset;
            valY *= sin(ofGetElapsedTimeMillis() * .0002) * offset;
            m.getVertices()[i] += ofVec2f(valX, valY);
            m.getVertices()[i] += offsets[i];
            
            // ugh
            ofSetColor( m.getColor(i));
            ofEllipse(m.getVertex(i), sizes[i], sizes[i]);
        }
        
        //m.draw();
        m.drawWireframe();
        
    }
    
    ofPoint last;
    float lastSpeed;
    
    void mouseMoved( int x, int y ){
        ofPoint p(x,y);
        lastSpeed = lastSpeed * .9 + last.distance(p) * .1;
        last = p;
        for ( int i=0; i<mesh.getVertices().size(); i++){
            float dist = p.distance(mesh.getVertex(i));
            if ( dist < 1000 ){
                offsets[i] = (mesh.getVertex(i) - p).normalize();
                offsets[i] *= lastSpeed;
                //offsets[i] = (mesh.getVertex(i) - p).normalize();
                //offsets[i] *= dist;
            } else {
                offsets[i].set(0,0);
            }
        }
        
    }
    
    void resetColor(){
        ofColor c = ofColor( ofRandom(255), ofRandom(255), ofRandom(255), 150.0f );
        c.setSaturation(255);
        for ( int i=0; i<mesh.getVertices().size(); i++){
            ofColor l = ofColor(c);
            l.setHue( c.getHue() + ofSignedNoise( mesh.getVertex(i).x, mesh.getVertex(i).y ) * 100.0f );
            if ( mesh.getColors().size() <= i ){
                mesh.addColor( ofColor(255));
            }
            mesh.setColor( i, l );
        }
    }
    
protected:
    
    float offset;
    ofxSVG path;
    ofMesh mesh;
    vector<float> sizes;
    vector<ofVec2f> offsets;
};
