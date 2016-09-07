
from __future__ import print_function
import sys
import time
import requests
import operator
import numpy as np
import os
from time import sleep
import urllib
#import serial
#osc
import OSC
import threading

aperturaCamara = 15 # La camara con el zoom a tope va de -15 a 15
resolucion = 1280.0

s=None   #Servidor osc
#arduinoSerial   = serial.Serial('/dev/cu.usbmodem621', 9600)

mresult = None
lastResult = None

facesIds = []
run = True

#video_capture = cv2.VideoCapture(0)
# Import library to display results
#import matplotlib.pyplot as plt
#%matplotlib inline
_url = 'https://api.projectoxford.ai/face/v1.0/detect'
#_key = '726500e04dc94de9a75e33ddd9310cfc'
_key = 'f9cf739b3848457b8ca4dfa6912f09ee'
_maxNumRetries = 2
pause=False;

client = OSC.OSCClient()
client.connect( ('127.0.0.1', 12000) )

def rcvOscOF():
    '''
    inicia servidor OSC
    '''
    s = OSC.OSCServer(('127.0.0.1', 12345))
    s.addDefaultHandlers()
    s.addMsgHandler("/video",rcvMessage)

    st = threading.Thread(target=s.serve_forever)
    st.setDaemon( True)
    st.start()

def sendDataOSC(result):  
    msg = OSC.OSCMessage()

    for currFace in result:
        faceRectangle = currFace['faceRectangle']
        print(str(currFace['faceAttributes']))
        msg.setAddress("/face")
        #int x, int y, int w, int h, int _beard, int _age, int _glasses, int _gender, int _smile
        msg.append([faceRectangle['left'], faceRectangle['top'], faceRectangle['width'],  faceRectangle['height'] ]) 
        msg.append(currFace['faceAttributes']['facialHair']['beard'])
        msg.append(currFace['faceAttributes']['age'])
        
        if( currFace['faceAttributes']['age'] == 'NoGlasses'):
            msg.append(0)
        else: msg.append(1)

        if( currFace['faceAttributes']['gender'] == 'male'):
            msg.append(0)
        else: msg.append(1)
        msg.append(currFace['faceAttributes']['smile'])

        #msg.append(str(currFace['faceAttributes']))
        msg.append(currFace['faceId'])
    client.send(msg)
    print("sent OSC Data " + str(msg))

        



def rcvMessage(addr, tags, stuff, source):
      msg_string = "%s [%s] %s" % (addr, tags, str(stuff))
      sys.stdout.write("OSCServer Got: '%s' from %s\n" % (msg_string, source))
      #comprobar que el video que llega es el suyo
      print ("received ed of video")
      #print (str(stuff))
      processImage();
      #actualizar videos
      #self.myModel.defineNextVideo();
      #self.myModel.setNextVideo(str(stuff))
      #2 Enviar nuevos videos a Processing
      #self.sendVideos()
      #cherrypy.engine.publish('websocket-broadcast', TextMessage( self.myModel.currentContentAsJson() ) )    

def sendDataLocal(pathFile):
    pathToFileInDisk = pathFile
    with open(pathToFileInDisk, 'rb') as f:
        data = f.read()

        # Draw a rectangle around the faces
        # for (x, y, w, h) in faces:
        #    cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)

        # Face detection parameters

    params = {'returnFaceAttributes': 'age,gender,smile,facialHair,glasses',
              'returnFaceLandmarks': 'false'}

    headers = dict()
    headers['Ocp-Apim-Subscription-Key'] = _key
    headers['Content-Type'] = 'application/octet-stream'

    json = None

    result = processRequest(json, data, headers, params)

    # Display the resulting frame
    # cv2.imshow('Video', frame)
    

    return result;

def sendStringImage(data):

    params = {'returnFaceAttributes': 'age,gender,smile,facialHair,glasses',
              'returnFaceLandmarks': 'false'}

    headers = dict()
    headers['Ocp-Apim-Subscription-Key'] = _key
    headers['Content-Type'] = 'application/octet-stream'

    json = None

    result = processRequest(json, data, headers, params)

    return result;

def processRequest(json, data, headers, params):
    """
    Helper function to process the request to Project Oxford
    Parameters:
    json: Used when processing images from its URL. See API Documentation
    data: Used when processing image read from disk. See API Documentation
    headers: Used to pass the key information and the data type request
    """

    retries = 0
    result = None

    while True:
        response = requests.request('post', _url, json=json, data=data, headers=headers, params=params)

        if response.status_code == 429:

            print("Message: %s" % (response.json()['error']['message']))

            if retries <= _maxNumRetries:
                time.sleep(1)
                retries += 1
                continue
            else:
                print('Error: failed after retrying!')
                break

        elif response.status_code == 200 or response.status_code == 201:

            if 'content-length' in response.headers and int(response.headers['content-length']) == 0:
                result = None
            elif 'content-type' in response.headers and isinstance(response.headers['content-type'], str):
                if 'application/json' in response.headers['content-type'].lower():
                    result = response.json() if response.content else None
                elif 'image' in response.headers['content-type'].lower():
                    result = response.content
        else:
            print("Error code: %d" % (response.status_code))
            print("Message: %s" % (response.json()['error']['message']))

        break

    return result



def newest_file_in_tree(rootfolder, extension=".jpg"):
    return max(
        (os.path.join(dirname, filename)
         for dirname, dirnames, filenames in os.walk(rootfolder)
         for filename in filenames
         if filename.endswith(extension)),
        key=lambda fn: os.stat(fn).st_mtime)



def processImage():
   # [image, imageString] = url_to_image('http://admin:agua@192.168.1.108/cgi-bin/snapshot.cgi?1')

    pathToFileInDisk = '/Users/sergiogalan/temporalborrar/snapshot2.jpg'
    with open( pathToFileInDisk, 'rb' ) as f:
        diskImage = f.read()
    # print ("face detectd")
    mresult = sendStringImage(diskImage)
#        mresult = sendDataLocal('Lenna.png')
    if mresult is not None:        
        sendDataOSC(mresult)


                

def quit_callback(path, tags, args, source):
    # don't do this at home (or it'll quit blender)
    global run
    run = False            
            

rcvOscOF();


while run:
    # do the game stuff:
    sleep(1)





