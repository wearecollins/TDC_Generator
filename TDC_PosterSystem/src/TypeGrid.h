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
        int index = 0;
        int x = 0, y = 0;
        int w = ofGetWidth(), h = ofGetHeight();
        
        int minx = 10000, maxx = -10000;
        int miny = 10000, maxy = -10000;
        
        for ( int i=0; i<path.getNumPath(); i++){
            ofPath pz = path.getPathAt(i);
            pz.setPolyWindingMode(OF_POLY_WINDING_ODD);
            ofRectangle r = pz.getOutline()[0].getBoundingBox();
            if ( r.x < minx ) minx = r.x;
            if ( r.x + r.width > maxx ) maxx = r.x + r.width;
            if ( r.y < miny ) miny = r.y;
            if ( r.y + r.height > maxy ) maxy = r.y + r.height;
        }
        
        x = minx - 10;
        w = maxx + 10;
        y = miny - 10;
        h = maxy + 10;
        
        for ( x; x<w; x += spacing){
            for ( y = miny - 10; y<h; y += spacing){
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
                    mesh.addIndex( index );
                    index++;
                }
            }
        }
    }
};
