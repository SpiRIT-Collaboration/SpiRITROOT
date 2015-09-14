#!/usr/bin/env python

import os
import sys
import subprocess as sub
import urllib2
import time



class bcolors:
  GREEN  = '\033[92m'
  YELLOW = '\033[93m'
  RED    = '\033[91m'
  ENDC   = '\033[0m'

print 
print "  ===================================="
print "        SpiRITROOT Configuration      "
print "  ===================================="
print 
#print "  < Check List >"
#print """  1. FairSoft environment settings and version
#  2. FairROOT environment settings and version
#  3. SpiRITROOT environment setting and version.
#  4. Geometry.
#  5. Link of mapping files.
#  6. Directories."""
#print 
#print "  < Test List >"
#print """  7. Reconstruction of MC.
#  8. Reconstruction of cosmic data.
#  9. Speed issue.  
#  """
#print

checkList = []
testList  = []

pop = os.popen('pwd',"r")
pwd = pop.readline()
pwd = pwd.replace('\n','')



# ------------------------------------------------
# 1. FairSoft environment settings and version

print " - Checking FairSoft...    ",
checkList.append("SIMPATH" in os.environ)
if checkList[0] == False: 
  print
  print bcolors.RED + "  [ERROR]" + bcolors.ENDC + """ FairSoft not found!
          Please set environments for FairSoft."
          See https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/How-to-install-packages for detail.
  """ 
  sys.exit()
pop = os.popen('fairsoft-config --version',"r")
FairSoft_version = pop.readline()
FairSoft_version = FairSoft_version.replace('\n','')
print "your FairSoft version is", bcolors.GREEN + FairSoft_version + bcolors.ENDC



# ------------------------------------------------
# 2. FairROOT environment settings and version

print " - Checking FairROOT...    ",
checkList.append("FAIRROOTPATH" in os.environ)
if checkList[1] == False: 
  print
  print bcolors.RED + "  [ERROR]" + bcolors.ENDC + """ FairROOT not found!
          Please set environments for FairROOT.
          See "https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/How-to-install-packages" for detail.
  """
  sys.exit()
pop = os.popen('$FAIRROOTPATH/bin/fairroot-config --version',"r")
FairROOT_version = pop.readline()
FairROOT_version = FairROOT_version.replace('\n','')
print "your FairROOT version is", bcolors.GREEN + FairROOT_version + bcolors.ENDC



# ------------------------------------------------
# 3. SpiRITROOT environment setting and version.

print " - Checking SpiRITROOT...  ",
checkList.append("VMCWORKDIR" in os.environ)
if checkList[2] == False: 
  print
  print bcolors.RED + "  [ERROR]" + bcolors.ENDC + " SpiRITROOT not found!"
  print """          Please build SpiRITROOT.
          See "https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/How-to-install-packages" for detail.
  """
  sys.exit()
print "your SpiRITROOT version is", bcolors.GREEN + "-" + bcolors.ENDC #SpiRITROOT_version



# ------------------------------------------------
# 4. Geometry

print " - Checking Geometry...    ",
dirWork = os.environ["VMCWORKDIR"]
dirGeom = dirWork + "/geometry/"
pathGeomTop = dirGeom + "geomSpiRIT.root"
pathGeomMgr = dirGeom + "geomSpiRIT.man.root"
pathGeomMcr = dirGeom + "geomSpiRIT.C"
listTop = pathGeomTop.split('/')
listMgr = pathGeomMgr.split('/')
if (os.path.exists(pathGeomTop) == False) or (os.path.exists(pathGeomMgr) == False) :
  checkList.append(False)
else :
  checkList.append(True)
if checkList[3] == False: 
  print
  print bcolors.YELLOW + "   [INFO]" + bcolors.ENDC + " Geometry not found!"
  print "            Creating geometry file... running macro" , pathGeomMcr
  os.system("root -q -l " + pathGeomMcr)
  print
else:
  print "Top     :", bcolors.GREEN + listTop[len(listTop)-1] + bcolors.ENDC
  print "                           ", 
  print "Manager :", bcolors.GREEN + listMgr[len(listMgr)-1] + bcolors.ENDC


  
# ------------------------------------------------
# 5. Mapping Files

print " - Checking Map...         ",
checkList.append(True)
pop = os.popen('readlink -f $VMCWORKDIR/parameters/AGET.map',"r")
AGET_map = pop.readline()
AGET_map = AGET_map.replace('\n','')
listAGET = AGET_map.split('/')
print "AGET.map     ->", bcolors.GREEN + listAGET[len(listAGET)-1] + bcolors.ENDC
pop = os.popen('readlink -f $VMCWORKDIR/parameters/UnitAsAd.map',"r")
UnitAsAd_map = pop.readline()
UnitAsAd_map = UnitAsAd_map.replace('\n','')
listUnitAsAd = UnitAsAd_map.split('/')
print "                           ", 
print "UnitAsAd.map ->", bcolors.GREEN + listUnitAsAd[len(listUnitAsAd)-1] + bcolors.ENDC



