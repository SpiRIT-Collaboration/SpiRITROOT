import sys
import os
import numpy as np
import pandas as pd
import warnings
import lightgbm as lgb
from sklearn.model_selection import train_test_split, StratifiedKFold, cross_val_score
from sklearn.metrics import roc_auc_score, auc, accuracy_score, log_loss, f1_score, precision_score, recall_score, accuracy_score
from sklearn.metrics import mean_squared_error
warnings.filterwarnings("ignore")

obsList = ['Mch','N','N(H-He)','ETL','ERAT','Npt','N(H-He)pt']
if len(sys.argv) == 4:
  obsList = sys.argv[3].split()
  print('Use custom observable list. The observables are:')
  print(obsList)
else:
  print('Use old definition of observables (i.e. proton in N(H-He)')
obsList = obsList + ['bim']

#Training data：free CS (HM) and in_medium CS (SM)
df_all = pd.read_table(sys.argv[1],sep='\t')
#df_train = df_all.sample(frac=0.95)[['Mch','N','N(H-He)','ETL','ERAT','Npt','N(H-He)pt','bim']]
#df_test = df_all.drop(df_train.index)[['Mch','N','N(H-He)','ETL','ERAT','Npt','N(H-He)pt']]
#df_all = df_all[df_all['bim'] < 3]
df_all['weight'] = 1
df_all['weight'].loc[df_all['bim'] < 1] = 3#2 - df_all['bim'].loc[df_all['bim'] < 1]

df_train = df_all[obsList]#.sample(frac=1)
df_test = df_all.sample(frac=0.01)[obsList]
df_train_weight = df_all['weight']

df_train['isTrain'] = 0

target= df_train['bim']
label = df_train['isTrain']
del df_train['bim']
del df_train['isTrain']
feats = [feat for feat in df_train.columns if feat not in ['bim','isTrain']]
print(f"feats:{feats}")


#LightGBM parameter settings
param = param = {'num_leaves': 20,
         'min_data': 1, 
         'min_data_in_bin': 1,
         'boost_from_average': False,
         'objective':'regression',
         'max_depth': -1,
         'learning_rate': 0.003,
         #"min_child_samples": 20,
         "boosting": "gbdt",
         "feature_fraction": 0.9,
         "bagging_freq": 1,
         "bagging_fraction": 0.9 ,
         "bagging_seed": 11,
         "metric": 'l1',
         "lambda_l1": 0.05,
         "verbosity": -1,
         "nthread": 20,
         "random_state": 2021}
folds = StratifiedKFold(n_splits=2, shuffle=True, random_state=209)
oof = np.zeros(len(df_train))
predictions = np.zeros(len(df_test))
feature_importance_df = pd.DataFrame()
clfs = []
for fold_, (trn_idx, val_idx) in enumerate(folds.split(df_train,label.values)):
    print("fold {}".format(fold_))
    trn_data = lgb.Dataset(df_train.iloc[trn_idx][df_train.columns], label=target.iloc[trn_idx], weight=df_train_weight.iloc[trn_idx])
    val_data = lgb.Dataset(df_train.iloc[val_idx][df_train.columns], label=target.iloc[val_idx], weight=df_train_weight.iloc[val_idx])
    num_round = 50000
    clf = lgb.train(param, trn_data, num_round, valid_sets = [trn_data, val_data], verbose_eval=100, early_stopping_rounds = 100)
    oof[val_idx] = clf.predict(df_train.iloc[val_idx][df_train.columns], num_iteration=clf.best_iteration)
    #predictions += clf.predict(df_test[df_train.columns], num_iteration=clf.best_iteration) / folds.n_splits
    clfs.append(clf)
#Test the testing data and output the predictions
print(mean_squared_error(oof, target))
print('over')

#save model
import joblib
joblib.dump([obsList[:-1], clfs], sys.argv[2])

