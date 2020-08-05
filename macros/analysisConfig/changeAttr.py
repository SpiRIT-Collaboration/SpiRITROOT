import xml.etree.ElementTree as ET
import argparse
import os
import tempfile

def create_nested_node(root, path):
  nodes_to_be_created = []
  aval_root = None
  while aval_root is None:
    aval_root = root.find(path)
    if aval_root is None:
      split_string = path.rsplit('/', 1)
      nodes_to_be_created.append(split_string[1])
      path = split_string[0]
  for node in nodes_to_be_created:
    element = ET.SubElement(aval_root, node)
    aval_root = element
  return element

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='analysisConfig modifier')
  parser.add_argument('input', help='XML file to be modified')
  parser.add_argument('-o', help='Output filename')
  parser.add_argument('--MMin', help='Minimum multiplicity')
  parser.add_argument('--MMax', help='Maximum multiplicity')
  parser.add_argument('--Dir', help='Directory to data source')
  parser.add_argument('--TgMass', help='Atomic mass of the target')
  parser.add_argument('--Unfold', help='Unfold file')
  parser.add_argument('--Filename', help='Filename of the data source')
  parser.add_argument('--exec', default=False, action='store_true', help='Execute the modified config')
  parser.add_argument('--execarg', help='(Only used with exec) Arguments passed to run_analysis_xml.sh')
  parser.add_argument('--Divide', default=False, action='store_true', help='Divide each event by half')
  parser.add_argument('--Comp', help='Choose complementary tracks to the given file')
  parser.add_argument('--PhiEff', help='Change phi efficiency file')
  parser.add_argument('--rmTask', help='Remove a task', nargs='+')
  parser.add_argument('--MassCoef', help='Mass Coef')
  parser.add_argument('--ChargeCoef', help='Charge Coef')
  parser.add_argument('--ConstCoef', help='Const Coef')
  parser.add_argument('--ParticleCoef', help='Particle Coef')



  values = vars(parser.parse_args())

  # read xml
  tree = ET.parse(values['input'])
  root = tree.getroot()

  parameter_table = {'MMin': 'TaskList/EventFilterTask/MultiplicityMin',
                     'MMax': 'TaskList/EventFilterTask/MultiplicityMax',
                     'Dir': 'IOInfo/DataDir',
                     'Unfold': 'TaskList/EfficiencyTask/UpdateUnfolding',
                     'TgMass': 'TaskList/TransformFrameTask/TargetMass',
                     'Filename': 'IOInfo/InputName',
                     'PhiEff': 'TaskList/ReactionPlaneTask/PhiEff',
                     'MassCoef': 'TaskList/ReactionPlaneTask/MassCoef',
                     'ChargeCoef': 'TaskList/ReactionPlaneTask/ChargeCoef',
                     'ConstCoef': 'TaskList/ReactionPlaneTask/ConstCoef',
                     'ParticleCoef': 'TaskList/ReactionPlaneTask/ParticleCoef'}

  for key, path in parameter_table.items():
    value = values[key]
    if value is not None:
      ele = root.find(path)
      if ele is None:
        #print('Cannot find ' + path)
        create_nested_node(root, path).text = value
      else:
        ele.text = value

  tasks = root.find('TaskList')
  if values['Divide'] is True:
    child = ET.SubElement(tasks, 'DivideEventTask')
    child.text = ''
    if values['Comp'] is not None:
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
  else:
    tree.write(values['o'])


