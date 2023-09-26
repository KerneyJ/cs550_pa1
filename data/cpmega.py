def mega():
    copyfrom = "./1M.data"
    file = open(copyfrom, "rb")
    data = file.read()
    for i in range(1000):
        with open("./mega/{:03d}.data".format(i), "wb") as out:
            out.write(data)

mega()
