from random import *
import sys

seed()

vertex_num = int(sys.argv[1])

edge_num = vertex_num * (vertex_num - 1) / 2

f = open("maxcut" + str(vertex_num) + ".txt", "w")
f.write(str(vertex_num) + " " + str(edge_num) + "\n")

for i in range(0, vertex_num):
    for j in range(i + 1, vertex_num):
		    f.write(str(i + 1) + " " + str(j + 1) + " " + str(randrange(-100, 100, 1)) + "\n")

print "Done!"
