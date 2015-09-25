#!/usr/bin/env python


import os
import sys
import subprocess as sub
import urllib2
import time
import re


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


# ------------------------------------------------
#  FairSoft environment settings and version
# ------------------------------------------------
print " - Checking FairSoft       ",
if ("SIMPATH" in os.environ) == False: 
  print
  print bcolors.RED + "   [ERROR]" + bcolors.ENDC + """ FairSoft not found!
          Please set environments for FairSoft."
          See https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/How-to-install-packages for detail.
  """ 
  sys.exit()
pop = os.popen('fairsoft-config --version',"r")
FairSoft_version = pop.readline()
FairSoft_version = FairSoft_version.replace('\n','')
print "your FairSoft version is", bcolors.GREEN + FairSoft_version + bcolors.ENDC



# ------------------------------------------------
#  FairROOT environment settings and version
# ------------------------------------------------
print " - Checking FairROOT       ",
if ("FAIRROOTPATH" in os.environ) == False: 
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
#  SpiRITROOT environment setting and version.
# ------------------------------------------------
print " - Checking SpiRITROOT     ",
if ("VMCWORKDIR" in os.environ) == False: 
  print
  print bcolors.RED + "  [ERROR]" + bcolors.ENDC + " SpiRITROOT not found!"
  print """          Please build SpiRITROOT.
          See "https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/How-to-install-packages" for detail.
  """
  sys.exit()
print "your SpiRITROOT version is", bcolors.GREEN + "-" + bcolors.ENDC #SpiRITROOT_version



# ------------------------------------------------
#  Define functions for reading parameters
# ------------------------------------------------
dirWork = os.environ["VMCWORKDIR"]

def GetParameterValue(parameter_file, parameter_name) :
  path = dirWork + "/parameters/" + parameter_file
  file = open(path, 'r')

  for line in file :
    line = line.rstrip()
    if line[0] == '[' : continue
    if line[0] == '#' : continue

    token  = line.split('#')
    token0 = token[0]
    token1 = re.split('\s+', token0)
    token2 = token1[0].split(':')
    name   = token2[0]
    value  = token1[1]

    if name == parameter_name :
      file.close()
      return value

  file.close()
  return -1


def GetFileName(input) :
  path = dirWork + "/parameters/ST.files.par"
  file = open(path, 'r')

  count_line = 0
  for line in file :
    line = line.rstrip()
    if count_line == input :
      count_line = count_line + 1 
      file.close()
      return line
    count_line = count_line + 1 

  file.close()
  return -1


# ------------------------------------------------
#  Mapping Files
# ------------------------------------------------
print
#print " - Checking Map..."
exist_file = False
while exist_file == False :
  print "  ",
  print bcolors.YELLOW + "[Q]" + bcolors.ENDC,
  print "Please enter name of the parameter file without the path. ex) ST.parameters.par"
  print "      ", "The file should be inside the SpiRITROOT/parameters directory."
  print "      ", "For ST.parameter.par, just press [Enter] :",
  kbInput = raw_input('')
  if(kbInput == '') : 
    exist_file = True
    name_parfile = "ST.parameters.par"
  else :
    name_parfile      = kbInput
    name_parfile_full = dirWork + "/parameters/" + kbInput
    exist_file = os.path.exists(name_parfile_full)
    if exist_file == False :
      print "  ",
      print bcolors.RED + "   [ERROR]" + bcolors.ENDC + " File does not exist! Please enter again:"

UA_mapfile   = GetFileName(int(GetParameterValue(name_parfile, 'UAMapFile')))
AGET_mapfile = GetFileName(int(GetParameterValue(name_parfile, 'AGETMapFile')))

print
print " - Checking Map            ", "AGET.map     ->", bcolors.GREEN + UA_mapfile + bcolors.ENDC
print "                           ", "UnitAsAd.map ->", bcolors.GREEN + AGET_mapfile + bcolors.ENDC


