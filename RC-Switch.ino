#include "SPI.h"
#include "SD.h"
#include "Wire.h"
#include "Ethernet.h"
#include "RTClib.h"
#include "WebServer.h"
#include "aJSON.h"
#include "RCSwitch.h"

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
WebServer webserver("", 80);
aJsonObject* devices; // an array of devices: [{"name":"Fan","state":0,"did":1131861,"id":1},...]
aJsonObject* timers; // an array of timers: [{"action":0,"did":1,"name":"Fan Timer","state":0,"time":64320000,"id":1},...]
File file;
RCSwitch mySwitch = RCSwitch();
int curindex = 0;
long tempdids[5];
RTC_DS1307 RTC;
int curday;
long tempdid = 0;
char * names[6];
int scheduler = 0;
long lastReceivedID = 0;

void indexCmd(WebServer &server, WebServer::ConnectionType type, char *, bool) {
	server.httpSuccess();
	if (type != WebServer::HEAD) {
		P(helloMsg) =
				"<!DOCTYPE html>\n"
				"<html lang=\"en\">\n"
				"<head>\n"
				"\t<meta charset=\"UTF-8\"/>\n"
				"\t<title>Arduino RC Timer Switch</title>\n"
				"\t<link rel=\"stylesheet\" href=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/css/screen.css\"/>\n"
				"\t<link rel=\"stylesheet\" href=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/css/bootstrap.css\"/>\n"
				"\t<link rel=\"stylesheet\" href=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/css/bootstrap-responsive.css\"/>\n"
				"\t<link rel=\"stylesheet\" href=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/css/datepicker.css\"/>\n"
				"\t<link rel=\"stylesheet\" href=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/css/timepicker.css\"/>\n"
				"</head>\n"
				"<body>\n"
				"<div class=\"navbar navbar-inverse navbar-fixed-top\">\n"
				"\t<div class=\"navbar-inner\">\n"
				"\t\t<div class=\"container\">\n"
				"\t\t\t<button type=\"button\" class=\"btn btn-navbar\" data-toggle=\"collapse\" data-target=\".nav-collapse\">\n"
				"\t\t\t\t<span class=\"icon-bar\"></span>\n"
				"\t\t\t\t<span class=\"icon-bar\"></span>\n"
				"\t\t\t\t<span class=\"icon-bar\"></span>\n"
				"\t\t\t</button>\n"
				"\t\t\t<a class=\"brand\" href=\"https://github.com/lasselukkari/Arduino-RC-Timer-Switch\">Arduino RC Timer Switch</a>\n"
				"\t\t\t<div class=\"nav-collapse collapse\">\n"
				"\t\t\t\t<ul class=\"nav\">\n"
				"\t\t\t\t\t<li class=\"\">\n"
				"\t\t\t\t\t\t<a href=\"#devices\">Devices</a>\n"
				"\t\t\t\t\t</li>\n"
				"\t\t\t\t\t<li>\n"
				"\t\t\t\t\t\t <a href=\"#timers\">Timers</a>\n"
				"\t\t\t\t\t</li>\n"
				"\t\t\t\t</ul>\n"
				"\t\t\t</div>\n"
				"\t\t</div>\n"
				"\t</div>\n"
				"</div>\n"
				"<div class=\"container\" id=\"main-container\">\n"
				"</div>\n"
				"<script src=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/js/jquery.min.js\"></script>\n"
				"<script src=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/js/underscore-min.js\"></script>\n"
				"<script src=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/js/backbone-min.js\"></script>\n"
				"<script src=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/js/bootstrap.min.js\"></script>\n"
				"<script src=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/js/Backbone.ModelBinder.js\"></script>\n"
				"<script src=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/js/bootstrap-datepicker.js\"></script>\n"
				"<script src=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/js/bootstrap-timepicker.js\"></script>\n"
				"<script src=\"http://cs.helsinki.fi/u/ljlukkar/rcswitch/js/app.js\"></script>\n"
				"</body>\n"
				"</html>";
		server.printP(helloMsg);
	}
}

// this method is performed before the reguest is prosessed and default responce is generated
void devicesBeforeResposeCmd(WebServer &server, WebServer::ConnectionType type, char * tail, bool tailComplete,
		aJsonObject ** collection, aJsonObject ** model) {

	if (type == WebServer::POST || type == WebServer::PUT) {

		aJsonObject* did = aJson.getObjectItem(&**model, "did");
		aJsonObject* state = aJson.getObjectItem(&**model, "state");
		long command = 0;

		if ((int) did->type == aJson_Int) {
			command = did->valueint;
		} else if ((int) did->type == aJson_Long) {
			command = did->valuelong;
		}

		int cmd = state->valueint;

		if (cmd == 1) {
			command = command << 2;
			command = command + 3;
		} else if (cmd == 0) {
			command = command << 2;
		}

		mySwitch.send(command, 24);

	}

	if (type == WebServer::DELETE) {

		aJsonObject* idObject = aJson.getObjectItem(&**model, "id");

		if ((int) idObject->type == aJson_Int) {
			tempdid = idObject->valueint;
		} else if ((int) idObject->type == aJson_Long) {
			tempdid = idObject->valuelong;
		}

	}

}

