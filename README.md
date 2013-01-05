Arduino-RC-Timer-Switch
=========

Arduino controlling RC switches with JavaScript UI<br />

I used these: http://www.clasohlson.com/fi/Pr363570000

This model has the last two bits of the 24 bit conrol message as 0 or 1 to set the state of the switch. Some other models use only 1 bit and wont work without changes to the code.

- Controll cheap 433mHz AC switches with arduino and web UI<br />
- No computer needed as a mediator<br />
- Multipurpose JSON REST interface<br />
- Teach devices to arduino by pushing the remote controller buttons<br />
- Remote controller actions are tracked by the arduino<br />
- Timers<br />
- Settings saved to a SD card

Dummy test UI: http://www.cs.helsinki.fi/u/ljlukkar/rcswitch
