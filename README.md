# TIVA-Motor-Control
Tiva Motor Controller is a program that controls a motor on a T4MC microprocessor. Users can desired speed, direction, and even position.

# How does it work?
This program enables GPIOs and utilizes Tiva's driver library to generate a pulse width modulation signal. Information about the motor such as velocity and position is gathered by interfacing with the motor's quadratic encoder. By reading from the QEI the program obtains data about the motor's current state to fulfill user requests.

# Where can I get it?

### Hardware
LaunchPad:Tiva C Series TM4C1294 Connected LaunchPad http://www.ti.com/tool/sw-ek-tm4c1294xl 

### Demo
https://www.youtube.com/watch?v=oRoG_mAtY50
