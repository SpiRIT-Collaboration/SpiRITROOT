import argparse
import pandas as pd

def TrainMachine(labeledData, modelSave):
  print('Train machien from data ' + labeledData)
  pass

def ClassifyParticle(input, output, modelSave):
  print('Start to classify data')
  pass

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
