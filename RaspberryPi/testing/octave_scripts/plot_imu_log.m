%
%	Copyright Federico Ramos
%	Air Image HD
%
%	This script is used to visualizate and process some data of the accelerometers.
%	It's based on Octave, see how to install in Raspberry in:
%
%	http://wiki.octave.org/Rasperry_Pi
%
%	Enjoy it!
close all;
clc;

accel = dlmread("/home/pi/Earhart/RaspberryPi/testing/capture_vibrations/reset.txt");
z=accel(:,3);
plot(z);

%decimate section, install and enable signal package (pkg load signal)
figure
z_dec=decimate(z,10);
plot(z_dec)


%print('reset.png','-dpng');