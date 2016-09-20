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
        frameRateSum+=ofGetFrameRate();
        frameRateCount++;
        if(ofGetElapsedTimeMillis()>(timePeriodInit+period) ){
            faceDetectionsTotal+=faceDetectionsPeriod;
            faceShotTotal += faceShotPeriod;
            
            if(frameRateCount>0)
                frameRateMean=frameRateSum/frameRateCount;
            ofLogNotice() << getString() << endl;
            timePeriodInit=ofGetElapsedTimeMillis();
   //         myTextFile.open("text.txt",ofFile::WriteOnly);
            
            ofHttpResponse resp = ofLoadURL("http://emoncms.org/input/post?apikey=d67a577b4ecff2f80e982adf4d749d44&json={faceDetectionsPeriod:"+ ofToString(faceDetectionsPeriod) +",faceShotPeriod:" + ofToString(faceShotPeriod)+" }");
            ofLogVerbose() << resp.data << endl;
            faceShotPeriod,faceDetectionsPeriod,frameRateSum,frameRateCount,frameRateMean=0;
        }
            
    
    }
    
    std::string  getString(){
        std::string s= "faces in period: " + ofToString(faceDetectionsPeriod) +
                        " ___ Shot in period: " + ofToString(faceShotPeriod)+
                        " ___ Total Faces: " + ofToString(faceDetectionsTotal)+
                        " ___ Total Shot: " + ofToString(faceShotTotal)+
                                " ___ Framerate: " + ofToString(frameRateMean);
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
    int period=5*60*1000 - 8000;
    int faceDetectionsPeriod=0;
    int faceShotPeriod=0;
    int faceDetectionsTotal=0;
    int faceShotTotal=0;
    float frameRateSum=0;
    float frameRateCount=0;
    float frameRateMean=0;
    ofFile logTotalFile;
    
    
};

#endif