// this method is performed after the responce is sent so it doens't block it anymore
void timersAfterResposeCmd(WebServer::ConnectionType type, aJsonObject ** collection, aJsonObject ** model) {
	pinMode(53, OUTPUT);
	if (type != WebServer::GET) {
		if (SD.exists("timers.txt")) {
			SD.remove("timers.txt");
		}

		file = SD.open("timers.txt", FILE_WRITE);
		aJson.print(&**collection, file);
		file.close();
	}
}

// this method is performed after the responce is sent so it doens't block it anymore
void devicesAfterResposeCmd(WebServer::ConnectionType type, aJsonObject ** collection, aJsonObject ** model) {
	pinMode(53, OUTPUT);

	if (type == WebServer::DELETE) {
		aJsonObject *child = (timers)->child;
		byte i = 0;

		while (child) {
			long tdid = 0;
			aJsonObject* didObject = aJson.getObjectItem(child, "did");

			if ((int) didObject->type == aJson_Int) {
				tdid = didObject->valueint;
			} else if ((int) didObject->type == aJson_Long) {
				tdid = didObject->valuelong;
			}

			if (tempdid == tdid) {
				aJson.deleteItemFromArray(timers, i);

				if (SD.exists("timers.txt")) {
					SD.remove("timers.txt");
				}

				file = SD.open("timers.txt", FILE_WRITE);
				aJson.print(&**collection, file);
				file.close();
			}

			child = child->next;
			i++;
		}
	}

	if (type != WebServer::GET) {

		if (SD.exists("devices.txt")) {
			SD.remove("devices.txt");
		}

		file = SD.open("devices.txt", FILE_WRITE);
		aJson.print(&**collection, file);
		file.close();
	}
}

void setup() {
	Serial.begin(9600);

	mySwitch.enableReceive(4); // interrupt 4 is the pin number 19
	mySwitch.enableTransmit(18);

	Wire.begin();
	RTC.begin();

	if (!RTC.isrunning()) {
		Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		RTC.adjust(DateTime(__DATE__, __TIME__));
		RTC.adjust(DateTime(__DATE__, __TIME__));
	}

	// if we know the key name we can define them to save memory. instead of new char array we use only a pointer
	names[0] = "id";
	names[1] = "name";
	names[2] = "state";
	names[3] = "did";
	names[4] = "action";
	names[5] = "time";
	aJson.setNames(names, 6);

	pinMode(53, OUTPUT);

	if (!SD.begin(4)) {
		Serial.println("SD initialization failed!");
	}

	//load data from file if found

	if (SD.exists("devices.txt")) {
		file = SD.open("devices.txt");
		devices = aJson.parse(file);
		file.close();

	} else {
		devices = aJson.createArray();
	}

	if (SD.exists("timers.txt")) {
		file = SD.open("timers.txt");
		timers = aJson.parse(file);
		file.close();

	} else {
		timers = aJson.createArray();
	}

	//add binding to webserver and set actions
	webserver.addJSONBinding("devices", &devices);
	webserver.setBeforeRequest("devices", &devicesBeforeResposeCmd);
	webserver.setAfterRequest("devices", &devicesAfterResposeCmd);

	webserver.addJSONBinding("timers", &timers);
	webserver.setAfterRequest("timers", &timersAfterResposeCmd);

	webserver.setDefaultCommand(&indexCmd);

	Ethernet.begin(mac);
	webserver.begin();

	curday = RTC.now().day();

	for (byte i = 0; i < 4; i++) {
		Serial.print(Ethernet.localIP()[i]);
		if (i < 3) {
			Serial.print('.');
		}
	}
	Serial.println();
}

