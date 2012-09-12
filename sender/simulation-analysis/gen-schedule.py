import sys
from math import exp
numarg=len(sys.argv)
# specify packets per second and time 
# even number because we want rate and duration pairs +1, for the argv[0]
##########3assert ((numarg%2==1) and (numarg >=3)); 
##########3i=1
##########3rate=[]
##########3time=[]
##########3while (i<numarg) :
##########3   rate.append(int(sys.argv[i]));
##########3   i=i+1;
##########3   time.append(int(sys.argv[i]));
##########3   i=i+1;
##########3assert(len(rate)==len(time));
##########3
##t=0;
##for i in range (0,len(rate)) :
## num_pkts=time[i]*rate[i];
## for pkt_id in range(0,num_pkts) :  # 1 hour in milliseconds 
##   t=t+(1000/float(rate[i]))
##   print int(round(t))


# 1000 packets a second at the beginning to allow awesome video
t=0
#for i in range(0,400*1000) : # generate a packet every ms 
#   t=t+1
#   print int(round(t))
#
# gradually reduce , this is convex downward 
#for i in range(0,400*1000) : # generate a packet every ms 
#   t=t+1+float(i/100000.0)       # 40*1000 is 40 seconds. 
#   print int(round(t))
#

# concave downward  decrease
def rate(x):
  return ( 1 - 1.1/(exp(0.007*(1400-x)) + 1)) if (x>400) else 1 

for i in range (0,1600):
    delta=1/rate(i)
    t=i*1000
    while (t< (i+1)*1000) :
      print int(round(t))
      t=t+delta;
