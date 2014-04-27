clear, clc, close all

plot_size = 1000;
buf_size = 12000;
channels = zeros(8, buf_size);
ndx = ones(1,8);

t = 0:1/100:2*pi;

counter = 0;

c_ndx = 0;

scrsz = get(0,'ScreenSize');
hfig = figure;
set(hfig,'position',scrsz);

t = tcpip('0.0.0.0',3000,'NetworkRole','server','Terminator','');
set(t,'InputBufferSize',120000);

disp('Waiting for connection');
fopen(t);

numChannels = 2;

while true
    c_ndx = fread(t,1,'uint32');         % channel number
    cnt = fread(t,1,'uint32');           % sample count
    samples = fread(t,cnt,'uint32');     % sample data
    
    channels(c_ndx, ndx(1,c_ndx):ndx(1,c_ndx)+cnt-1) = samples;
    ndx(1,c_ndx) = ndx(1,c_ndx) + cnt;
    if ndx(1,c_ndx) > (buf_size - 4000)
        ndx(1,c_ndx) = 1;
    end
    
    %if ndx(1,c_ndx) > plot_size
    counter = counter + 1;
    if mod(counter, numChannels) == 0
        
        counter = 0;
        for(i = 1:numChannels)
        subplot(numChannels,1,i);
        plot(1:plot_size, channels(i, 1:plot_size));
        end
        drawnow
    end
    %end
end