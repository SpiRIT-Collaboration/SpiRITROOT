#!/usr/bin/python

import os
import sys
import datetime, time








# List of parameters
#   First letter of name to be written in capital is recommended.
#   double type variable should have decimal point.
#   [name], [value]
listPar = [
  ['PadPlaneX',                864.,       'mm', ''],
  ['PadPlaneZ',                1344.,      'mm', ''],
  ['PadSizeX',                 8.,         'mm', ''],
  ['PadSizeZ',                 12.,        'mm', ''],
  ['PadRows',                  108,        'mm', ''],
  ['PadLayers',                112,        'mm', ''],
  ['AnodeWirePlaneY',          -4.05,      'mm', ''],
  ['GroundWirePlaneY',         -8.1,       'mm', ''],
  ['GatingWirePlaneY',         -14.,       'mm', ''],
  ['EField',                   0.,         '', ''],
  ['NumTbs',                   512,        'The number of time buckets in the actual data', 'returns the number of time buckets that the data actually has'],
  ['WindowNumTbs',             512,        'The number of time buckets of the window', 'returns the number of time buckets of the time window'],
  ['WindowStartTb',            0,          'First time bucket number of the window', 'returns the time bucket number of the starting point of time window'],
  ['SamplingRate',             25,         'MHz', ''],
  ['DriftLength',              496.,       'mm', 'returns the drift length in mm'],
  ['YDivider',                 496,        'Full drift length will be divided by this number when finding cluster', 'returns the slice divider'],
  ['EIonize',                  0.0000262,  'MeV', ''],
  ['DriftVelocity',            5.43,       'cm/us or 1.E-3*cm/us', 'returns the drift velocity in cm/us.'],
  ['CoefDiffusionLong',        0.034,      'cm^{-1/2}', ''],
  ['CoefDiffusionTrans',       0.024,      'cm^{-1/2}', ''],
  ['CoefAttachment',           0.2,        'cm^{-1/2}', ''],
  ['Gain',                     1270,       'multiplicity', ''],
]
nParameters = len(listPar)

# List of file names
#   [name], [file name]
listFilePar = [
  ['UAMapFile',   'parameters/UnitAsAd.map'],
  ['AGETMapFile', 'parameters/AGET.map'],
]
nFileParameters = len(listFilePar)








nameSourceTemp = "ParTemp.cc"
nameHeaderTemp = "ParTemp.hh"

SpiRIRTWorkDir = os.environ["VMCWORKDIR"]

namePar     = SpiRIRTWorkDir + "/parameters/ST.parameters.par"
nameFilePar = SpiRIRTWorkDir + "/parameters/ST.files.par"
nameSource  = SpiRIRTWorkDir + "/par/STDigiPar.cc"
nameHeader  = SpiRIRTWorkDir + "/par/STDigiPar.hh"

#namePar     = "ST.parameters.par"
#nameFilePar = "ST.files.par"
#nameSource  = "STDigiPar.cc"
#nameHeader  = "STDigiPar.hh"


print "========================================================================"
print ""
print "List of parameters :"
for iPar in range(nParameters):
  print "   " + str(listPar[iPar][0]) + " = " + str(listPar[iPar][1])
print ""
print "List of file parameters :"
for iPar in range(nFileParameters):
  print "   " + str(listFilePar[iPar][0]) + " = " + str(listFilePar[iPar][1])
print ""
print "You can edit parameters inside the script."
print ""
print "This macro will create (or replace) following files :"
print "   " + namePar
print "   " + nameFilePar
print "   " + nameSource
print "   " + nameHeader
print ""
print "========================================================================"
print ""
keyboardInput = raw_input('DO YOU WANT TO CONTINUE? (Y/N) : ')
if(keyboardInput is not 'Y') : 
  print "Ending program."
  sys.exit()

print "Running..."










## create >>> ST.parameters.par 

filePar = open(namePar,"w")
dateStamp = datetime.date.today()
filePar.write(
"""################################################################################
# Parameters used throughout SPiRITROOT software
# Format:
# parameter_name:parameter_type(Int_t-integer, Double_t-double) parameter_value
# STDigiPar
# Description of parameters
################################################################################ 
[STDigiPar]
"""
)

lenMaxName = 25
lenMaxValue = 15

for iPar in range(nParameters):
  name = listPar[iPar][0]
  value = listPar[iPar][1]
  comment = listPar[iPar][2]
  strValue = str(value)

  lenName   = len(name)
  lenSpace  = lenMaxName - lenName
  lenValue  = len(strValue)
  lenSpace2 = lenMaxValue - lenValue

  if type(value) is int :
    filePar.write(name + ":Int_t ")
    for space in range(lenSpace) : filePar.write(" ")
    filePar.write(strValue)
    for space in range(lenSpace2) : filePar.write(" ")
    filePar.write("   # "+comment+"\n")
  elif type(value) is float :
    filePar.write(name + ":Double_t ")
    for space in range(lenSpace-3) : filePar.write(" ")
    filePar.write(strValue)
    for space in range(lenSpace2) : filePar.write(" ")
    filePar.write("   # "+comment+"\n")
  else :
    print "  *** ERROR *** The value is neiter integer nor float : ", listPar[iPar]
    continue

