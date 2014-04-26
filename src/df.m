%% Copyright (C) 2014 William W Smith Jr
%%
%% Identify Angle-of-Arrival given two sampled signals, sample frequency, and baseline distance between sensors


%
%  Test inputs
%

scrsz = get(0,'ScreenSize');
hfig = figure;
set(hfig,'position',scrsz);



t = tcpip('0.0.0.0',3000,'NetworkRole','server','Terminator','');
set(t,'InputBufferSize',120000);

disp('Waiting for connection');
fopen(t);
numSamples = 0;
signal1 = [];
signal2 = [];
while true
    while (length(signal1) < 2^16) || (length(signal2) < 2^16) %.5 seconds of data
        channel = fread(t,1,'uint32');         % channel number
        arraySize = fread(t,1,'uint32');           % sample count
        data = fread(t,arraySize,'uint32');     % sample data
        numSamples = numSamples + arraySize;
        if(channel == 1)
            signal1 = [signal1; data];
        else if(channel == 2)
                signal2 = [signal2; data];
            end
        end 
    end
    
    
    fs = 96000;		% [Samples/Second]
    baseline = 1.5;		% [meter]
    
    if(length(signal1) < length(signal2))
        signal2 = signal2(1:end-1);
    else if length(signal2) < length(signal1)
            signal1 = signal1(1:end-1);
        end
    end
    
    
    %% usage: angle = df(signal1, signal2, baseline, fs)
    %%
    %% angle[degrees] = estimate of angle of arrival
    %%
    %% signal1, signal2 = sampled signal arrays
    %% baseline = distance between sensors [meters]
    %% fs = sample frequency [Samples/Second]
    %%
    
    % To turn this script into a function, use this line
    %
    % function angle = df(signal1, signal2, baseline, fs)
    
    
    cair = 343; 		% [meters/Second in 20C air]
    cwater = 1497;   	% [meters/Second in 25C fresh water]
    cwater = 1560;   	% [meters/Second in salt water]
    c = cair;		% Air for now
    
    %
    %
    
    samplelength = c/fs;		% length of a sample [meters]
    samplemax = floor(baseline/samplelength); % max possible offset [Samples]
    
    nn = (-samplemax):1:(samplemax);	% delay sample index [Samples]
    lagmax = samplemax/fs;		% maximum delay across
    tautau = nn * 1/fs;		% lags [Seconds]
    
    
    % AC Couple both signals, get rid of DC components
    %
    sig1AC = signal1 - mean(signal1);
    sig2AC = signal2 - mean(signal2);
    
    
    % Cross-Correlate the two signals
    %
    
    % 1 Using FFT Techniques
    %
    lags12 = real( ifft( fft(sig1AC) .* conj( fft(sig2AC) ) ) );
    
    lags12zeroindex = fix(length(lags12)/2);   % Put Zero delay in middle of plot
    lags12 = circshift(lags12, lags12zeroindex);
    
    % 2 Using the Dot-Product
    %
    % Out of time: Try this later as it may be faster over the range of possible solutions
    
    % Plot results
    %
    % Remove this if only need numerical result
    %
    
    
    subplot(4,1,1)
    plot(sig1AC);
    title('Signal1 AC Coupled');
    xlabel('[Samples]');
    subplot(4,1,2)
    plot(sig2AC);
    title('Signal2 AC Coupled');
    %xlabel('[Samples]');
    
    %subplot(6,1,3);
    %plot( (-lags12zeroindex):(lags12zeroindex-1), lags12 );
    %title('XCorr(1,2)');
    %xlabel('lags');
    
    %subplot(6,1,4)
    %plot(nn, lags12( (lags12zeroindex-samplemax):(lags12zeroindex+samplemax) ) );
    %title('XCorr(1,2)');
    %xlabel('lags');
    
    
    subplot(4,1,3)
    plot(1000* 1000* tautau, lags12( (lags12zeroindex-samplemax):(lags12zeroindex+samplemax) ) );
    title('XCorr(1,2)');
    xlabel('[uSec]');
    
    angleindex = 180/pi*asin(tautau * c / baseline);
    
    subplot(4,1,4)
    plot(angleindex, lags12( (lags12zeroindex-samplemax):(lags12zeroindex+samplemax) ) );
    title('XCorr(1,2)');
    xlabel('[Degrees]');
    
    
    % Polar Plot
    %
    theta = -2*pi*angleindex/360 + pi/2;
    rho = lags12( (lags12zeroindex-samplemax):(lags12zeroindex+samplemax) );
    polar(theta, rho' );
    
    [maxValue, maxIndex] = max( lags12( (lags12zeroindex-samplemax):(lags12zeroindex+samplemax) )  );
    %maxValue
    %maxIndex
    
    
    % This is the best estimate of the Angle-of-Arrival
    %
    angle = angleindex(maxIndex)	% Estimate of Angle-of-Arrival [Degrees]
    drawnow;
    % Note that maxValue will provide a quality-of-service with bigger values indicating a better estimate for amplitude normalized input signals
    
    
    % endfunction
%     input('Press any key to restart');
    signal1 = [];
    signal2 = [];
    flushinput(t);
    numSamples = 0;
end



% Note that angles and delays are offset to be consistent with each other in the graphs.
