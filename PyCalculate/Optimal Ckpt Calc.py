import math
import matplotlib as plt
from matplotlib import pyplot

def prod_pred(b,N,j,R):
    temp = 1
    for k in range(0,j):
        temp = temp * ((2*(b-k)) + (R*(N-(2*b)+k)))/(N-k)
    return temp

def sum_pred(b,N,R):
    temp = 1
    for j in range(1,b+1):
        temp = temp + prod_pred(b,N,j,R)
    return temp

N = 8192
C = 215
M = 2000

xaxis = [0.1,0.25,0.33]
norecall = []
halfrecall = []
nintyrecall = []
fullrecall = []

for r in xaxis:
    for R in [0.5]:
        b = round(N*r)
        if b == 82:
            b = 80
        if b == 819:
            b = 832
        if b == 2703:
            b = 2704
        a = N-(2*b)
        Eab = sum_pred(b,N,R)
        Mintr = M*Eab
        Ckpt = C*(a+b)/N
        Topt = math.sqrt(2*Mintr*Ckpt)
        print("r =",r,", R =",R,", b =",b,"a =",a," a+b =",a+b,"(a+b)/N",(a+b)/N,"Eab =",Eab,"Mintr =",Mintr,"Ckpt =",Ckpt,"Topt =",Topt)
        if R == 0:
            norecall.append(Eab)
        else: 
            if R == 0.5:
                halfrecall.append(Eab)
            if R == 0.9:
                nintyrecall.append(Eab)
            if R == 1:
                fullrecall.append(Eab)