import wave
import matplotlib.pyplot as plt
import numpy as np
import os
 
filepath  = "C:/Users/yaojieno1/Music/"
filename  = "20181027-7th-2.wav"
isShowBin = False
threshold = 0
framerate = 0
nframes   = 0
waveData  = []

isDebug = False

def log(fmt):
    if isDebug:
        print(fmt)

def readfile():
    global filepath, filename, framerate, waveData, nframes
    
    # 读取文件，获取通道数/帧率
    f = wave.open(filepath+filename,'rb')
    params = f.getparams()
    nchannels, sampwidth, framerate, nframes = params[:4]
    log("nchannels=" + str(nchannels)+" samwidth="+str(sampwidth)+" framerate="+str(framerate)+" nframes="+str(nframes))

    # 读取音频值
    strData = f.readframes(nframes)
    waveData = np.fromstring(strData,dtype=np.uint8)

    # 归一化
    waveData = waveData * 1.0 / 128  #(max(abs(waveData)))
    waveData = np.reshape(waveData,[nframes,nchannels]).T

    print("len(waveData)=" + str(len(waveData[0])))
    f.close()

#==============

def search_max(arr):
    m = 0
    c = -1

    for i in range(len(arr)):
        if arr[i].real > m:
            m = arr[i].real
            c = i
    return c, m

#==============

def calc_freq(start, end, index):
    global waveData, framerate, threshold

    if end == 0:
        end = start + 1024
    
    N  = end - start       # 采样个数
    df = framerate / (N-1) # 分辨率
    freq = [ df * n for n in range(0, N) ]

    data=waveData[0][start:end]

    c = np.fft.fft(data) # * 2 / N
    e = int(len(c) / 2)
    while freq[e] > 2000:   
        e -= 10

    b = 0
    while freq[b] < 400:
        b += 10

    if index > 0:
        plt.figure(index)
        plt.plot(freq[b:e-1], abs(c[b:e-1]))

    i, m = search_max(abs(c[b:e-1]))
  
    if (i > 0):
        i += b

    log("m=" + str(m) + " i=" + str(i) + " freq[i]=" + str(freq[i])  + " start=" + str(start) + " end=" + str(end))

    if m < threshold or i == -1:
        return -1

    if (freq[i] < 600):
        return 0

    if (freq[i] > 1100):
        return 1

    log("{EXCEPTION] output = 8" + " m=" + str(m) + " i=" + str(i) + " start=" + str(start) + " end=" + str(end) + " freq[i]=" + str(freq[i]))
    return 8


def show_result(final):
    global threshold, isShowBin

    if (len(final) < 1):
        return

    for v in final:
        o = ord(v)
        if (o == 0xA or o == 0x9 or o==0xD or o==0x11):
            continue
        if (o < 0x20 or o > 0x7F):
            if (False == isShowBin):
                return
    print("threshold:" + str(threshold) + " result: " + ("".join(final)))


def proc():
    global nframes

    i = 0
    t = -1
    val = []
    final = []
    while i < nframes:
        s = calc_freq(i, 0, -1)
        if s == -1 or s == 8:
            if t != -1:
                log("t == -1, i=" + str(i))
                t = -1
        elif s == 1:
            if t == 0.7:
                log("val[].append(1), i=" + str(i))
                val.append(1)
                t = 1
            elif t != 1:
                t = 0.7
                log("t=0.7, i=" + str(i));
        elif s == 0:
            if t == 0.2:
                log("val[].append(0), i=" + str(i))
                val.append(0)
                t = 0
            elif t != 0:
                t = 0.2
                log("t=0.2, i=" + str(i));
                

        i += 200
        if(len(val) == 8):
            v = 0
            for j in range(8):
                v = (v << 1) + val[j]
            log("v=" + hex(v) + " chr=" + chr(v))
            final.append(chr(v))
            val = []
    show_result(final)


def check(beg, end):
    n = 1
    k = beg
    while k <= end:
        print(calc_freq(k, 0, n))
        n += 1
        k += 200
    plt.show()

def process():
    global threshold

    for x in range(10,36):
        threshold = x / 10
        proc()


def main():
    readfile()
    process()
    # check(5000, 8000)


main()
