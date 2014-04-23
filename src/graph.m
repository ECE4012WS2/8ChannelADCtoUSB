
% system('make');
% system('./project');
% 
t = tcpip('0.0.0.0',3000,'NetworkRole','server','Terminator','');
set(t,'InputBufferSize',120000);
%set(t,'ByteOrder','littleEndian')
t
disp('Waiting for connection')
%fopen(t);

disp('Going into read loop')

data = zeros(8,5000);
while true
    for i = 1:8
        %channelNum = fread(t,1,'uint32');
        %size = fread(t,1,'uint32');
        size=1000;
        arraySize = size;
        %data(i) = fread(t,arraySize,'uint32');
    end
    
    scrsz = get(0,'ScreenSize');
    hfig = figure;
    set(hfig,'position',scrsz);
    
    subplot(8,1,1)
    plot(1:arraySize-1, data(1,1:arraySize-1));
    title('Channel 1');
    subplot(8,1,2)
    plot(1:arraySize-1, data(1,1:arraySize-1));
    title('Channel 2');
    subplot(8,1,3)
    plot(1:arraySize-1, data(1,1:arraySize-1));
    title('Channel 3');
    subplot(8,1,4)
    plot(1:arraySize-1, data(1,1:arraySize-1));
    title('Channel 4');
    subplot(8,1,5)
    plot(1:arraySize-1, data(1,1:arraySize-1));
    title('Channel 5');
    subplot(8,1,6)
    plot(1:arraySize-1, data(1,1:arraySize-1));
    title('Channel 6');
    subplot(8,1,7)
    plot(1:arraySize-1, data(1,1:arraySize-1));
    title('Channel 7');
    subplot(8,1,8)
    plot(1:arraySize-1, data(1,1:arraySize-1));
    title('Channel 8');
    xlabel('Sample Number');
end

fclose(t);
return;

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
