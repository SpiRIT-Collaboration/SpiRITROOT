import xml.etree.ElementTree as ET
import argparse
import os
import tempfile

parser = argparse.ArgumentParser(description='analysisConfig modifier')
parser.add_argument('input', help='XML file to be modified')
parser.add_argument('-o', help='Output filename')
parser.add_argument('--MMin', help='Minimum multiplicity')
parser.add_argument('--MMax', help='Maximum multiplicity')
parser.add_argument('--ERatMin', help='Minimum ERatcut')
parser.add_argument('--ERatMax', help='Maximum ERat cut')
parser.add_argument('--Dir', help='Directory to data source')
parser.add_argument('--AddDir', help='Append directory to data source', nargs='+')
parser.add_argument('--TgMass', help='Atomic mass of the target')
parser.add_argument('--PrMass', help='Projectile mass')
parser.add_argument('--Energy', help='Beam energy per nucleons')
parser.add_argument('--Unfold', help='Unfold file')
parser.add_argument('--Filename', help='Filename of the data source')
parser.add_argument('--exec', default=False, action='store_true', help='Execute the modified config')
parser.add_argument('--execarg', help='(Only used with exec) Arguments passed to run_analysis_xml.sh')
parser.add_argument('--Divide', default=False, action='store_true', help='Divide each event by half')
parser.add_argument('--Comp', help='Choose complementary tracks to the given file')
parser.add_argument('--PhiEff', help='Change phi efficiency file')
parser.add_argument('--BiasCor', help='Change phi bias correction file')
parser.add_argument('--UseMCRP', default=False, action='store_true', help='Rotate according to MC truth reaction plane instead of the inferred reaction plane')
parser.add_argument('--rmTask', help='Remove a task', nargs='+')
parser.add_argument('--MassCoef', help='Mass Coef')
parser.add_argument('--ChargeCoef', help='Charge Coef')
parser.add_argument('--ConstCoef', help='Const Coef')
parser.add_argument('--ParticleCoef', help='Particle Coef')
parser.add_argument('--RejectEmpty', default=False, action='store_true', help='Enable trigger rejection in MC')
parser.add_argument('--MinBias', default=False, action='store_true', help='Use minimum bias in MC')
parser.add_argument('--ObsFile', help='Output selected observables to text file', nargs='?', const='')
parser.add_argument('--TrueImpFile', help='Truth file from UrQMD such that the true impact parameter can be saved')
parser.add_argument('--ImpModel', help='Model file for impact parameter determination')
parser.add_argument('--PIDFit', help='PID fit to data')
parser.add_argument('--PIDPrior', help='PID prior to data')



all_defaults = vars(parser.parse_args(['input']))

def create_nested_node(root, path):
  nodes_to_be_created = []
  aval_root = None
  while aval_root is None:
    aval_root = root.find(path)
    if aval_root is None:
      split_string = path.rsplit('/', 1)
      nodes_to_be_created.append(split_string[1])
      path = split_string[0]
  for node in reversed(nodes_to_be_created):
    element = ET.SubElement(aval_root, node)
    aval_root = element
  return element

def main(input, values):

  # fill in default if it does not exist in values
  for key, value in values.items():
    all_defaults[key] = value
  values = all_defaults

  # read xml
  tree = ET.parse(input)
  root = tree.getroot()

  if values['UseMCRP']:
    values['UseMCRP'] = ''
  else:
    values['UseMCRP'] = None

  if values['RejectEmpty']:
    values['RejectEmpty'] = ''
  else:
    values['RejectEmpty'] = None
  if values['MinBias']:
    values['MinBias'] = ''
  else:
    values['MinBias'] = None



  parameter_table = {'MMin': 'TaskList/EventFilterTask/MultiplicityMin',
                     'MMax': 'TaskList/EventFilterTask/MultiplicityMax',
                     'ERatMin': 'TaskList/EventFilterTask/ERatMin',
                     'ERatMax': 'TaskList/EventFilterTask/ERatMax',
                     'Dir': 'IOInfo/DataDir',
                     'Unfold': 'TaskList/EfficiencyTask/UpdateUnfolding',
                     'TgMass': ['TaskList/TransformFrameTask/TargetMass', 'IOInfo/targetA'],
                     'Energy': 'IOInfo/beamEnergyPerA',
                     'PrMass': 'IOInfo/beamA',
                     'Filename': 'IOInfo/InputName',
                     'PhiEff': 'TaskList/PhiEfficiencyTask/PhiEff',
                     'UseMCRP': 'TaskList/ReactionPlaneTask/UseMCReactionPlane',
                     'MassCoef': 'TaskList/ReactionPlaneTask/MassCoef',
                     'ChargeCoef': 'TaskList/ReactionPlaneTask/ChargeCoef',
                     'ConstCoef': 'TaskList/ReactionPlaneTask/ConstCoef',
                     'BiasCor': 'TaskList/ReactionPlaneTask/CorrectBias',
                     'ParticleCoef': 'TaskList/ReactionPlaneTask/ParticleCoef',
                     'RejectEmpty': 'TaskList/EventFilterTask/RejectEmpty',
                     'MinBias': 'TaskList/EventFilterTask/MinBias',
                     'ObsFile': 'TaskList/ObsWriterTask/ObsFile',
                     'TrueImpFile': 'TaskList/ObsWriterTask/UrQMDTruthFile',
                     'PIDFit': 'TaskList/PIDProbTask/PIDFit',
                     'PIDPrior': 'TaskList/PIDProbTask/MetaFile',
                     'ImpModel': 'TaskList/ImpactParameterMLTask/ModelFile'}

  for key, paths in parameter_table.items():
    value = values[key]
    if type(paths) is not list:
      paths = [paths]
    for path in paths:
      if value is not None:
        ele = root.find(path)
        if ele is None:
          #print('Cannot find ' + path)
          create_nested_node(root, path).text = value
        else:
          ele.text = value

  if values['AddDir'] is not None:
    ele = root.find('IOInfo')
    for dir_ in values['AddDir']:
      ET.SubElement(ele, 'DataDir').text = dir_

  tasks = root.find('TaskList')
  if values['Divide'] is True:
    child = ET.SubElement(tasks, 'DivideEventTask')
    child.text = ''
    if values['Comp'] is not None:
      # we will reject EventFilterTask if Comp file is set
      # since we accept all events in the comp file, regardless of EventFilterTask
      print('Since comp file is set, we will remove EventFilterTask')
      if values['rmTask'] is None:
        values['rmTask'] = ['EventFilterTask']
      else:
        values['rmTask'] = values['rmTask'] + ['EventFilterTask']
      comp = ET.SubElement(child, 'ComplementaryTo')
      comp.text = values['Comp']

  if values['rmTask'] is not None:
    for task_name in values['rmTask']:
      print('Removing task %s' % task_name)
      task = tasks.find(task_name)
      if task is None:
        print('Cannot find the above task. Will not be removed')
      else:
        tasks.remove(task)

  if values['exec']:
    with tempfile.NamedTemporaryFile() as fo:
      tree.write(fo.name)
      fo.flush()
      args = values['execarg']
      if args is None:
        args = ''
      os.system('./run_analysis_xml.sh ' + fo.name + ' ' + args)
      print('python finished')
  else:
    tree.write(values['o'])


if __name__ == '__main__':

  values = vars(parser.parse_args())
  main(values['input'], values)

