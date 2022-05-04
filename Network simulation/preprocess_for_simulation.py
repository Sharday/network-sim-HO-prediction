'''
@Author: Sharday Olowu
@Date: 03/05/2022

Script to pre-process mobility trace files into a format suitable for ns-3 simulation.

'''

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import math
from os import listdir
from os.path import isfile, join

dir = 'cabspottingdata/'
trace_files = [f for f in listdir(dir) if isfile(join(dir, f))]



valid_trace_files = []
rejected_trace_files = []
sumx = []
sumy = []

rejected = 0
fileNum = -1

for file in trace_files:
    fileNum += 1

    rejectX = 0
    rejectY = 0
    path = dir + file

    df = pd.read_csv(path, sep=' ', header = None)

    # column organisation
    headers=['x','y','occ','time']
    df.columns = headers
    df = df.reindex(columns = ['time','x','y','occ'])
    df = df.drop('occ', 1)

    # rescale timings
    df['time'] = df['time'] - df['time'].min()
    df = df.reindex(index=df.index[::-1])
    df["time"] = df["time"]/math.ceil(df["time"].max()/1000)
    df = df.reset_index(drop=True)

    df["x"] = 2000*(df["x"] - df["x"].min())
    df["y"] = 2000*(df["y"] - df["y"].min())

    xmax = df["x"].max()
    ymax = df["y"].max()

    for x, y in zip(df["x"].values, df["y"].values):
        if x < 620 or x > 880:
            rejectX += 1
        if y < 180 or y > 400:
            rejectY += 1



    prop_x = rejectX/len(df)
    prop_y = rejectY/len(df)

    # reject file if more than 30% of x or y co-ordinates are out of bounds
    if prop_x > 0.3 or prop_y > 0.3:
        rejected += 1
        continue

    # reject file if any co-ordinate is greater than 1500m
    elif xmax < 1500 and ymax < 1500:
        valid_trace_files.append(path)
        sumx.append(df["x"].values)
        sumy.append(df["y"].values)
        df.to_csv(r'for_sim/' + str(filenum) + '.txt', header=None, index=None, sep=',')

    else:
        rejected += 1


sumx = np.asarray(sumx)
sumy = np.asarray(sumy)

# visualise distributions of x and y co-ordinates
plt.hist(np.ravel(sumx))
plt.title("Histogram of x values")
plt.xlabel('x co-ordinate')
plt.show()
plt.hist(np.ravel(sumy))
plt.title("Histogram of y values")
plt.xlabel('y co-ordinate')
plt.show()


print("valid:",len(valid_trace_files))
print("rejected:",rejected)
print("total:",len(trace_files))
