import random
from math import sqrt

WINDOW_SIZE = 5

def avg(window):
    return window[WINDOW_SIZE-1] / WINDOW_SIZE

def alpha(window):
    # return (window[WINDOW_SIZE-1] - window[0]) / WINDOW_SIZE
    yfi = window[WINDOW_SIZE-1] - window[0]
    return yfi / sqrt(yfi**2 + WINDOW_SIZE**2)

def mpx_plus(mpx, n, window):
    # print(mpx, n, alpha(window), avg(window), window)
    mpx = (mpx*n*(alpha(window)) + (1-alpha(window))*avg(window)) / ((n+1) * alpha(window))
    return mpx

mpx = 0
for n in range(10000):
    window = []
    last = 0
    for _ in range(WINDOW_SIZE):
        last = random.randint(0,10) + last
        window.append(last)
        # print(last)

    mpx = mpx_plus(mpx, n, window)
    print(mpx, n) 