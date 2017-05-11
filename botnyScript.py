import requests
import time
print 'Im running'
while 1:
        time.sleep(10)
        getCSV = requests.get('http://botny.ca/botny.csv')
        getCSV.raise_for_status()
        playfile = open('/home/pi/Desktop/botny.csv','wb')
        for chunk in getCSV.iter_content(100000):
                playfile.write(chunk)

	playfile.close()
	

