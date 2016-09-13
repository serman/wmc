import urllib2
import json

response = urllib2.urlopen('http://server.eclectico.net/reboot.php')
data = json.load(response)  
if data['mustreboot'] == 0:
    print "not reboot"
else:
    print "reboot"
print data