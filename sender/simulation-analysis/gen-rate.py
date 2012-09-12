import sys
numarg=len(sys.argv)
# even number because we want rate and duration pairs +1, for the argv[0]
assert ((numarg%2==1) and (numarg >=3)); 
i=1
rate=[]
time=[]
while (i<numarg) :
   rate.append(int(sys.argv[i]));
   i=i+1;
   time.append(int(sys.argv[i]));
   i=i+1;
assert(len(rate)==len(time));

t=0;
###for i in range (0,len(rate)) :
### for j in range(0,time[i]*1000) :  # 1 hour in milliseconds 
###   t=t+1
###   print t,"\t",(rate[i]/(8.0*1000))  # because we want the credit in bytes and not bits 
### 
