clear, clc, close all

plot_size = 1000;
buf_size = 12000;
channels = zeros(8, buf_size);
ndx = ones(1,8);

t = 0:1/100:2*pi;

counter = 0;

c_ndx = 0;

t = tcpip('0.0.0.0',3000,'NetworkRole','server','Terminator','');
set(t,'InputBufferSize',120000);

disp('Waiting for connection');
fopen(t);

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
       if mod(counter, 8) == 0
           counter = 0;
           subplot(8,1,1);
           plot(1:plot_size, channels(1, 1:plot_size));
           subplot(8,1,2);
           plot(1:plot_size, channels(2, 1:plot_size));
           subplot(8,1,3);
           plot(1:plot_size, channels(3, 1:plot_size));
           subplot(8,1,4);
           plot(1:plot_size, channels(4, 1:plot_size));
           subplot(8,1,5);
           plot(1:plot_size, channels(5, 1:plot_size));
           subplot(8,1,6);
           plot(1:plot_size, channels(6, 1:plot_size));
           subplot(8,1,7);
           plot(1:plot_size, channels(7, 1:plot_size));
           subplot(8,1,8);
           plot(1:plot_size, channels(8, 1:plot_size));
           drawnow
       end
   %end
end