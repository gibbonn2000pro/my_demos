import zlib

f = open("zlib_data", "r")
s = f.read()
f.close()

s = zlib.decompress(s)

f = open("zlib_decompress_data", "w")
f.write(s)
f.close()