//
//  TypeMesh.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/10/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxSvg.h"
#include "ofxXmlSettings.h"

class TypeMesh {
    
public:
    
    TypeMesh(){
        SUFFIX = "_shape";
    }
    
    void load( string svg_to_load ){
        lastSpeed = 0;
        path.load( svg_to_load );
        
        // sets up grid
        
        int inc = 2;
        
        string file = "meshes/" + svg_to_load.substr(0, svg_to_load.length()-4) + SUFFIX;
        string xmlFile = "meshes/" + svg_to_load.substr(0, svg_to_load.length()-4) +SUFFIX + ".xml";
        
        mesh.load( file );
        
        bool bLoad = mesh.getVertices().size() > 0;
        
        if ( !bLoad ){
            cout << "CREATING MESH "<<endl;
            mesh.setupIndicesAuto();
            
            for ( int i=0; i<path.getNumPath(); i++){
                letters.push_back( vector<ofVec2f>() );
            }
            regenerateGrid( inc );
            cout << "SAVING FILE "<<endl;
            mesh.save( file, false );
            ofxXmlSettings lettersXml;
            for (int i=0; i<letters.size(); i++){
                lettersXml.addTag("letter");
                lettersXml.pushTag("letter", i);
                for (int j=0; j<letters[i].size(); j++){
                    lettersXml.addTag("point");
                    lettersXml.pushTag("point", j);
                    lettersXml.addValue("x", letters[i][j].x);
                    lettersXml.addValue("y", letters[i][j].y);
                    lettersXml.popTag();
                }
                lettersXml.popTag();
            }
            lettersXml.saveFile(xmlFile);
        } else {
            ofxXmlSettings lettersXml;
            lettersXml.loadFile( xmlFile );
            
            for ( int i=0; i<path.getNumPath(); i++){
                letters.push_back( vector<ofVec2f>() );
            }
            
            int n = lettersXml.getNumTags("letter");
            for (int i=0; i<n; i++){
                lettersXml.pushTag("letter", i);
                int o = lettersXml.getNumTags("point");
                for (int j=0; j<o; j++){
                    letters[i].push_back( ofVec2f() );
                    lettersXml.pushTag("point", j);
                    letters[i].back().x = lettersXml.getValue("x", 0.0);
                    letters[i].back().y = lettersXml.getValue("y", 0.0);
                    
                    lettersXml.popTag();
                }
                lettersXml.popTag();
            }
        }
        
        mesh.setMode( OF_PRIMITIVE_POINTS );
        offset = ofRandom(10.0f, 50.0f);
        
        for ( int i=0; i<mesh.getVertices().size(); i++){
            sizes.push_back( ofRandom(0.1,3.0f));
            offsets.push_back(ofVec2f(0,0));
            occupied.push_back(false);
        }
        
        resetColor();
        
    }
    
    void draw(){
        mesh.drawWireframe();
        
    }
    
    ofPoint last;
    float lastSpeed;
    
    ofMesh & getMesh()
    {
        return mesh;
    }
    
    void occupyIndex( int index )
    {
        occupied[index] = true;
    }
    
    bool isOccupied( int index )
    {
        return occupied[index];
    }
    
    bool isFull(){
        return std::find(occupied.begin(), occupied.end(), false) == occupied.end();
    }
    
    // SVG ACCESS
    
    ofxSVG & getSVG(){
        return path;
    }
    
    // this SHOULD be consistent, depending on how we make these things...
    int getNumLetters()
    {
        return path.getNumPath();
    }
    
    // returns OUTLINE of letter
    vector<ofPolyline> getLetter( int index )
    {
        if ( index >= path.getNumPath() ){
            ofLogError()<<"Index past length of SVG";
            return vector<ofPolyline>();
        }
        ofPath pz = path.getPathAt(index);
        pz.setPolyWindingMode(OF_POLY_WINDING_ODD);
        vector<ofPolyline> lines = pz.getOutline();
        return lines;
    }
    
    // returns vector of points inside specified letter
    vector<ofVec2f> getPointsInLetter( int index )
    {
        if ( index >= path.getNumPath() ){
            ofLogError()<<"Index past length of SVG, returning empty vector";
            return vector<ofVec2f>();
        }
        return letters[index];
    }
    
    bool pointIsInsideLetter( int whichLetter, int x, int y )
    {
        return pointIsInsideLetter( whichLetter, ofVec2f(x,y));
    }
    
    bool pointIsInsideLetter( int whichLetter, ofVec2f pt )
    {
        if ( whichLetter >= path.getNumPath() ){
            ofLogError()<<"Index past length of SVG, returning FALSE";
            return false;
        }
        
        bool bGood = false;
        vector<ofPolyline> letter = getLetter( whichLetter );
        
        bool bInMainShape = ofInsidePoly( pt, letter[0].getVertices());
        bool bInSubShape  = false;
        if ( bInMainShape && letter.size() > 1 ){
            for ( int j=1; j<letter.size(); j++){
                if ( letter[j].size() > 0 ){
                    bInSubShape = ofInsidePoly( pt, letter[j].getVertices());
                    if ( bInSubShape ) break;
                }
            }
        }
        return bInMainShape && !bInSubShape;
    }
    
    // tricky one: get which letter a point resides in
    int getLetterByParticle( int particleIndex )
    {
        ofVec2f p = mesh.getVertex( particleIndex );
        
        for (int i=0; i<letters.size(); i++){
            for (int j=0; j<letters[i].size(); j++){
                if ( letters[i][j].distance( p ) < 2 ){
                    return i;
                }
            }
        }
        ofLogError()<<"Not found :( "<<particleIndex;
        return 0;
    }
    
    // even tricker: letter interior or exterior
    bool isParticleInterior( int particleIndex )
    {
        ofVec2f p = mesh.getVertex( particleIndex );
        
        for (int i=0; i<letters.size(); i++){
            for (int j=0; j<letters[i].size(); j++){
                if ( letters[i][j].distance( p ) < 2 ){
                    return j == 0;
                }
            }
        }
        ofLogError()<<"Not found :( "<<particleIndex;
        return false;
    }
    
    // brute force particle finder
    int getParticleIndex( ofVec3f part )
    {
        for (int i=0; i<mesh.getNumVertices(); i++)
        {
            if ( mesh.getVertex(i) == part ){
                return i;
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
    
    // spacing is in pixels
    virtual void regenerateGrid( int spacing ) = 0;
    
protected:
    
    float offset;
    ofxSVG path;
    ofMesh mesh;
    vector<float> sizes;
    vector<bool>    occupied;
    vector<ofVec2f> offsets;
    vector< vector<ofVec2f> > letters;
    string SUFFIX;
};

