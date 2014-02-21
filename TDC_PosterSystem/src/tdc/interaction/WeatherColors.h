//
//  WeatherColors.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/20/14.
//
//


#pragma once

#include "ofMain.h"

class WeatherColors {
public:
    
    WeatherColors();
    
    ofColor& operator[]( int n ){ return colors[n];}
    
protected:
    map<int,ofColor> colors;
};

static const WeatherColors weather;