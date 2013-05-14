# usage:
# python gen.py numtrees numbushes

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
    w = random.random()*2.0 + 1.25
    h = random.random()*2.0 + 2.0
    botw = random.random()/2.0
    x = (random.random()*2.0 - 1.0)*500.0
    z = (random.random()*2.0 - 1.0)*500.0

    print "begin -1"

    print  str(w) + " 0 0 " + str(x)
    print "0 " + str(h) + " 0 0"
    print "0 0 " + str(w) + " " + str(z)
    print "0 0 0 1"

    print "mesh 2 pinetree1top.off"

    print "begin -1"

    print  str(botw) + " 0 0 0"
    print "0 1 0 0"
    print "0 0 1 0"
    print "0 0 0 1"

    print "mesh 1 pinetree1bot.off"

    print "end"
    print "end"

# bushes
for i in range(0, int(sys.argv[2])):
    a = random.random()*2.0*math.pi
    c = math.cos(a)
    s = math.sin(a)
    n = -s
    h = random.random()/2.0 + 0.75
    x = (random.random()*2.0 - 1.0)*500.0
    z = (random.random()*2.0 - 1.0)*500.0

    print "begin -1"

    print  str(c) + " 0 " + str(s) + " " + str(x)
    print "0 " + str(h) + " 0 0"
    print  str(n) + " 0 " + str(c) + " " + str(z)
    print "0 0 0 1"

    print "mesh 2 bush1top.off"

    print "begin -1"

    print "1 0 0 0"
    print "0 1 0 0"
    print "0 0 1 0"
    print "0 0 0 1"

    print "mesh 1 bush1bot.off"

    print "end"
    print "end"
