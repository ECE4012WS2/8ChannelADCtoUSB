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


import matplotlib as mpl
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cbook as cbook
import socket
from multiprocessing import Process

#From StackOverflow
def get_files():
  numFiles = 1;
  s = socket.socket()
  s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  s.bind(("",3000))
  s.listen(10)
  print "Waiting for file: channel" + str(numFiles) + ".csv"
  while(numFiles < 9):
    sc, address = s.accept()
    f = open('channel'+ str(numFiles)+".csv",'w')

    l = sc.recv(1024)
    while (l):
            f.write(l)
            l = sc.recv(1024)

    f.close()
    sc.close()
    print "Got file:  channel" + str(numFiles) + ".csv"
    numFiles = numFiles + 1
  s.close()



def read_datafile(file_name,plotnum):
    # the skiprows keyword is for heading, but I don't know if trailing lines
    # can be specified
    data = np.genfromtxt(file_name, delimiter=',',names=['x','y'])


    ax1 = fig.add_subplot(8,1,plotnum)

    # ax1.set_title(file_name)
    ax1.get_yaxis().set_ticks([])
    if plotnum ==1:
        ax1.set_xlabel('Sample Number')
        ax1.set_ylabel('V')
    ax1.get_xaxis().set_ticks([])
    if plotnum == 8:
        ax1.get_xaxis().set_ticks([0,len(data['x']) - 1])

    ax1.plot(data['x'],data['y'], c='r', label='the data')


    return data


fig = plt.figure()

get_files()


data = read_datafile('./channel1.csv',1)
data = read_datafile('./channel2.csv',2)
data = read_datafile('./channel3.csv',3)
data = read_datafile('./channel4.csv',4)
data = read_datafile('./channel5.csv',5)
data = read_datafile('./channel6.csv',6)
data = read_datafile('./channel7.csv',7)
data = read_datafile('./channel8.csv',8)
plt.show()
