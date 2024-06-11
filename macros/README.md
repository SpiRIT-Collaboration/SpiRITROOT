# Unpacking many runs

You can unpack many runs at once like:
```
./parallel --jobs 5 --joblog ./log.txt < input/parallelInput.txt
```

where `--jobs` sets the maximum number of jobs to run at once. `--joblog ./fileName` sets the log file which will save the command, runtime, and return/error code. The file piped in contains each command to run (one per line).

The file to pass to unpack all 150 torr runs is `./input/150torr.txt`
The file to pass to create ROOT files split by species is `./input/species150Torr.txt`
