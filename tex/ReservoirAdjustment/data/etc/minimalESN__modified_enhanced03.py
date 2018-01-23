# -*- coding: utf-8 -*-
"""
A minimalistic Echo State Networks demo with Mackey-Glass (delay 17) data
in "plain" scientific Python.
by Mantas LukoÅ¡eviÄ?ius 2012
http://minds.jacobs-university.de/mantas
---
Modified by Xavier Hinaut: 2015-2016
http://www.xavierhinaut.com
"""
#from numpy import *
import numpy as np
#from matplotlib.pyplot import *
import matplotlib.pyplot as plt
#import scipy.linalg
from scipy import linalg
#from numpy import linalg

def set_seed(seed=None):
    """Making the seed (for random values) variable if None"""

    # Set the seed
    if seed is None:
        import time
        seed = int((time.time()*10**6) % 4294967295)
    try:
        np.random.seed(seed)
    except Exception, e:
        print "!!! WARNING !!!: Seed was not set correctly."
        print "!!! Seed that we tried to use: "+str(seed)
        print "!!! Error message: "+str(e)
        seed = None
    print "Seed used for random values:", seed
    return seed

# load the data
trainLen = 2000
testLen = 2000
initLen = 100

data = np.loadtxt('MackeyGlass_t17.txt')

# plot some of it
plt.figure(0).clear()
plt.plot(data[0:1000])
plt.ylim([-1.1,1.1])
plt.title('A sample of input data')

# generate the ESN reservoir
inSize = outSize = 1 #input/output dimension
resSize = 300 #reservoir size (for prediction)
#resSize = 1000 #reservoir size (for generation)
a = 0.3 # leaking rate
spectral_radius = 1.25
input_scaling = 1.
reg =  1e-8 #None # regularization coefficient, if None, pseudo-inverse is use instead of ridge regression

mode = 'prediction'
#mode = 'generative'

#change the seed, reservoir performances should be averaged accross at least 20 random instances (with the same set of parameters)
seed = None #42

set_seed(seed) #random.seed(seed)
Win = (np.random.rand(resSize,1+inSize)-0.5) * input_scaling
W = np.random.rand(resSize,resSize)-0.5
# Option 1 - direct scaling (quick&dirty, reservoir-specific):
#W *= 0.135
# Option 2 - normalizing and setting spectral radius (correct, slow):
print 'Computing spectral radius...',
rhoW = max(abs(linalg.eig(W)[0]))
#np_rhoW = max(abs(np.linalg.eig(W)[0]))
#print "Difference between scipy and numpy .eig() method: scipy="+str(rhoW)+" numpy="+str(np_rhoW)
print 'done.'
W *= spectral_radius / rhoW

# allocated memory for the design (collected states) matrix
X = np.zeros((1+inSize+resSize,trainLen-initLen))
# set the corresponding target matrix directly
Yt = data[None,initLen+1:trainLen+1]

# run the reservoir with the data and collect X
x = np.zeros((resSize,1))
for t in range(trainLen):
    u = data[t]
    x = (1-a)*x + a*np.tanh( np.dot( Win, np.vstack((1,u)) ) + np.dot( W, x ) )
    if t >= initLen:
        X[:,t-initLen] = np.vstack((1,u,x))[:,0]

# train the output
X_T = X.T
if reg is not None:
    # use ridge regression (linear regression with regularization)
    Wout = np.dot(np.dot(Yt,X_T), linalg.inv(np.dot(X,X_T) + \
        reg*np.eye(1+inSize+resSize) ) )
    ### Just if you want to try the difference between scipy.linalg and numpy.linalg which does not give the same results
        ### For more info, see https://www.scipy.org/scipylib/faq.html#why-both-numpy-linalg-and-scipy-linalg-what-s-the-difference
#    np_Wout = np.dot(np.dot(Yt,X_T), np.linalg.inv(np.dot(X,X_T) + \
#        reg*np.eye(1+inSize+resSize) ) )
#    print "Difference between scipy and numpy .inv() method:\n\tscipy_mean_Wout="+\
#        str(np.mean(Wout))+"\n\tnumpy_mean_Wout="+str(np.mean(np_Wout))
else:
    # use pseudo inverse
    Wout = np.dot( Yt, linalg.pinv(X) )

# run the trained ESN in a generative mode. no need to initialize here,
# because x is initialized with training data and we continue from there.
Y = np.zeros((outSize,testLen))
u = data[trainLen]
for t in range(testLen):
    x = (1-a)*x + a*np.tanh( np.dot( Win, np.vstack((1,u)) ) + np.dot( W, x ) )
    y = np.dot( Wout, np.vstack((1,u,x)) )
    Y[:,t] = y
    if mode == 'generative':
        # generative mode:
        u = y
    elif mode == 'prediction':
        ## predictive mode:
        u = data[trainLen+t+1]
    else:
        raise Exception, "ERROR: 'mode' was not set correctly."

# compute MSE for the first errorLen time steps
errorLen = 2000 #500
mse = sum( np.square( data[trainLen+1:trainLen+errorLen+1] - Y[0,0:errorLen] ) ) / errorLen
print 'MSE = ' + str( mse )

# plot some signals
plt.figure(1).clear()
plt.plot( data[trainLen+1:trainLen+testLen+1], 'g' )
plt.plot( Y.T, 'b' )
plt.title('Target and generated signals $y(n)$ starting at $n=0$')
if mode == 'generative':
    plt.legend(['Target signal', 'Free-running predicted signal'])
elif mode == 'prediction':
    plt.legend(['Target signal', 'Predicted signal'])

# plt.figure(2).clear()
# #plot( X[0:20,0:200].T )
# plt.plot( X[0:2,0:2000].T ) #xav
# plt.legend(["biais", "input"])
# plt.ylim([-1.1,1.1])
# plt.title('Input $\mathbf{u}(n)$ and bias for 2000 time steps')
#
# plt.figure(3).clear()
# plt.plot( X[2:12,0:200].T )
# plt.ylim([-1.1,1.1])
# plt.title('Activations $\mathbf{x}(n)$ from Reservoir Neurons ID 0 to 9 for 200 time steps')
#
# plt.figure(4).clear()
# plt.plot( X[2:12,:].T )
# plt.ylim([-1.1,1.1])
# plt.title('Activations $\mathbf{x}(n)$ from Reservoir Neurons ID 0 to 9 for all time steps')
#
# plt.figure(5).clear()
# plt.plot( X[2:,0:200].T )
# plt.ylim([-1.1,1.1])
# plt.title('All reservoir activations $\mathbf{x}(n)$ for 200 time steps')

## Bug, I don't know why
# plt.figure(6).clear()
# plt.bar( range(1+inSize+resSize), Wout.T )
# plt.title('Output weights $\mathbf{W}^{out}$')

plt.show()
