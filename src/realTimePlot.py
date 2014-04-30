# // The MIT License (MIT)
# //
# // Copyright (c) 2014 Austin Ward, Yao Lu, Fujun Xie, Eric Patterson, Mohan Yang
# //
# // Permission is hereby granted, free of charge, to any person obtaining a copy of
# // this software and associated documentation files (the "Software"), to deal in
# // the Software without restriction, including without limitation the rights to
# // use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# // the Software, and to permit persons to whom the Software is furnished to do so,
# // subject to the following conditions:
# //
# // The above copyright notice and this permission notice shall be included in all
# // copies or substantial portions of the Software.
# //
# // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# // FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# // COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# // IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# // CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.



import matplotlib.pyplot as plt
import time
import sys
import os
import fcntl
import errno
import random
from collections import deque
import numpy as np
import socket


def onclick(event):
    print 'button=%d, x=%d, y=%d, xdata=%f, ydata=%f'%(
        event.button, event.x, event.y, event.xdata, event.ydata)


port = 3000
bufferSize = 3000

s = socket.socket()
s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)

s.bind(("",port))
s.listen(10)
print "Socket created on port " + str(port)


fig = plt.figure()
num = fig.number
cid = fig.canvas.mpl_connect('button_press_event', onclick)
subplot = []
dataBuffersY = []
dataBuffersX = []

for i in range(1,9):
  print "Initializing plot " + str(i)
  dataBuffersX.append(deque(maxlen = bufferSize))
  dataBuffersY.append(deque(maxlen = bufferSize))
  for j in range(0,bufferSize):
    dataBuffersX[i-1].append(j)
    dataBuffersY[i-1].append(0)
  subplot.append(fig.add_subplot(8,1,i,autoscalex_on=True,autoscaley_on=True))
  subplot[i-1].get_yaxis().set_ticks([])
  subplot[i-1].get_xaxis().set_ticks([])




print "Ready for socket connection"
sc,address = s.accept()
sc.setblocking(0)

print "New connection " + str(address)
plt.ion()
plt.show()

while True:
    if not plt.fignum_exists(num):
      exit(1)
    plt.draw()
    # time.sleep(0.1)
    # plt.pause(0.0001)

    try:
        data = sc.recv(1024)
        if len(data) > 0:
          # print "Got data: " + data
          dataList = data.split('\n')
          for xy in dataList:
              splitList = xy.split(',')
              if(len(splitList) == 2):
                x = splitList[0]
                y = splitList[1]
                for i in range(1,9):
                  dataBuffersY[i-1].append(y)
                  dataBuffersX[i-1].append(int(x) % bufferSize)
                  # print "X: " + str(dataBuffersX[i-1])
                  # print "Y: " + str(dataBuffersY[i-1])
                  # subplot[i-1].cla()
                  subplot[i-1].plot(dataBuffersX[i-1],dataBuffersY[i-1],c='r',label='the data')
    except socket.error, e:
        err = e.args[0]
        if err == errno.EAGAIN or err == errno.EWOULDBLOCK:
          #no data available
            continue
        else:
            # a "real" error occurred
            print e
            exit(1)
            continue
    except ValueError:
      continue
