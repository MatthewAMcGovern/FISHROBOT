clear 
clc
close all
load('FishData1,110.mat')
L = length(FishData1);
Fs = 1/.01869;

Y = fft(FishData1);

P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);
P1(1) = 0;


f = Fs*(0:(L/2))/L;
plot(f,P1) 
title('119 Hz FFT')
xlabel('f (Hz)')
ylabel('|P1(f)|')