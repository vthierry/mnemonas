#!/usr/bin/env python
import sys, os
import numpy as np
import matplotlib.pyplot as plt
import argparse
import math

def plot_activation(activation, name = "?", time_limit = 10000):
    f = plt.figure(figsize = (5,2))
    p = f.add_subplot(1,1,1)
    p.set_title(name)
    p.plot(range(time_limit), activation[:time_limit])
    return f


def main(argv):
    print(argv)
    # SETTING THE ARGS
    parser = argparse.ArgumentParser(description=argv[0]+': Script generating data following the macky glass equation')
    parser.add_argument("--alpha", nargs = "?", help= "alpha parameter", default = 0.2, type = float)
    parser.add_argument("--beta", nargs = "?", help= "beta parameter", default = 10., type = float)
    parser.add_argument("--gamma", nargs = "?", help= "gamma parameter", default = 0.1, type = float)
    parser.add_argument("--delta", nargs = "?", help= "delta parameter (sampling constant)", default = 0.1, type = float)
    parser.add_argument("--tau", nargs = "?", help= "tau parameter", type = int, default = 17)
    parser.add_argument("--init", nargs = "?", help= "initial value of the time serie", type = float, default = 1.2)
    parser.add_argument("--output", nargs = "?", help= "directory/file where the sequence will be saved, if not provided then it won't be saved", type = str, default = None)
    parser.add_argument("time", nargs = "?", help= "number of time sample", type = int, default = 10000)
    parser.add_argument("-p", action = "store_true", help= "if the option is set then there will be plotting debugs")
    parser.add_argument("--rmparams", action = "store_true", help= "if the option is set then the params won't be added to the file name")
    args = parser.parse_args(argv[1:])
    #print(args)

    # PYTHON VERSION
    print(sys.version)

    y = np.empty(shape = (1, args.time, 1))
    y[0,0] = args.init
    delay = math.floor(args.tau/args.delta)-1
    for n in range(1, args.time):
        y_delay = y[0,n-delay] if n-delay >= 0 else args.init
        y[0,n] = y[0,n-1]+args.delta*((args.alpha*y_delay/(1.+y_delay**args.beta))-args.gamma*y[0,n-1])

    if args.output != None:
        if args.rmparams:
            path = args.output
        else:
            path = args.output+"_t{0}_alpha{1}_beta{2}_gamma{3}_delta{4}_init_{5}_tau{6}".format(args.time, args.tau, args.beta, args.gamma, args.delta, args.init, args.tau)
        np.save(path, np.tanh(y))

    if args.p:
        plot_activation(np.tanh(y)[0], name = "Mackey")
        plt.show()

if __name__ == "__main__":
    main(sys.argv)
else:
    print("There is no reason to import this file at the moment")
