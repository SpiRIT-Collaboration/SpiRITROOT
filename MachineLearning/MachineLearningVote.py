import argparse
import numpy as np
import pandas as pd
from sklearn.pipeline import Pipeline
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import cross_val_score, train_test_split
from sklearn.externals import joblib
from pickle import dump, load
from sklearn.neural_network import MLPClassifier
from sklearn.ensemble import VotingClassifier
from sklearn.experimental import enable_hist_gradient_boosting
from sklearn.ensemble import HistGradientBoostingClassifier
from sklearn.ensemble import RandomForestClassifier


def TrainMachine(labeledData, modelSave):

  #ALL THINGS NEEDED TO ADD NEW PARTICLES ARE LABELLED 'HERE'

  
  #READ DATASET
  trains = pd.read_table(str(labeledData), delimiter=',')
  
  trains = trains[(trains['Px']**2+trains['Py']**2+trains['Pz']**2)<4600**2]
  trains = trains[(trains['dEdX'])<1600]
  trains = trains[(trains['dEdX'])>0]
  train = trains

  label = train['Type']

  train = train[['dEdX','Px','Py','Pz']]
  
  
  #MACHINE LEARN

  #VotingModel
  clf1 = MLPClassifier(hidden_layer_sizes=(30,30,30,30,30), max_iter=500, batch_size=100)
  clf2 = HistGradientBoostingClassifier()
  clf3 = RandomForestClassifier()


  modelVote = VotingClassifier(estimators=[('NN', clf1), ('GB', clf2), ('RF', clf3)], voting='soft')
  modelVote.fit(train,label)
  
  dump(modelVote, open(str(modelSave), 'wb'))
  



def ClassifyParticle(input, output, modelSave):
  model = load(open(str(modelSave), 'rb'))
  
  dataset = pd.read_table(str(input), header=0, delim_whitespace=True)
  eventid = dataset['eventid']
  data = dataset.values
  
  pred = model.predict(data[:, :4])
  
  predDf = pd.DataFrame(pred, columns=['ptype'])
  predDf['eventid'] = eventid
  dataset = predDf[['eventid','ptype']]
  
  dataset.loc[dataset['ptype'] == 'Pion', 'ptype'] = 211
  dataset.loc[dataset['ptype'] == 'Proton', 'ptype'] = 2212
  dataset.loc[dataset['ptype'] == 'Deuteron', 'ptype'] = 1000010020
  dataset.loc[dataset['ptype'] == 'Triton', 'ptype'] = 1000010030
  dataset.loc[dataset['ptype'] == 'He3', 'ptype'] = 1000020030
  dataset.loc[dataset['ptype'] == 'He4', 'ptype'] = 1000020040
  #dataset.loc[dataset['ptype'] == 6, 'ptype'] = 1000020060

  dataset.to_csv(str(output), index = False, header=True, sep= '\t')




if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Classification of particles with Machine learning algorithm')
  parser.add_argument('-i', help='Input dEdX and momentum file', required=False)
  parser.add_argument('-o', help='Name of the output to which classified PID is stored', required=False)
  parser.add_argument('-l', help='labeled data used in training the machine', required=False)
  parser.add_argument('-m', help='Name of the model file', required=True)

  
  args = vars(parser.parse_args())
  if args['i'] is not None and args['o'] is not None:
    ClassifyParticle(args['i'], args['o'], args['m'])
  elif args['l'] is not None:
    TrainMachine(args['l'], args['m'])
  else:
    raise Exception('Invide arguments provided')
