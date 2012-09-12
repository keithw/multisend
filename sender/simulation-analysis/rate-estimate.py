import sys
fh=open(sys.argv[1])
acc=[0]*3700 # 1 hour of data
for line in fh.readlines() :
  time=int(float(line.split()[0]));
  acc[(time/1000)]=acc[(time/1000)]+8*1500; # ms to sec

for i in range(0,len(acc)) :
    print i,"\t",(acc[i]/1.0e6); # in mbps
