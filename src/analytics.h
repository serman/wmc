//
//  analytics.h
//  wmc
//
//  Created by Sergio Galan on 14/09/16.
//
//
#include "ofMain.h"

#ifndef wmc_analytics_h
#define wmc_analytics_h
class analytics{
public:
    analytics(){
                timePeriodInit=ofGetElapsedTimeMillis();
    }

    
    void addDetection(){
        faceDetectionsPeriod++;
    }
    void addShot(){
        faceShotPeriod++;
    }
    void update(){
        if(ofGetElapsedTimeMillis()>(timePeriodInit+period)){
            faceDetectionsTotal+=faceDetectionsPeriod;
            faceShotTotal += faceShotPeriod;
            getString();
            timePeriodInit=ofGetElapsedTimeMillis();
            
        }
            
    
    }
    
    std::string  getString(){
        std::string s= "faces in period: " + ofToString(faceDetectionsPeriod) +
                        " ___ Shot in period: " + ofToString(faceShotPeriod)+
                        " ___ Total Faces: " + ofToString(faceDetectionsTotal)+
                        " ___ Total Shot: " + ofToString(faceShotTotal);
        return s;
    };
    
    void reset(){
        faceDetectionsPeriod=0;
        faceShotPeriod=0;
        faceDetectionsTotal=0;
        faceShotTotal=0;
    }
    
private:
    long timePeriodInit=0;
    int period=60*5*1000;
    int faceDetectionsPeriod=0;
    int faceShotPeriod=0;
    int faceDetectionsTotal=0;
    int faceShotTotal=0;
    
    
    
};

#endif
