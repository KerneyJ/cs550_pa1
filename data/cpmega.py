import sys

def mega():
    copyfrom = "./1M.data"
    file = open(copyfrom, "rb")
    data = file.read()
    for i in range(1000):
        with open("./mega/vm{}_{:03d}_1M.data".format(sys.argv[1], i), "wb") as out:
            out.write(data)

mega()
