import sys

def mega():
    copyfrom = "{}/data/1M.data".format(sys.argv[2])
    file = open(copyfrom, "rb")
    data = file.read()
    for i in range(1000):
        with open("{}/data/vm{}_1M_{:06d}.data".format(sys.argv[2], sys.argv[1], i), "wb") as out:
            out.write(data)

mega()
