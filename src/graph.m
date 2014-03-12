
system('make');
system('./project');


c1 = dlmread('channel1.csv', ',');
c2 = dlmread('channel2.csv', ',');
c3 = dlmread('channel3.csv', ',');
c4 = dlmread('channel4.csv', ',');
c5 = dlmread('channel5.csv', ',');
c6 = dlmread('channel6.csv', ',');
c7 = dlmread('channel7.csv', ',');
c8 = dlmread('channel8.csv', ',');

scrsz = get(0,'ScreenSize');

hfig = figure;
set(hfig,'position',scrsz);


subplot(8,1,1)
plot(c1(:,1), c1(:,2));
title('Channel 1');
subplot(8,1,2)
plot(c2(:,1), c2(:,2));
title('Channel 2');
subplot(8,1,3)
plot(c3(:,1), c3(:,2));
title('Channel 3');
subplot(8,1,4)
plot(c4(:,1), c4(:,2));
title('Channel 4');
subplot(8,1,5)
plot(c5(:,1), c5(:,2));
title('Channel 5');
subplot(8,1,6)
plot(c6(:,1), c6(:,2));
title('Channel 6');
subplot(8,1,7)
plot(c7(:,1), c7(:,2));
title('Channel 7');
subplot(8,1,8)
plot(c8(:,1), c8(:,2));
title('Channel 8');
xlabel('Sample Number');
