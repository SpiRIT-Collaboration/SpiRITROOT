import xml.etree.ElementTree as ET
import argparse
import os
import tempfile

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

  values = vars(parser.parse_args())

  # read xml
  tree = ET.parse(values['input'])
  root = tree.getroot()

  parameter_table = {'MMin': 'TaskList/EventFilterTask/MultiplicityMin',
                     'MMax': 'TaskList/EventFilterTask/MultiplicityMax',
                     'Dir': 'IOInfo/DataDir',
                     'Unfold': 'TaskList/EfficiencyTask/UpdateUnfolding',
                     'TgMass': 'TaskList/TransformFrameTask/TargetMass',
                     'Filename': 'IOInfo/InputName'}

  for key, path in parameter_table.items():
    value = values[key]
    if value is not None:
      ele = root.find(path)
      if ele is None:
        print('Cannot find node ' + path + '. Will not change ' + key)
      else:
        ele.text = value

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


