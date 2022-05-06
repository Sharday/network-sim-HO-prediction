# Bachelor's Thesis Research Project - applying machine learning for mobility prediction in 5G network scenarios within a simulation environment


This project involves the application of machine learning in 5G cellular network research for handover prediction.

The San Francisco taxis mobility traceset was used for this project, which can be found at: https://crawdad.org/epfl/mobility/20090224/.

## Network Simulation

The first stage involved ns-3 network simulations, using the mobility traces to drive UE movements. 5G gNB handover is not supported at the time of writing, so simulations were run on the basis of an LTE network, as a 5G Non-Standalone Architecture. Release 3.35 of ns-3 was used for this project. Please see instructions for downloading and installing ns-3 at https://www.nsnam.org/docs/release/3.29/tutorial/html/getting-started.html.

The mobility traces first need to be processed via _preprocess_for_simulation.py_, to convert them to a format suitable for simulation. Please execute this via:

```Python
python preprocess_for_simulation.py
```

Simulations can then be run in ns-3 using _taxi_sim_2_cells.cc_ or _taxi_sim_4_cells.cc_, which correspond to 2-cell and 4-cell network scenarios.

For example, to run the 2-cell scenario simulation, please execute:

```Python
./waf --run scratch/taxi_sim_2_cells
```

This can be repeated using as many pre-processed mobility files as desired. Each simulation will produce an output trace text file of simulation time, position, RSRP of each cell, RSRQ of each cell and S-PCI (serving cell).

As NetAnim (the standard ns-3 visualisation tool) does not fully support animation of this type of network scenario, a Python script has been written for visualisation of the simulations, which can be used for debugging. Please execute this on the simulation output via:

```Python
python network_animation.py
```
## Machine Learning

This stage involves the training and testing of machine learning models for handover prediction, using the simulation output. A machine learning ensemble of LSTM and XGBoost is used. The LSTM (Long-Short Term Memory) network is used for multivariate time series forecasting of RSRP values, based on a recent history of RSRP and RSRQ values. Then XGBoost (eXtreme Gradient Boosting) is used to classify the serving cell based on the forecasted RSRP values. The ADVANCE_STEPS variable can be adjusted according to the desired forecast length. The performance of the LSTM was evaluated using MAE, and the performance of XGBoost using F1-score, accuracy, ROC curves and AUC.

The processes for training, testing and evaluation are applied for the 2-cell and 4-cell scenarios in _handover_prediction_2_cells.ipynb_ and _handover_prediction_4_cells.ipynb_ respectively.

## ML-simulation integration

An interconnection of the machine learning framework with the ns-3 simulation environment was achieved using ns3-gym. NS3-gym is a toolkit developed by Piotr Gawlowicz and Anatolij Zubow for the training of reinforcement learning algorithms within ns-3 simulations. However, due to the flexibility of the framework interface, any machine learning model can be integrated into the simulation for inference purposes, to support a wide variety of applications. The API is intuitive and the ns3-gym middleware supports synchronous and asynchronous communication via ZMQ sockets.

Instructions for installation of ns3-gym and example programs can be found at: https://github.com/tkn-tub/ns3-gym.

To demonstrate the integration, the current use case of handover prediction is implemented using the interface (based on the "opengym" example of ns3-gym) as follows:

* The ns3-gym interface is initialised and callback functions implemented within the simulation script
* Data scalers and pre-trained LSTM and XGBoost models are loaded using the agent script
* The Python agent and ns-3 simulation run as separate processes, communicating synchronously every 0.5 seconds
* A queue structure is used to store and update the recent history of RSRP and RSRQ values within the agent
* At each timestep, the processes interact via the interface: the agent collects 'observations' of RSRP and RSRQ values from the simulation, predictions are produced by the ML ensemble, and these are fed back to the simulation as 'actions'

Please copy the ML-ns3 directory into the scratch folder of ns3-gym and rebuild. Please ensure that the required dependencies for loading and running your Python models have been installed. The simulation can be run by executing:

```Python
./waf --run "ML-ns3"
```

and the Python process can be run from the scratch directory using:

```Python
./agent.py --start=0
```

Machine learning has proven to be extremely useful in recent networking literature; this two-way system integration can facilitate its application in future cellular network research, by allowing training of reinforcement learning algorithms or using trained models for inference. For example, the current use case could be used to develop low latency 5G applications, which require timely handover predictions.
