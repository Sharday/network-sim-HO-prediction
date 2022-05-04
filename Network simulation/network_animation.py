'''
@Author: Sharday Olowu
@Date: 03/05/2022

This work is adapted from a Stack Overflow answer by Joe Kington:
https://stackoverflow.com/questions/9401658/how-to-animate-a-scatter-plot

Modifications:
➢ Simplification of code
➢ Reading simulation output file into a dataframe
➢ Selection of key values from dataframe and use of a generator to feed a
   stream of x and y co-ordinates
➢ Setting up base stations in the required positions for the network scenario
➢ Rescaling of axes to view required area
➢ Reformatting of colours and shapes (UE as red circle, base stations as
   triangles of different colours)

'''


import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import pandas as pd
from matplotlib.colors import ListedColormap
from sklearn.preprocessing import MinMaxScaler

class AnimatedScatter(object):
    """An animated scatter plot using matplotlib.animations.FuncAnimation."""
    def __init__(self, filename, numpoints=50):
        self.numpoints = numpoints
        self.filename = filename
        self.stream = self.data_stream()

        # Setup the figure and axes...
        self.fig, self.ax = plt.subplots()
        # Then setup FuncAnimation.
        self.ani = animation.FuncAnimation(self.fig, self.update, interval=10,
                                          init_func=self.setup_plot, blit=False)

    def setup_plot(self):
        """Initial drawing of the scatter plot."""
        x, y = next(self.stream).T
        c = 'red'
        s = 50

        # 2-cell scenario only
        self.scat = self.ax.scatter(x=675, y=180, c='blue', s=400, marker="^", edgecolor="k") #enb1
        self.scat = self.ax.scatter(x=800, y=300, c='green', s=400, marker="^",edgecolor="k") #enb2

        # add on for 4-cell arrangement A
        self.scat = self.ax.scatter(x=925, y=180, c='purple', s=400, marker="^", edgecolor="k") #enb3
        self.scat = self.ax.scatter(x=550, y=300, c='pink', s=400, marker="^",edgecolor="k") #enb4

        # add on for 4-cell arrangement B
        # self.scat = self.ax.scatter(x=675, y=300, c='purple', s=400, marker="^", edgecolor="k") #enb3
        # self.scat = self.ax.scatter(x=800, y=180, c='pink', s=400, marker="^",edgecolor="k") #enb4

        # add on for 4-cell arrangement C
        # self.scat = self.ax.scatter(x=675, y=260, c='purple', s=400, marker="^", edgecolor="k") #enb3
        # self.scat = self.ax.scatter(x=800, y=220, c='pink', s=400, marker="^",edgecolor="k") #enb4


        self.scat = self.ax.scatter(x, y, c=c, s=s, vmin=0, vmax=1,
                                    cmap='hsv', edgecolor="k")
        self.ax.axis([400, 1075, 100, 375])
        self.ax.set_xlabel('x')
        self.ax.set_ylabel('y')


        # For FuncAnimation's sake, we need to return the artist we'll be using
        # Note that it expects a sequence of artists, thus the trailing comma.
        return self.scat,

    def data_stream(self):

        df = pd.read_csv(self.filename, sep=",", header=None)
        xy = df.iloc[:,[1,2]].values


        for i in range(len(df)):
            yield np.reshape(xy[i], (-1,2))

    def update(self, i):
        """Update the scatter plot."""
        data = next(self.stream)
        print(data)

        # Set x and y data...
        self.scat.set_offsets(data)

        self.scat.set_color('r')

        # We need to return the updated artist for FuncAnimation to draw..
        # Note that it expects a sequence of artists, thus the trailing comma.
        return self.scat,


if __name__ == '__main__':
    filename = 'sim_outputs_4_cells/0.txt'
    a = AnimatedScatter(filename, 1)
    plt.show()
