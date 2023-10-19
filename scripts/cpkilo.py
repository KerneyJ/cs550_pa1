import sys

def kilo():
    copyfrom = "{}/data/1K.data".format(sys.argv[2])
    file = open(copyfrom, "rb")
    data = file.read()
    for i in range(10000):
        with open("{}/data/vm{}_1K_{:06d}.data".format(sys.argv[2], sys.argv[1], i), "wb") as out:
            out.write(data)

kilo()
