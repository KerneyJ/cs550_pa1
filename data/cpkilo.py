import os
import sys

def kilo():
    copyfrom = "./1K.data"
    file = open(copyfrom, "rb")
    data = file.read()
    for i in range(1000000):
        with open("./kilo/{:06d}.data".format(i), "wb") as out:
            out.write(data)

kilo()
