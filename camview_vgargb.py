#!/usr/bin/env python

# Display VGA rgb444 images from the OV767/FIFO module

# modified by HN
# remove sleep delay from writeslow
# use 'COM10' instead of 5 for serial port
# use 230400 instead of 115200 for baudrate
# VGA instead of QVGA
# saves binary file on PC 

import pygame
import sys
import serial
import time
import re
import struct


def parsergb444(byte1, byte2):
    byte12 = (byte1 << 8) | byte2
    red = (byte12 >> 8) & 0x0f
    grn = (byte12 >> 4) & 0x0f
    blu = byte12 & 0x0f
    red *= 16
    grn *= 16
    blu *= 16
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
        time.sleep(0.1)
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
        sys.stdout.write('\rReading line %d/%d [%d%%]' % \
            (y + 1, image_height,
            int((y + 1) / float(image_height) * 100.0)))
        sys.stdout.flush()
        writeslow('read %d\r' % ((image_width * 2),))
        l = ser.read(image_width * 2)
        if len(l) != (image_width * 2):
            print '\nonly got %d bytes!' % (len(l),)
            sys.exit(1)
        imagebuf[y] = l
    print
    return imagebuf


# the software uart on the ov7670 project is not perfect...
def writeslow(s):
    for c in s:
        ser.write(c)
        # time.sleep(0.01)


def saveimage():
    fp = open("vgargb444.bin","wb")
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
    for y in range(0, image_height):
        i = 0
        for x in range(0, image_width):
            color = parsergb444( ord(buf[y][i]), ord(buf[y][i + 1]) )
            screen.set_at((x, y), color)
            i += 2


if __name__ == '__main__':

    image_width = 640
    image_height = 480
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
            timeout = 10
        )
    ser.close()
    ser.open()
    ser.isOpen()
    print 'Serial port open'

    print 'Reading image from camera...'
    starttime = time.time()
    buf = readimage()
    print 'Read complete in %.3f seconds' % (time.time() - starttime)
    #saveimage()
    #print 'Saved binary file vgargb444.bin'
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
                    #saveimage()
                    pass
        pygame.display.flip()

        clock.tick(240)

