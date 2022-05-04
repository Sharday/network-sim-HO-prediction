#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
@Author: Sharday Olowu
@Date: 03/05/2022

@Sources:
This work is built upon test.py, an example program written by Piotr Gawlowicz 
to demonstrate the use of the ns3-gym interface - available in the ns3-gym Github 
repository https://github.com/tkn-tub/ns3-gym at scratch/opengym.

Modifications:
➢ Loading of data scalers and pre-trained machine learning models for performing 
   inference to obtain network measurement predictions, using observations from 
   the simulation (network_sim.cc)
➢ Network measurement predictions (RSRP and S-PCI) fed back to the simulation as 
   actions
➢ Storing of recent simulation observations in a queue structure, which is updated 
   after each interaction
➢ Data pre-processing of observations before ML inference
➢ Simulation time parameter modified to reflect taxi simulation scenario in 
   network_sim.cc

'''

import argparse
from ns3gym import ns3env
import tensorflow as tf
from tensorflow import keras
import numpy as np
import pandas as pd
from tensorflow.keras.models import Sequential, Model, load_model
from tensorflow.keras.layers import LSTM
from tensorflow.keras.layers import Dense, Dropout
from sklearn.preprocessing import StandardScaler
from tensorflow.keras.layers import *
from tensorflow.keras.callbacks import ModelCheckpoint
from tensorflow.keras.losses import MeanSquaredError
from tensorflow.keras.metrics import RootMeanSquaredError
from tensorflow.keras.optimizers import Adam
import pickle
from collections import deque

__author__ = "Piotr Gawlowicz"
__copyright__ = "Copyright (c) 2018, Technische Universität Berlin"
__version__ = "0.1.0"
__email__ = "gawlowicz@tkn.tu-berlin.de"


def getPredictions(forecaster, classifier, queue, sc_f, sc_c):
    # print(queue)
    data = np.asarray(queue).reshape(5,4)
    data = -data
    # print(data)

    X2_data = sc_f.transform(data)
    expanded_preds = np.repeat(forecaster.predict(X2_data.reshape(-1,5,4)), 2, axis=1)
    predictions = sc_f.inverse_transform(expanded_preds)

    cell_pred = classifier.predict(sc_c.transform(predictions[:,:2]))

    return [predictions[:,0][0], predictions[:,1][0], cell_pred[0]]


parser = argparse.ArgumentParser(description='Start simulation script on/off')
parser.add_argument('--start',
                    type=int,
                    default=1,
                    help='Start ns-3 simulation script 0/1, Default: 1')
parser.add_argument('--iterations',
                    type=int,
                    default=1,
                    help='Number of iterations, Default: 1')
args = parser.parse_args()
startSim = bool(args.start)
iterationNum = int(args.iterations)

port = 5555
simTime = 1000 # seconds
stepTime = 0.5  # seconds
seed = 0
simArgs = {"--simTime": simTime,
           "--testArg": 123}
debug = False

env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
# simpler:
#env = ns3env.Ns3Env()
env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

stepIdx = 0
currIt = 0

forecaster = load_model("/mnt/c/Users/shard/Documents/workspace_5G/ML/model2.h5")
classifier = pickle.load(open('/mnt/c/Users/shard/Documents/workspace_5G/ML/classifier_2.sav', 'rb'))
sc_f = pickle.load(open('/mnt/c/Users/shard/Documents/workspace_5G/ML/scaler.pkl','rb'))
sc_c = pickle.load(open('/mnt/c/Users/shard/Documents/workspace_5G/ML/scaler_classify2.pkl','rb'))

queue = deque()

try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset() #get observation
        print("Step: ", stepIdx)
        print("---obs:", obs)

        while True:
            stepIdx += 1
            # action = env.action_space.sample() #select action
            
            if stepIdx > 6:
                queue.popleft()
            
            if stepIdx > 1:
                queue.append(obs)

            # print("---past 5:",queue)

            if stepIdx > 5:   
                action = getPredictions(forecaster, classifier, queue, sc_f, sc_c)
                # print(len(queue))
            else:
                action = [-1, -1, -1] #no prediction yet

            print("---action: ", action)

            print("Step: ", stepIdx)
            obs, reward, done, info = env.step(action) #do action, collect observation
            print("---obs, reward, done, info: ", obs, reward, done, info)
            # print(obs[0])
            

            

            if done:
                stepIdx = 0
                if currIt + 1 < iterationNum:
                    env.reset()
                break

        currIt += 1
        if currIt == iterationNum:
            break

except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")