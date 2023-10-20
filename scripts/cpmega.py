import sys

def mega():
    copyfrom = "{}/data/100M.data".format(sys.argv[2])
    file = open(copyfrom, "rb")
    data = file.read()
    for i in range(10):
        with open("{}/data/vm{}_100M_{:06d}.data".format(sys.argv[2], sys.argv[1], i), "wb") as out:
            out.write(data)

mega()
