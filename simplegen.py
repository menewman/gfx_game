# usage:
# python gen.py numtrees > file.scn

import sys, random, math

# camera
print "camera 0 16 67  0.0287 -0.2 -0.979  -0.004 0.979 -0.200  0.25  0.01 1000"

# material 0: grass
print "material   0 0 0   0.5 0.5 0.5   0.5 0.5 0.5   0 0 0   0 0 0   10 1 grass.jpg"
# material 1: bark
print "material   0 0 0   0.5 0.5 0.5   0.5 0.5 0.5   0 0 0   0 0 0   10 1 bark.jpg"
# material 2: leaves
print "material   0 0 0   0.5 0.5 0.5   0.5 0.5 0.5   0 0 0   0 0 0   10 1 leaves.jpg"

# background: sky color
print "background 0.529 0.808 0.980"

# direct sunlight
print "dir_light  0.950 0.950 0.824  -0.3 -0.7 0"

# ambient light
print "ambient  1 1 1"

# ground box
print "box    0    -500 -0.5 -500    500 0 500"

# trees
for i in range(0, int(sys.argv[1])):
    r = random.random()*6 + 4
    w = random.random()*0.5 + 0.5
    h = random.random()*10 + 15
    x = (random.random()*2.0 - 1.0)*500.0
    z = (random.random()*2.0 - 1.0)*500.0

    print "cylinder 1 " + str(x) + " " + str(h/2) + " " + str(z) + " " + str(w) + " " + str(h)
    print "sphere 2 " + str(x) + " " + str(h) + " " + str(z) + " " + str(r)
