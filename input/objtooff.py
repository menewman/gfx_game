import sys
import string

inobj = sys.argv[1]

outoff = str.split(inobj,'.')
outoff[1] = 'off'
outoff = string.join(outoff,'.')

fin = open(inobj, "r")
fout = open(outoff,"w")

with fin as f:
    content = f.readlines()
    
verts = 0
faces = 0

for line in content:
    if line[0] == 'v':
        verts += 1
    if line[0] == 'f':
        faces += 1
        
    
print verts
print faces

fout.write("OFF\n")
fout.write(str(verts) + " " + str(faces) + " 0\n")
for line in content:
    if line[0] == 'v':
        fout.write(line[2:len(line)])

for line in content:
    if line[0] == 'f':
        lineminus = line[2:len(line)]
        verts = str.split(lineminus,' ')
        fout.write(str(len(str.split(lineminus,' '))))
        for vert in verts:
            fout.write(" " + str(int(vert)-1))
        fout.write("\n");



fin.close()
fout.close()