# Below method reads soft link of the file
#if(kbInput == 'y' or kbInput == 'Y') : 
#pop = os.popen('readlink -f $VMCWORKDIR/parameters/AGET.map',"r")
#AGET_map = pop.readline()
#AGET_map = AGET_map.replace('\n','')
#listAGET = AGET_map.split('/')
#print "AGET.map     ->", bcolors.GREEN + listAGET[len(listAGET)-1] + bcolors.ENDC
#pop = os.popen('readlink -f $VMCWORKDIR/parameters/UnitAsAd.map',"r")
#UnitAsAd_map = pop.readline()
#UnitAsAd_map = UnitAsAd_map.replace('\n','')
#listUnitAsAd = UnitAsAd_map.split('/')
#print "                           ", 
#print "UnitAsAd.map ->", bcolors.GREEN + listUnitAsAd[len(listUnitAsAd)-1] + bcolors.ENDC



# ------------------------------------------------
#  Geometry
# ------------------------------------------------
print " - Checking Geometry       ",
dirGeom = dirWork + "/geometry/"
pathGeomTop = dirGeom + "geomSpiRIT.root"
pathGeomMgr = dirGeom + "geomSpiRIT.man.root"
pathGeomMcr = dirGeom + "geomSpiRIT.C"
listTop = pathGeomTop.split('/')
listMgr = pathGeomMgr.split('/')
if (os.path.exists(pathGeomTop) == False) or (os.path.exists(pathGeomMgr) == False) :
  geomFlag = False
else :
  geomFlag = True
if geomFlag == False: 
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
#  Directories
# ------------------------------------------------
print " - Checking Directories    ",
if os.path.exists("data") == False : 
  print
  print bcolors.YELLOW + "   [INFO]" + bcolors.ENDC + " data directory not found!",
  print "Creating data directory..."
  os.system("mkdir data")
else: 
  print bcolors.GREEN + "data" + bcolors.ENDC
print 
print 



# ------------------------------------------------
#  Cosmic reconstruction test
# ------------------------------------------------
print " - Cosmics Reconstruction Test"
print "  ",
print bcolors.YELLOW + "[Q]" + bcolors.ENDC,
print "Run cosmics reconstruction test? (~ 30 sec.) (y/n):",
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
    file.close()

  if (existCosmicShort == False) :
    url = urllib2.urlopen(urlCosmicShort)
    file = open(pathCosmicShort, 'wb')
    meta = url.info()
    file_size = int(meta.getheaders("Content-Length")[0])
    print "          " + "Downloading " + nameCosmicShort + " (" + str(file_size) + " Bytes) from " + urlCosmicShort
    file.write(url.read())
    file.close()
    print "          Complete."
  sim_name = '"cosmic_short"'
  event_name = '"%s"' % pathCosmicShort
  command_reco = "source " + dirWork + "/build/config.sh > /dev/null; root -l -q -b 'run_reco.C(" + sim_name + "," + event_name + ")'"

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
#  MC simulation test
# ------------------------------------------------
print " - MC Reconstruction Test"
print "  ",
print bcolors.YELLOW + "[Q]" + bcolors.ENDC,
print "Run MC reconstruction test? (~ 3 min.) (y/n):",
kbInput = raw_input('')
if(kbInput == 'y' or kbInput == 'Y') : 

  sim_name = '"urqmd_short"'
  gen_name = '"UrQMD_300AMeV_short.egen"'
  use_map  = 'kFALSE'

  command_mc   = "source " + dirWork + "/build/config.sh > /dev/null; root -l -q -b 'run_mc.C("   + sim_name + ',' + gen_name + ',' + use_map + ")'"
  command_digi = "source " + dirWork + "/build/config.sh > /dev/null; root -l -q -b 'run_digi.C(" + sim_name + ")'"
  command_reco = "source " + dirWork + "/build/config.sh > /dev/null; root -l -q -b 'run_reco.C(" + sim_name + ")'"

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
  print "   - Geant4 simulation time : %.2f seconds  (%.2f seconds per event)" % (time_diff_mc,   time_diff_mc   / 10.)
  print "   - Digitization      time : %.2f seconds  (%.2f seconds per event)" % (time_diff_digi, time_diff_digi / 10.)
  print "   - Reconstruction    time : %.2f seconds  (%.2f seconds per event)" % (time_diff_reco, time_diff_reco / 10.)



print
print "  Ending configuration."
print