void checkTimers() {
	DateTime now = RTC.now();

	aJsonObject* timerChild = (timers)->child;
	long epoch = now.unixtime();
	long datetime = (now.hour() * 60 + now.minute()) * 60000; //seconds since midnight

	while (timerChild) {
		aJsonObject* stateObject = aJson.getObjectItem(timerChild, "state");

		if (stateObject->valueint == 0) {
			aJsonObject* timeObject = aJson.getObjectItem(timerChild, "time");
			aJsonObject* didObject = aJson.getObjectItem(timerChild, "did");
			long time = 0;
			long timerdid = 0;

			if ((int) didObject->type == aJson_Int) {
				timerdid = didObject->valueint;
			} else if ((int) didObject->type == aJson_Long) {
				timerdid = didObject->valuelong;
			}

			if (timerdid == 0) {
				break;
			}

			if ((int) timeObject->type == aJson_Int) {
				time = timeObject->valueint;
			} else if ((int) timeObject->type == aJson_Long) {
				time = timeObject->valuelong;
			}

			boolean timeToAct = false;

			if (time != 0 && time < 86400000) {
				if (time < datetime) {
					timeToAct = true;
				}
			} else if (time != 0 && time > 86400000) {
				if (time < epoch) {
					timeToAct = true;
				}
			}

			if (timeToAct) {
				int action = aJson.getObjectItem(timerChild, "action")->valueint;

				aJsonObject *deviceChild = (devices)->child;
				while (deviceChild) {
					didObject = aJson.getObjectItem(deviceChild, "did");
					aJsonObject* idObject = aJson.getObjectItem(deviceChild, "id");

					long id = 0;
					long did = 0;

					if ((int) idObject->type == aJson_Int) {
						id = idObject->valueint;
					} else if ((int) idObject->type == aJson_Long) {
						id = idObject->valuelong;
					}

					if ((int) didObject->type == aJson_Int) {
						did = didObject->valueint;
					} else if ((int) didObject->type == aJson_Long) {
						did = didObject->valuelong;
					}

					if (id == timerdid) {
						aJson.getObjectItem(deviceChild, "state")->valueint = action;
						if (action == 0) {
							did = did << 2;
							did = did + 3;
						} else if (action == 1) {
							did = did << 2;
						}

						mySwitch.send(did, 24);
						aJson.getObjectItem(timerChild, "state")->valueint = 1;
						if (SD.exists("timers.txt")) {
							SD.remove("timers.txt");
						}

						file = SD.open("timers.txt", FILE_WRITE);
						aJson.print(timers, file);
						file.close();
						break;

					}
					deviceChild = deviceChild->next;
				}

			}

		}
		timerChild = timerChild->next;
	}
}

void checkDayChange() {
	DateTime now = RTC.now();

	if (curday != now.day()) {
		curday = now.day();

		aJsonObject *child = (timers)->child;
		while (child) {

			long time = 0;
			aJsonObject* timeObject = aJson.getObjectItem(child, "time");

			if ((int) timeObject->type == aJson_Int) {
				time = timeObject->valueint;
			} else if ((int) timeObject->type == aJson_Long) {
				time = timeObject->valuelong;
			}

			if (time < 86400000) {
				aJson.getObjectItem(child, "state")->valueint = 0;
			}
			child = child->next;
		}
	}
}

void checkRadio() {
	if (mySwitch.available()) {

		long receivedID = mySwitch.getReceivedValue();

		if (receivedID) {

			if (receivedID == lastReceivedID) {

				long shiftedID = receivedID >> 2;
				byte state = receivedID % 2;

				boolean found = false;

				for (int i = 0; i < 5; i++) {

					if (tempdids[i] == shiftedID) {

						long maxID = 0;
						aJsonObject *child = (devices)->child;
						while (child) {

							aJsonObject* didObject = aJson.getObjectItem(child, "did");
							aJsonObject* idObject = aJson.getObjectItem(child, "id");

							long id = 0;
							long did = 0;

							if ((int) idObject->type == aJson_Int) {
								id = idObject->valueint;
							} else if ((int) idObject->type == aJson_Long) {
								id = idObject->valuelong;
							}

							if ((int) didObject->type == aJson_Int) {
								did = didObject->valueint;
							} else if ((int) didObject->type == aJson_Long) {
								did = didObject->valuelong;
							}

							if (id > maxID) {
								maxID = id;
							}

							if (did == shiftedID) {
								aJson.getObjectItem(child, "state")->valueint = state;
								found = true;
								break;
							}

							child = child->next;
						}
						if (!found) {
							aJsonObject* device = aJson.createObject();
							aJson.addItemToObject(device, "name", aJson.createItem("New Device"));
							aJson.addNumberToObject(device, "state", (int) state);
							aJson.addNumberToObject(device, "did", shiftedID);
							aJson.addNumberToObject(device, "id", maxID + 1);
							aJson.addItemToArray(devices, device);

							if (SD.exists("devices.txt")) {
								SD.remove("devices.txt");
							}

							file = SD.open("devices.txt", FILE_WRITE);
							aJson.print(devices, file);
							file.close();
						}
					}

					tempdids[curindex++] = shiftedID;
					if (curindex == 5) {
						curindex = 0;
					}
				}
			}
			lastReceivedID = receivedID;
		}
	}
	mySwitch.resetAvailable();
}

void loop() {

	webserver.processConnection();

	checkRadio();

	if (!scheduler++) {
		checkDayChange();
		checkTimers();
	}
}


