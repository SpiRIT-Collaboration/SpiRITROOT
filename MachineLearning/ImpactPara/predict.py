import sys
import joblib
import numpy as np

if __name__ == '__main__':
  clfs = joblib.load(sys.argv[1])
  print('start')
  while True:
      data = input()
      if data == 'end':
          break
      data = np.atleast_2d(np.array(data.split()).astype(np.float))
      predict = 0
      for clf in clfs:
        predict += clf.predict(data, num_iteration=clf.best_iteration)[0]
      print(predict/len(clfs))
      #print(clf.predict(data, num_iteration=clf.best_iteration)[0])
