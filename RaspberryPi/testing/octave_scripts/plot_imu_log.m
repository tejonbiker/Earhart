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

%accel = dlmread("/home/pi/Earhart/RaspberryPi/testing/capture_vibrations/reset.txt");
accel=dlmread("/home/pi/Earhart/RaspberryPi/testing/tilt_sensing/archivo_salida.txt");

x=accel(:,1);
y=accel(:,2);
z=accel(:,3);
figure
plot(x);
figure
plot(y);
figure
plot(z);

%decimate section, install and enable signal package (pkg load signal)
%figure
%z_dec=decimate(z,10);
%plot(z_dec)


%print('reset.png','-dpng');
