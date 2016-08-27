import os;
import os.path;
import sys;
n = hou.node("/obj/box/solver1");
for i in xrange(1, 5000):
    print "Start writing frame " + str(i) + "...\n";
    Str = "C:\\Simulation\\Velocity\\frame_" + str(i) + ".txt";
    f = open(Str, "w");
    for point in n.geometryAtFrame(i).points():
        data = point.attribValue("P");
        Str = str(data[0]) + "," + str(data[1]) + "," + str(data[2]) + "\n";
        f.write( Str );
    f.close();
    print "End writing frame " + str(i) + "\n";
