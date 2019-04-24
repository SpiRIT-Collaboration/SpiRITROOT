// Source: http://fairroot.gsi.de/?q=node/34
// 
// The following parameters are needed:
// 
// * int ncomp       - number of components in the material (ncomp= 1 for a basic material and <1 or >1 for a mixture
//                     If ncomp > 0 the array wm contains the proportion by weight of each material in the mixture.
//                     If ncomp < 0 the array wm contains the proportion by number of atoms of each kind.
//                     For more detailed information see NLMAT in Geant3 routine GSMIXT
// * float aw[ncomp] - atomic weights A for the components
// * float an[ncomp] - atomic numbers Z for the components
// * float dens      - density DENS in g cm(**-3)
// * float wm[ncomp] - weights WMAT of each component in a mixture (only for a mixture)
// * int   sensflag  - sensitivity flag ISVOL
// * int   fldflag   - fieldflag IFIELD
// * float fld       - maximum field value FIELDM in kilogauss
// * float epsil     - boundary crossing precision EPSIL
// * int   npckov    - number of values used to define the optical properties of the medium.
//
// The variable npckov is 0 for all media except some special media used for the
// Rich where the tracking of the Cerenkov photons is necessary. These media have
// additinal parameters
//
// * float ppckov[npckov] - photon momentum in eV
// * float absco[npckov]  - absorption length in case of dielectric and of absorption  probabilities in case of a metal
// * float effic[npckov]  - detection efficiency
// * float rindex[npckov] - refraction index for a dielectric, rindex[0]=0 for a metal
// 
// Remark: In the present program version a mixture may contain a maximum of 5
// components. If this is not sufficient one has to change MAXCOMP in
// hgeomedium.h.
//
// The following parameters are normally not read. The default values are -1 and
// the real values are automatically calculated by Geant. If you want to set these
// values by yourself, you must type the keyword AUTONULL in your media file.
// After this keyword all media must contain these additional 4 parameters before
// the Cerenkov (int npckov).
// 
// * float madfld  - maximum angular deviation TMAXFD due to field
// * float maxstep - maximum step permitted STEMAX
// * float maxde   - maximum fractional energy loss DEEMAX
// * float minstep - minimum value for step STMIN
//
//
// example)
//
// air 3 14.01 16. 39.95.  7.  9.  18. 1.205e-3  .755  .231  .014
// 0 1 3.  .001
// 0
//
//----------------------------------------------------------
vacuum             1  1.e-16  1.e-16  1.e-16
                   0  1  20.  .001
                   0
carbon             1  12.011  6.0  2.265
                   0  1  20.  .001
                   0
gold               1  196.97  79.  19.3
                   0  1  20.  .001
                   0
lead               1  207.1  82.  11.34
                   0  1  20.  .001
                   0
silicon            1  28.0855 14.0 2.33
                   0  1  20.  .001
                   0
aluminium          1  26.98 13. 2.7
                   0  1  20.  .001
                   0
magAl		           1  26.98 13. 2.7
                   0  1  20.  .001
                   0
beryllium          1  9.01  4.0 1.848
                   0  0  20.  .001
                   0
iron               1  55.847  26.  7.87
                   0  1  20.  .001
                   0
air                3  14.01  16.  39.95  7.  8.  18.  1.205e-3  .755  .231  .014
                   0  1  30.  .001
                   0

copper             1  63.54  29.  8.96
                   0   0     20.  0.001  0		   

kapton            -4  14.006  12.011   1.008  16.
                       7.      6.      1.      8.
                       1.42
                       2      22      10       5
                   0   0      20.      0.001   0

pcb                5  28.09      15.999     12.011      1.00794    79.904
                      14          8          6          1          35
                      1.7
                      0.180774    0.405633   0.278042   0.0684428   0.671091 
                   0  0          20.         0.001      0


polycarb	   -3  12.01  15.999  1.007  6  8  1  1.2  16  3  14
		    0  1  30.  .001
		    0
				       

// Kyotoarray mat
polystyrene        -2       1.008       12.011
                            1.          6.
                            1.06
                            8           8
                   1      1     20      0.001     0      

PPO                 -3      12.011      1.008     16.
                            6.          1.        8.
                            1.06
                            8           8         1
                            1     1     20      0.001     0

POPOP               -4      12.011      1.008     14.01     16.
                            6.          1.        7.        8.
                            1.204
                            24          16        2         2
                            1     1     20      0.001     0



// KATANA
bc404               2     1.008   12.001
                          1.      6.
                          1.032
                          .085    .915
                          1   1   20    0.001   0

bc408               2     1.008   12.001
                          1.      6.
                          1.032
                          .085    .915
                          1   1   20    0.001   0


polyvinyltoluene  -2	12.01	1.008	6.	1.	1.032	9.	10.	
		   1  1	 20.  .001	
                   0


AUTONULL

p10                3     12.011     1.008     39.95
                          6.        1.        18.
                          1.53e-3
                          0.032     0.011      0.957
                   1      1         6.         0.0001    99.
                   0.01   0.0001    0.0000001  0

argon              1     39.948    18          0.00178
                   1      1        30.         0.001     99.
                   0.01   0.0001    0.0000001  0
                   

