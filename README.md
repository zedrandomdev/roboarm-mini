#Overview:
The goal of this project was to create a robotic arm with 3 degrees of freedom while learning about embedded processors that have limited capacity, such as the PIC16F690 8-bit processor.

###Checklist:
- [x] Respond to user control through computer and UART
- [x] Pick up and move small objects

###Main Challenge:
Controlling 4 servos with a single PMW module.
**Solution:** Use timer and pulse steering to sample and update each channel within 50hz intervals.


#Design:
The design of the robot arm was based on the open source [uARM model] (https://www.kickstarter.com/projects/ufactory/uarm-put-a-miniature-industrial-robot-arm-on-your) (originally a kickstarter project). However to save on costs, we simply downloaded the designs and used a laser cutter to cut all the parts required out of plexiglass.

<img src="https://github.com/ced92/roboarm-mini/raw/master/img/arm.png" width="500">

The design files are available [here] (http://www.thingiverse.com/thing:367431/#files)
and assembly instructions [here] (https://cdn.thingiverse.com/assets/dc/ae/c2/94/d0/Latest_greatest_uArm_Assembly_Instructions_v1.pdf)

#Materials:
- PIC16F690 (x1)
- Sanwa SRM-102 Servos (x3)
- Tower Pro SG90 Servo (x1)
- USB/UART tool (Pic Programmer)
