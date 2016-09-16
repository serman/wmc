//
//  ofApp_network.cpp
//  ipcameraWMC
//
//  Created by Sergio Galan on 08/07/16.
//
//

#include <stdio.h>
#include "ofApp.h"
using namespace ofxCv;
using namespace cv;
//https://gist.github.com/darrenmothersele/7597016

// The main openFrameworks include
#include "ofMain.h"

void ofApp::sendImage(){
   

}

void ofApp::oscRcvUpdate(){
    while(myosc.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;

        myosc.getNextMessage(&m);
        detections.clear();
        // check for mouse moved message
        if(m.getAddress() == "/face"){
            //cout << ofToString(m.getNumArgs());
            detectionData d;
            d.set(m.getArgAsInt32(0),//x
                  m.getArgAsInt32(1),//y
                  m.getArgAsInt32(2),//w
                  m.getArgAsInt32(3),//h
                  m.getArgAsInt32(4),//bear
                  m.getArgAsInt32(5),//age
                  m.getArgAsInt32(6),//glasses
                  m.getArgAsInt32(7),//gender
                  m.getArgAsInt32(8)//smile
                  );
            detections.push_back(d);
            ofLogVerbose()<< ofGetTimestampString("%d-%H %M:%S ")<< "data from Msoft: " << d.print() << endl;
            timeLastDetectionFromAPI=ofGetElapsedTimeMillis()/1000;
            
        }
        status=FINDING;
        takeShootingDecision();
    }
    
    
}


void ofApp::takeShootingDecision(){
    //detections
    for(std::size_t i = 0; i < detections.size(); i++){
       if(detections[i].age>25 && detections[i].age<50 && detections[i].beard>50 && detections[i].gender==0 )
           killThatOne();
            ofLogNotice()<< ofGetTimestampString("%d-%H %M:%S ")<< "BANG!!!" << detections[i].print() << endl;
    }
    
}
void ofApp::killThatOne(){
    // send arduino Orde
        status=SHOOTING;
    
        manalytics.addShot();
    
        if(finder.getTracker().existsCurrent(lastFaceTrackingIdSent)){
        //detectado sigue existiendo
            ofLog(OF_LOG_VERBOSE) << "detectado sigue existiendo" << lastFaceTrackingIdSent;
            targetMoved=finder.getTracker().getCurrent(lastFaceTrackingIdSent);
//            getIndexFromLabel(lastFaceTrackingIdSent);
            
            int p1=targetMoved.x +targetMoved.width/2;
            int resolucion = videoWidth;
            unsigned char angulo= ofMap(p1,0,resolucion,maxDeltaAngleServo,0);
            if(angulo>=0 && angulo <= maxDeltaAngleServo){ //verificación redundante pero por si acaso
                bangServoMsg[1]=angulo;
                if(serial.isInitialized()){
                    bool byteWasWritten = serial.writeBytes(&bangServoMsg[0],3);
                    if ( !byteWasWritten )
                        ofLogError("byte was not written to serial port");
                    timeLastMotorRotation=ofGetElapsedTimeMillis();
                }
                
            }
            else{
                if(serial.isInitialized()){
                    bool byteWasWritten = serial.writeBytes(&bangMsg[0],3);
                }
            }
        }
        else{
        //detectado ya no existe
            targetMoved.width=0;
            ofLog(OF_LOG_VERBOSE) << "detectado ya no existe. Disparo en la posicion actual";
            if(serial.isInitialized()){
                bool byteWasWritten = serial.writeBytes(&bangMsg[0],3);
                if ( !byteWasWritten )
                    ofLogError("byte was not written to serial port");
            }
        }
}