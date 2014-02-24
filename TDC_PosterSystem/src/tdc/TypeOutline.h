//
//  TypeOutline.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/10/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxSvg.h"
#include "ofxXmlSettings.h"
#include "TypeMesh.h"

class TypeOutline : public TypeMesh {
    
public:
    TypeOutline(){
        SUFFIX = "_outline";
    }
    
    // spacing is in pixels
    void regenerateGrid( int spacing ){
        // sample each letter
        int index = 0;
        for ( int i=0; i<path.getNumPath(); i++){
            
            vector<ofPolyline> letter = getLetter(i);
            for ( int j=0; j<letter.size(); j++){
                ofPolyline l = letter[j].getResampledBySpacing(spacing);
                for ( int p=0; p<l.size(); p++ ){
                    ofVec2f point = l[p];
                    letters[i].push_back(point);
                    mesh.addVertex(point);
                    mesh.addColor( ofColor(255));
                    mesh.addIndex( index );
                    index++;
                }
            }
        }
    }
    
protected:
};
