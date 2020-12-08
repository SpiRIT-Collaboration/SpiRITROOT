#!/bin/sh

if [ "$#" -eq 4 ]; then
  root -b -l -q 'run_analysis_xml.C("'$1'","'$2'",'$3','$4')'
else
  root -b -l -q 'run_analysis_xml.C("'$1'","'$2'",'$3','$4','$5')'
fi
