import os
import sys

if len(sys.argv)==1 :
  name = "noname"
  mode = "full"

else :
  name  = sys.argv[1]
  mode = sys.argv[2]





modePossibleList = ['mc','digi','reco','eve','full','dummy']
runFlag = False

modeList = []
if mode.find("+") != -1 : 
  modeList = mode.split("+")
  mode = mode.replace("+"," + ")
if not modeList : modeList = [mode]

for imode in modeList :
  checkFlag = False
  for modePossible in modePossibleList : 
    if(imode == modePossible) : checkFlag = True
  if checkFlag == False : 
    runFlag = False
    break
  runFlag = True





print
print "======== SPiRIT Simulation Macro ========"
print 
print "Tag name :", name
print "Run mode :", mode
print

if not runFlag :
  print "Run mode must be one of :"
  for modePossible in modePossibleList : print " -" , modePossible 
  print "or combination of these using \"+\" sign without black space." 
  print "Combination should not jump over the order of simulation steps."
  print 'Meaning "mc+digi+reco" is good, "mc+reco" is not.'
  print 
  print "Ending program."
  sys.exit()




dirSPiRIT = os.environ["VMCWORKDIR"]
dirGeom   = dirSPiRIT + "/geometry/"
dirConf   = dirSPiRIT + "/gconfig/"
os.environ["GEOMPATH"] = dirGeom
os.environ["CONFIG_DIR"] = dirConf

pathGeomSPiRIT      = dirGeom + "geomSPiRIT.root"
pathGeomSPiRITMacro = dirGeom + "geomSPiRIT.C"
existGeom = os.path.exists(pathGeomSPiRIT)
print "Checking SPiRIT geometry file ..." , existGeom
if not existGeom : 
  print "  Creating geometry file ..."
  print "  Running macro file :" , pathGeomSPiRITMacro
  print
  os.system("root -q -l " + pathGeomSPiRITMacro)
  print

existData = os.path.exists("data")
print "Checking data directory ..." , existData
if not existData : 
  print "  Creating data directory ..."
  print
  os.system("mkdir data")
  print





print 
for imode in modeList :
  if imode == "full" :
    os.system("root -l -q 'run_mc.C(\""+name+"\")'")
    os.system("root -l -q 'run_digi.C(\""+name+"\"'")
    os.system("root -l -q 'run_reco_mc.C(\""+name+"\")'")
    os.system("root -l    'run_eve.C(\""+name+"\")'")
  if imode == "mc"   : os.system("root -l -q 'run_mc.C(\""+name+"\")'")
  if imode == "digi" : os.system("root -l -q 'run_digi.C(\""+name+"\")'")
  if imode == "reco" : os.system("root -l -q 'run_reco_mc.C(\""+name+"\")'")
  if imode == "eve"  : os.system("root -l    'run_eve.C(\""+name+"\")'")
  if imode == "dummy" : print "dummy run."

print
print "Ending program."
