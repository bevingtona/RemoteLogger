#### Todo list (work items) for Rachel's work:

#### By end of March:
- [x] start writing on library section of paper
- [ ] remove the concept of predefined set pins - let the user set in the written software
    - note some sensors have more than one pin (e.g. ultrasonic)
- [ ] pull set/unset pins out from library

Code examples to include (clean using library):
- hydros
- hydros + analite
- ott
- ott + analite
- hydros + ultrasonic
- ultrasonic


#### Lower-priority:

- [x] basic library for functions (RemoteLogger.h/.cpp)
- [x] documentation for importing the library 
- [ ] generalize take measurement 
    - [ ] document header meanings (in measurement messages)
- [ ] generalize prep message 
- [ ] add tests for basics (sensors, iridium, etc) to examples for library
    - [ ] clean up/document the hydros test example (just take measurements, print to console)
    - [ ] blinky example
    - [ ] send "hello world" on iridium example
    - [ ] measurement from analite (uses voltage not sdi bus) example
- [ ] make data pin (for bus) flexible -- parameter for the constructor?
- [ ] quiet/verbose modes -- in param file?
