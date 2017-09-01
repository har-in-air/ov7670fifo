#!/usr/bin/env python

# Display QVGA YUV images from the OV7670+AL422FIFO module

# modified by HN
# remove sleep delay from writeslow
# use 'COM10' instead of 5 for serial port
# use 230400 instead of 115200 for baudrate
# YUV instead of RGB
# saves binary file on PC 

import pygame
import sys
import serial
import time
import re
import struct

def parseyuv(byte1, byte2):
    red = byte1
    grn = byte1
    blu = byte1
    return (red, grn, blu)

def yuv2rgb(y, u, v):
    red = y + 1.408* (v - 128)
    grn = y - 0.345*(u - 128)  - 0.717*(v - 128)
    blu = y + 1.779*(u - 128)
    if red < 0:
        red = 0
    if red > 255:
        red = 255
    if grn < 0:
        grn = 0
    if grn > 255:
        grn = 255
    if blu < 0:
        blu = 0
    if blu > 255:
        blu = 255
    return (red, grn, blu)

def camera_capture():
    l = ''
    while l != 'OK':
        writeslow('cap\r')
        time.sleep(1)
        l = ser.readline().strip()

def camera_rrst():
    l = ''
    while l != 'OK':
        writeslow('rrst\r')
        time.sleep(0.2)
        l = ser.readline().strip()

def readimage():
    imagebuf = [None] * image_height
    print 'Requesting new image'
    camera_capture()
    print 'New image taken, resetting read pointer'
    camera_rrst()
    print 'Transferring buffer via serial'
    l = ''
    for y in range(0, image_height):
        sys.stdout.write('\rReading line %d/%d [%d%%]' % (y + 1, image_height,int((y + 1) / float(image_height) * 100.0)))
        sys.stdout.flush()
        writeslow('read %d\r' % ((image_width * 2),))
        l = ser.read(image_width * 2)
        if len(l) != (image_width * 2):
            print '\nonly got %d bytes!' % (len(l),)
            sys.exit(1)
        imagebuf[y] = l
    print
    return imagebuf

def readimagef():
    imagebuf = [None] * image_height
    print 'Requesting new image'
    camera_capture()
    print 'New image taken, resetting read pointer'
    camera_rrst()
    print 'Transferring buffer via serial'
    bfr = ''
    sys.stdout.flush()
    writeslow('rimg\r')
    bfr = ser.read(image_height* image_width * 2)
    if len(bfr) != (image_width*image_width * 2):
        print '\nonly got %d bytes!' % (len(bfr),)
        sys.exit(1)
    for yy in range(0, image_height):
        for xx in range(0, image_width*2):
            imagebuf[yy][xx] = bfr[yy*image_width + xx]
    print
    return imagebuf

# the software uart on the ov7670 project is not perfect...
def writeslow(s):
    for c in s:
        ser.write(c)
        # time.sleep(0.01)



def saveimage():
    fp = open("qvgayuv.bin","wb")
    for y in range(0, image_height):
        i = 0
        for x in range(0, image_width):
            h = ord(buf[y][i])
            l = ord(buf[y][i + 1])
            fp.write(struct.pack('B',h))
            fp.write(struct.pack('B',l))
            i += 2            
    fp.close()            


def drawimage():
    for row in range(0, image_height):
        i = 0
        while i < image_width*2:
            col = i / 2 
            y0 = ord(buf[row][i])
            u = ord(buf[row][i + 1])
            y1 = ord(buf[row][i + 2])
            v = ord(buf[row][i + 3])
            color = yuv2rgb(y0,u,v)
            screen.set_at((col, row), color)
            color = yuv2rgb(y1,u,v)
            screen.set_at((col+1, row), color)
            i += 4


if __name__ == '__main__':

    image_width = 320
    image_height = 240
    if len(sys.argv) != 3:
        print str(sys.argv[0]) + ' COMx baudrate'
        sys.exit(1)
    comport = str(sys.argv[1])
    baud = sys.argv[2]
    
    ser = serial.Serial(
            port = comport,
            baudrate = baud,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS,
            timeout = 100
        )
    ser.close()
    ser.open()
    ser.isOpen()
    print 'Serial port open'

    print 'Reading image from camera...'
    starttime = time.time()
    buf = readimage()
    print 'Read complete in %.3f seconds' % (time.time() - starttime)
    saveimage()
    print 'Saved binary file qvgayuv.bin'
    print 'Opening window'
    width = image_width
    height = image_height
    screen = pygame.display.set_mode((width, height))
    clock = pygame.time.Clock()

    running = True
    while running:
        drawimage()
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif (event.type == pygame.KEYDOWN):
                if (event.key == pygame.K_SPACE):
                    buf = readimage()
                    saveimage()
                    pass
        pygame.display.flip()

        clock.tick(240)

