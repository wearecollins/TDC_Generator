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
#include "ofxXmlSettings.h"
#include "TypeMesh.h"

class TypeGrid : public TypeMesh {

public:
    
    TypeGrid(){
        SUFFIX = "_mesh";
    }
    
    // spacing is in pixels
    void regenerateGrid( int spacing ){
        for ( int x=0; x<ofGetWidth(); x += spacing){
            for ( int y=0; y<ofGetHeight(); y += spacing){
                ofVec2f p = ofVec2f(x,y);
                
                bool bGood = false;
                for ( int i=0; i<path.getNumPath(); i++){
                    bGood = pointIsInsideLetter(i, p);
                    if (bGood){
                        letters[i].push_back(p);
                        break;
                    }
                }
                
                if ( bGood ){
                    mesh.addVertex(p);
                    mesh.addColor( ofColor(255));
                }
            }
        }
    }
};
