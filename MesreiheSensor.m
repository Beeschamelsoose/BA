clear all;
close all;
Zeit1=[0.5,0.75,1,1.25,1.5,1.75,2,2.25,3.00,3.5,4,4.5,5,5.5,6,6.5,7,7.5,8,8.5,9,9.5,10,15,20,25,25.5,26,30,35,40,45];

Werte1=[9.54,11.6,12.96,13.98,14.46,14.96,15.24,15.39,19.86,13.13,6.42,4.15,3.08,2.44,2.06,1.83,1.70,1.63,1.62,1.64,1.66,1.70,1.76,2.43,2.96,9.5,3.5,3.36,3.46,3.6,3.71,3.74];
Zeit2=[0,0.25,0.5,0.75,1,1.25,1.5,1.75,2,2.5,3,3.5,4,4.5,5,5.5,6,6.5,7,7.5,8,8.5,9,10,11];
Werte2=[3.99,3.92,4.34,4.63,4.81,4.99,5.15,5.34,5.47,5.79,6.15,6.38,6.68,6.97,7.24,7.30,5.7,4.42,4.25,4.38,4.12,4.1,4.1,4.3,4.3];
X=[10,20,30,40];
Y=[1.76,2.96,3.46,3.71];
X2=[0.5,5];
Y2=[4.34,7.24];

figure;
plot(Zeit1,Werte1,'-x');
grid on;
xline(2.5,'--');
xline(22.5,'--');
hold on;
line(X,Y);
xlabel('Zeit in min');
ylabel('Iout in mA');
title('Zeitverlauf Sensor 1');
subtitle('bei 2,5 min Einstellen des Gains auf maximalen Ausschlag | bei 22,5 min kurzes Ausschalten der Elektronik');
figure;
plot(Zeit1,4-Werte1);
grid on;


figure;
plot(Zeit2,Werte2,'-x');
grid on;
hold on;
xline(5.75,'--');
line(X2,Y2);
xlabel('Zeit in min');
ylabel('Iout in mA');
title('Zeitverlauf Sensor 2');
subtitle('Spülen mit Luft ab 5:45')