# ------------------------------------------------
# 6. Directories

print " - Checking Directories... ",
checkList.append(os.path.exists("data"))
if checkList[5] == False : 
  print
  print bcolors.YELLOW + "   [INFO]" + bcolors.ENDC + " data directory not found!",
  print "Creating data directory..."
  os.system("mkdir data")
else: 
  print bcolors.GREEN + "data" + bcolors.ENDC
print 
print 



# ------------------------------------------------
# 7. Cosmic

print " - Cosmics Reconstruction Test"
print "  ",
print bcolors.YELLOW + "[Q]" + bcolors.ENDC,
print "Run cosmics reconstruction test? (y/n):",
kbInput = raw_input('')
if(kbInput == 'y' or kbInput == 'Y') : 
  nameCosmicShort = 'run_0457_event4n10.dat'
  pathCosmicShort = dirWork + '/input/' + nameCosmicShort
  urlCosmicShort = 'http://ribf.riken.jp/~genie/box/run_0457_event4n10.dat'
  existCosmicShort = os.path.exists(pathCosmicShort)

  if (existCosmicShort == False) :
    print bcolors.YELLOW + "   [INFO] " + bcolors.ENDC + nameCosmicShort + " not found in input directory!"
  else :
    file = open(pathCosmicShort, 'r')
    file_size = int(os.path.getsize("../input/run_0457_event4n10.dat"))
    if (file_size == 0) :
      print bcolors.YELLOW + "   [INFO] " + bcolors.ENDC + nameCosmicShort + " is empty!"
      existCosmicShort = False

  if (existCosmicShort == False) :
    url = urllib2.urlopen(urlCosmicShort)
    file = open(pathCosmicShort, 'wb')
    meta = url.info()
    file_size = int(meta.getheaders("Content-Length")[0])
    print "          " + "Downloading " + nameCosmicShort + " (" + str(file_size) + " Bytes) from " + urlCosmicShort
    file.write(url.read())
    print "          Complete."
  sim_name = '"cosmic_short"'
  event_name = '"%s"' % pathCosmicShort
  command_reco = "root -l -q -b 'run_reco.C(" + sim_name + "," + event_name + ")'"

  time_start = time.time()
  os.system(command_reco)
  time_end = time.time()
  time_diff = time_end - time_start

  print
  print "   " + bcolors.GREEN + "[Summary]" + bcolors.ENDC + " Cosmics reconstruction test"
  print "   - Number of events : 2"
  print "   - Reconstruction time : %.2f seconds  (%.2f seconds per event)" % (time_diff, time_diff / 2.)

print



# ------------------------------------------------
# 8. MC

print " - MC Reconstruction Test"
print "  ",
print bcolors.YELLOW + "[Q]" + bcolors.ENDC,
print "Run MC reconstruction test? (y/n):",
kbInput = raw_input('')
if(kbInput == 'y' or kbInput == 'Y') : 

  sim_name = '"urqmd_short"'
  gen_name = '"UrQMD_300AMeV_short.egen"'
  use_map  = 'kFALSE'

  command_mc   = "root -l -q -b 'run_mc.C("   + sim_name + ',' + gen_name + ',' + use_map + ")'"
  command_digi = "root -l -q -b 'run_digi.C(" + sim_name + ")'"
  command_reco = "root -l -q -b 'run_reco.C(" + sim_name + ")'"

  time_start = time.time()
  os.system(command_mc)
  time_end = time.time()
  time_diff_mc = time_end - time_start

  time_start = time.time()
  os.system(command_digi)
  time_end = time.time()
  time_diff_digi = time_end - time_start

  time_start = time.time()
  os.system(command_reco)
  time_end = time.time()
  time_diff_reco = time_end - time_start

  print 
  print "   " + bcolors.GREEN + "[Summary]" + bcolors.ENDC + " MC reconstruction test"
  print "   - Number of events : 10"
  print "   - Geant4 simulation time : %.2f seconds  (%.2f seconds per event)" % (time_diff_mc,   time_diff_digi / 10.)
  print "   - Digitization      time : %.2f seconds  (%.2f seconds per event)" % (time_diff_digi, time_diff_digi / 10.)
  print "   - Reconstruction    time : %.2f seconds  (%.2f seconds per event)" % (time_diff_reco, time_diff_digi / 10.)

print
print "  Ending configuration."
print