for iPar in range(nFileParameters):
  name = listFilePar[iPar][0]
  lenName = len(name)
  lenSpace = lenMaxName - lenName

  filePar.write(name + ":Int_t ")
  for space in range(lenSpace) : filePar.write(" ")
  filePar.write(str(iPar)+"\n")

filePar.write("#########################################################################")
filePar.close()











## create >>> ST.files.par 

fileFilePar = open(nameFilePar,"w")
for iPar in range(nFileParameters):
  name = listFilePar[iPar][1]
  fileFilePar.write(name+"\n")
fileFilePar.close()








## create >>> STDigiPar.hh

headerTemp = open(nameHeaderTemp,"r")
headerFile = open(nameHeader,"w")

headerFile.write(
"/**\n"
"* @brief Store parameters from ST.parameters.par for later use.\n"
"*/\n"
"\n"
)

while 1 :
  line = headerTemp.readline()
  if not line : break

  if line.find('USER-PUBLIC') is not -1 :
    for iPar in range(nParameters):
      name  = listPar[iPar][0]
      value = listPar[iPar][1]
      comment = listPar[iPar][3]
      if   type(value) is int   : 
        if comment != '' : headerFile.write("    /// " + comment + "\n")
        headerFile.write("       Int_t Get"+name+"();\n")
      elif type(value) is float : 
        if comment != '' : headerFile.write("    /// " + comment + "\n")
        headerFile.write("    Double_t Get"+name+"();\n")
      else : continue
    headerFile.write("\n")
    for iPar in range(nFileParameters):
      name = listFilePar[iPar][0]
      headerFile.write("     TString Get"+name+"Name();\n")

  elif line.find('USER-PRIVATE') is not -1 :
    for iPar in range(nParameters):
      name = listPar[iPar][0]
      value = listPar[iPar][1]
      if   type(value) is int   : headerFile.write("       Int_t f"+name+";\n")
      elif type(value) is float : headerFile.write("    Double_t f"+name+";\n")
      else : continue
    headerFile.write("\n")
    for iPar in range(nFileParameters):
      name = listFilePar[iPar][0]
      headerFile.write("    TString  f"+name+"Name;\n")
      headerFile.write("      Int_t  f"+name+";\n")

  else : headerFile.write(line)

headerFile.close()









## create >>> STDigiPar.cc

sourceTemp = open(nameSourceTemp,"r")
sourceFile = open(nameSource,"w")

sourceFile.write(
"/**\n"
"* @brief Store parameters from ST.parameters.par for later use.\n"
"*/\n"
"\n"
)

while 1 :
  line = sourceTemp.readline()
  if not line : break

  if line.find('USER-GETTER') is not -1 :
    for iPar in range(nParameters):
      name  = listPar[iPar][0]
      value = listPar[iPar][1]
      lenName = len(name)
      lenSpace = lenMaxName - lenName
      if   type(value) is int   : 
        sourceFile.write("Int_t    STDigiPar::Get"+name+"()")
        for space in range(lenSpace) : sourceFile.write(" ")
        sourceFile.write(" { return f"+name+"; }\n")
      elif type(value) is float :
        sourceFile.write("Double_t STDigiPar::Get"+name+"()")
        for space in range(lenSpace) : sourceFile.write(" ")
        sourceFile.write(" { return f"+name+"; }\n")
      else : continue
    for iPar in range(nFileParameters):
      name = listFilePar[iPar][0]
      lenName = len(name)
      lenSpace = lenMaxName - lenName
      sourceFile.write("TString  STDigiPar::Get"+name+"Name()")
      for space in range(lenSpace-4) : sourceFile.write(" ")
      sourceFile.write(" { return f"+name+"Name; }\n")
    sourceFile.write("\n")

  elif line.find('USER-GETPARAMS') is not -1 :
    for iPar in range(nParameters):
      name  = listPar[iPar][0]
      sourceFile.write(
      '    if (!(paramList -> fill("'+name+'", &f'+name+'))) {\n'
      '      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find '+name+' parameter!");\n'
      '      return kFALSE;\n'
      '    }\n'
      )
    for iPar in range(nFileParameters):
      name = listFilePar[iPar][0]
      sourceFile.write(
      '    if (!(paramList -> fill("'+name+'", &f'+name+'))) {\n'
      '      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find '+name+' parameter!");\n'
      '      return kFALSE;\n'
      '    }\n'
      '    f'+name+'Name = GetFile(f'+name+');\n'
      )
    sourceFile.write("\n")

  elif line.find('USER-PUTPARAMS') is not -1 :
    for iPar in range(nParameters):
      name  = listPar[iPar][0]
      sourceFile.write('  paramList -> add("'+name+'", f'+name+');\n')
    for iPar in range(nFileParameters):
      name  = listFilePar[iPar][0]
      sourceFile.write('  paramList -> add("'+name+'", f'+name+');\n')

  else : sourceFile.write(line)

sourceFile.close()








print "Complete"
