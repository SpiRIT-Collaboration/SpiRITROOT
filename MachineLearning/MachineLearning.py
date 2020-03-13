import argparse
import numpy as np
import tensorflow as tf
from tensorflow import keras
from keras.callbacks import EarlyStopping
from keras.models import Sequential
from keras.layers import Dense
import pandas as pd
from keras.models import load_model

def TrainMachine(labeledData, modelSave, nClus):

  #ALL THINGS NEEDED TO ADD NEW PARTICLES ARE LABELLED 'HERE'

  
  #READ DATASET
  dataset = pd.read_table(str(labeledData), header=0, delimiter=',')
  dataset.loc[dataset['Type'] == 'Pion', 'Type'] = 0.
  dataset.loc[dataset['Type'] == 'Proton', 'Type'] = 1.
  dataset.loc[dataset['Type'] == 'Deuteron', 'Type'] = 2.
  dataset.loc[dataset['Type'] == 'Triton', 'Type'] = 3.
  dataset.loc[dataset['Type'] == 'He3', 'Type'] = 4.
  dataset.loc[dataset['Type'] == 'He4', 'Type'] = 5.

  if nClus is not None:
    nClus = int(nClus)
    dataset = dataset[dataset['NClus'] > nClus]

  #HERE
  #dataset.loc[dataset['Type'] == 'He6', 'Type'] = 6.
  data = dataset.values
  np.random.shuffle(data)
  #REMOVE FARAWAY POINTS THAT WOULD MESS WITH ML
  data = data[data[:,0]>0]
  data = data[(data[:,1]**2+data[:,2]**2+data[:,3]**2)**.5<4600]
  data = data[data[:,0]<1600]
  #CHOOSE INPUT DATA dEdx and Px, Py, Pz
  x = data[:,[0,1,2,3]]
  
  #MAKE SOFTMAX OUTPUS OUT OF NAMES OF PARTICLES
  y = []
  for i in data:
      #HERE
      output = [0,0,0,0,0,0]
      output[int(i[6])] = 1
      y.append(output)
  y = np.array(y)
  
  
  #MACHINE LEARN
  batch_size = 100
  epochs = 100
  
  model = Sequential()

  model.add(Dense(7, input_dim = 4, kernel_initializer='normal', activation='relu'))
  model.add(Dense(30, kernel_initializer='normal', activation='relu'))
  model.add(Dense(30, kernel_initializer='normal', activation='relu'))
  model.add(Dense(30, kernel_initializer='normal', activation='relu'))
  model.add(Dense(30, kernel_initializer='normal', activation='relu'))
  #HERE          \/Number of Particles
  model.add(Dense(6, kernel_initializer='normal', activation='softmax'))

  model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['acc','mae'])

  es = EarlyStopping(monitor='val_loss', mode='min', verbose=0, patience=200)
  model.fit(x[0:300000], y[0:300000], epochs=epochs, batch_size=batch_size, validation_split=.3, verbose=1)
  
 
  #SAVE
  model.save(str(modelSave))
  



def ClassifyParticle(input, output, modelSave):
  model = load_model(str(modelSave))
  
  dataset = pd.read_csv(str(input), header=0, sep='\t')
  eventid = dataset['eventid']
  data = dataset.values
  
  predictions = model.predict(data[:,:4])
  pred = np.array(tf.argmax(predictions,1))
  
  predDf = pd.DataFrame(pred, columns=['ptype'])
  predDf['eventid'] = eventid
  dataset = predDf[['eventid','ptype']]
  
  dataset.loc[dataset['ptype'] == 0, 'ptype'] = 211
  dataset.loc[dataset['ptype'] == 1, 'ptype'] = 2212
  dataset.loc[dataset['ptype'] == 2, 'ptype'] = 1000010020
  dataset.loc[dataset['ptype'] == 3, 'ptype'] = 1000010030
  dataset.loc[dataset['ptype'] == 4, 'ptype'] = 1000020030
  dataset.loc[dataset['ptype'] == 5, 'ptype'] = 1000020040
  dataset.loc[dataset['ptype'] == 6, 'ptype'] = 1000020060
  
  dataset.to_csv(str(output), index = False, header=True, sep= '\t')




if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Classification of particles with Machine learning algorithm')
  parser.add_argument('-i', help='Input dEdX and momentum file', required=False)
  parser.add_argument('-o', help='Name of the output to which classified PID is stored', required=False)
  parser.add_argument('-l', help='labeled data used in training the machine', required=False)
  parser.add_argument('-m', help='Name of the model file', required=True)
  parser.add_argument('-n', help='Number of clusters cut on trainning files', required=False)
  
  args = vars(parser.parse_args())
  if args['i'] is not None and args['o'] is not None:
    ClassifyParticle(args['i'], args['o'], args['m'])
  elif args['l'] is not None:
    TrainMachine(args['l'], args['m'], args['n'])
  else:
    raise Exception('Invide arguments provided')
