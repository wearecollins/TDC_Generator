//
//  Data.h
//  TDC_PosterSystem
//
//  This is just a storage object with some small calculations

#pragma once

#include "ofMain.h"

class Data
{
public:
    
    Data(){
        environmentImmediate = 0.0;
        environmentLocal = 0.0;
        environmentGlobal = 0.0;
        language = 0.0;
        time = 0.0;
        eiWeight = elWeight = egWeight = langWeight = .25;
    }
    
    void setup(){
        ofAddListener( ofEvents().update, this, &Data::update);
    }
    
    void update( ofEventArgs & e ){
        // update time + date
        static float h, m, s;
        h = (float) ofGetHours() / 24.0;
        m = (float) ofGetMinutes() / (60. * 24.0);
        s = (float) ofGetSeconds() / (60. * 60. * 24.);
        time = h + m + s;
        
        static float d,mo;
        d = (float) ofGetDay();
        mo = (float) ofGetMonth();
        
        int days = 0;
        for ( int i=1; i<mo; i++){
            i += calcDays( i, ofGetYear());
        }
        date = (float) (days + d) / 365.0f;
        
        // update weights
        // ?
        
        eiWeight = ofClamp(eiWeight, 0.0, 1.0);
        elWeight = ofClamp(elWeight, 0.0, 1.0);
        egWeight = ofClamp(egWeight, 0.0, 1.0);
        langWeight = ofClamp(langWeight, 0.0, 1.0);
        timeWeight = ofClamp(timeWeight, 0.0, 1.0);
        dateWeight = ofClamp(dateWeight, 0.0, 1.0);
    }
    
    float getWeightedEI(){
        return environmentImmediate * eiWeight;
    }
    
    float getWeightedEL(){
        return environmentLocal * elWeight;
    }
    
    float getWeightedEG(){
        return environmentGlobal * egWeight;
    }
    
    float getWeightedLang(){
        return language * langWeight;
    }
    
    float getWeightedTime(){
        return time * timeWeight;
    }
    
    float getWeightedDate(){
        return date * dateWeight;
    }
    
    float getUserWeight(){
        return 1.0; //hm
    }
    
    inline int is_leap(int year)
    {
        return year > 0 && !(year % 4) && (year % 100 || !(year % 400));
    };
    
    int calcDays(int month, int year)
    {
        static const int monthdays[2][13] = {
            { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
            { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
        };
        
        if (month < 1 || month > 12) {
            throw std::invalid_argument("Invalid month");
        }
        
        return monthdays[is_leap(year)][month - 1];
    }
    
    // everything is public for now
    
    float environmentImmediate;
    float environmentLocal;
    float environmentGlobal;
    
    float language;
    float time;
    float date;
    
    // these will be tied to specific objects
    float eiWeight;
    float elWeight;
    float egWeight;
    float langWeight;
    float timeWeight;
    float dateWeight;
};
