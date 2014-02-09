Please note: this project is outdated with the dependencies.

Arduino RC Timer Switch
=========
Arduino controlling RC switches with a JavaScript UI<br />

- Controll cheap 433MHz AC switches with arduino and web UI<br />
- No computer needed as a mediator<br />
- Multipurpose JSON REST interface<br />
- Teach devices to arduino by pushing the remote controller buttons<br />
- Remote controller actions are tracked by the arduino<br />
- Timers<br />
- Settings saved to a SD card

Dummy test UI: http://www.cs.helsinki.fi/u/ljlukkar/rcswitch

<img src="http://www.cs.helsinki.fi/u/ljlukkar/rcswitch/board.jpg" />

I used these ones: <br />
http://www.clasohlson.com/fi/Pr363570000<br />
<img src="http://www.clasohlson.com/medias/sys_master/8871984332830.jpg" /> <img src="http://www.clasohlson.com/medias/sys_master/8871984234526.jpg" /><br />
This model has the last two bits of the 24 bit control message set as 0 or 1 to set the state of the switch. Some other models use only the last bit and wont work without changes to the code.

Use with the modified Webduino and aJson libraries:<br />
https://github.com/lasselukkari/Webduino<br />
https://github.com/lasselukkari/aJson

Other library dependencies:<br />
http://code.google.com/p/rc-switch/<br />
https://github.com/adafruit/RTClib

Part list
========
- Arduino Mega 2560 or a cheap clone<br />
- Wiznet W5100 ethernet / SD card reader chield (cheap ones seem to have problems with resetting)<br />
- 433MHz transmitter and receiver. 2$ shipped from ebay 
- DS1307 real time clock. 2$ shipped from ebay 
