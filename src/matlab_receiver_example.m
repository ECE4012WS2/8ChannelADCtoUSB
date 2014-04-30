% 
% // The MIT License (MIT)
% //
% // Copyright (c) 2014 Austin Ward, Yao Lu, Fujun Xie, Eric Patterson, Mohan Yang
% //
% // Permission is hereby granted, free of charge, to any person obtaining a copy of
% // this software and associated documentation files (the "Software"), to deal in
% // the Software without restriction, including without limitation the rights to
% // use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
% // the Software, and to permit persons to whom the Software is furnished to do so,
% // subject to the following conditions:
% //
% // The above copyright notice and this permission notice shall be included in all
% // copies or substantial portions of the Software.
% //
% // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
% // FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
% // COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
% // IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
% // CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


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