# Measure-RMS-with-ATmega16
Project for Master Thesis in University of Science and Technology AGH

Hi!
At first, I want to apologize for my poor English, I'm not native speaker.
The goal of this thesis is to construct a electric field meter with dipole antenna. The thesis includes written material and physical execution of a PCB containing a microcontroller. The designed meter is designed to determine in real time the RMS value of the processed signal. The measuring and calculating system is built on the basis of an ATmega series microcontroller, on which a program code written by the diplomat is uploaded, setting the microcontroller and performing operations on the data necessary to determine the correct result. The work is based on the recommendations formulated by the electrical standard EN-IEC 61786-1:2014. The result of the work is a working device whose metrological properties are compared with a device with legislation.

Main file of program is in file 'main.c'. It's also necessary to link files 'timery.h' i 'pomiar_nap.h'.
It is all based on 'Mikrokontrolery AVR Język C' written by Mirosław Kardaś (PL) and screen library is downloaded from www.atnel.pl. I do not own any rights to these files.
