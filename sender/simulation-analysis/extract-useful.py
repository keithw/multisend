import sys
fh=open(sys.argv[1]);
start=int(sys.argv[2]);
end=int(sys.argv[3]);
repeats=int(sys.argv[4]);
useful=[]

# 3G : 1000 secs to 1200 secs
# 4G : 200 to 415 seconds
start=start*1000
end=end*1000
for line in  fh.readlines() :
    ts= int(line)
    if((ts>=start) and (ts<=end)) :
        useful.append(ts-start)

for i in range(0,repeats) :
    for j in range(0,len(useful)):
       print useful[j]+i*(end-start)

