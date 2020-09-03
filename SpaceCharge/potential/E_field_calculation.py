import matplotlib
matplotlib.use('Agg')

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import scipy
import ROOT
from matplotlib.animation import FuncAnimation
from multiprocessing import Pool
from functools import partial
from matplotlib.colors import BoundaryNorm
from matplotlib.ticker import MaxNLocator
from scipy.ndimage import zoom
from matplotlib.widgets import Slider

class TPCGrid:
  Width = 86.4
  Height = 50.61
  Length = 144.35#134.
  
  Vbottom = -124.7*Height#-7023.5#6463
  Vtop = 0

  def __init__(self, dx=1, dy=2, dz=1):#h=1.4):#1.4):
    # h must equally divide y for boundary condition to be exact
    self.dx = self.Width/(int(self.Width/dx))
    self.dy = self.Height/(int(self.Height/dy))
    self.dz = self.Length/(int(self.Length/dz))

    self.x = np.arange(0, self.Width + self.dx, self.dx)
    self.y = np.arange(0, self.Height + self.dy, self.dy)
    self.z = np.arange(0, self.Length + self.dz, self.dz)
    self.V = np.zeros((self.z.shape[0], self.y.shape[0], self.x.shape[0]))
    self.V[:, :, :] = (self.Vbottom + (self.Vtop - self.Vbottom)*self.y/self.Height)[None, :, None]
    self._ApplyBC()
    self.old_val = self.V.copy()

  def _ApplyBC(self):
    self.V[:, 0, :] = self.Vbottom
    self.V[:, -1, :] = self.Vtop
    self.V[0, :, :] =  (self.Vbottom + (self.Vtop - self.Vbottom)*self.y/self.Height)[:, None]
    self.V[-1, :, :] = (self.Vbottom + (self.Vtop - self.Vbottom)*self.y/self.Height)[:, None]
    self.V[:, :, 0] =  (self.Vbottom + (self.Vtop - self.Vbottom)*self.y/self.Height)[None, :]
    self.V[:, :, -1] = (self.Vbottom + (self.Vtop - self.Vbottom)*self.y/self.Height)[None, :]

  def Snapshot(self):
    self.old_val = self.V.copy()

  def Error(self):
    tmp = self.V - self.old_val
    return np.sqrt(np.mean((tmp*tmp).ravel()))

  def UpdateBC(self):
    self._ApplyBC()

  @property
  def TPCx(self):
    return self.Width/2. - self.x

  @property
  def TPCy(self):
    return self.y - self.Height

  @property
  def TPCz(self):
    return self.z

class SecVolCharge:
  def __init__(self):
    pass

  def SetGeo(self, x, y, z, h):
    ch_x, ch_y, ch_z = np.meshgrid(x[1:-1], y[1:-1], z[1:-1])
    ch_x = ch_x.flatten()
    ch_y = ch_y.flatten()
    ch_z = ch_z.flatten()
    ch = (x.max()  - np.fabs(ch_x))/x.max()*(np.fabs(ch_y)/np.fabs(y).max())

    idx, idy, idz = np.meshgrid(np.arange(x.shape[0]), np.arange(y.shape[0]), np.arange(z.shape[0]))
    self.geo_index = [idz.flatten(), idy.flatten(), idx.flatten()]
    self.geo_charge = ch

class LineCharge:

  def __init__(self, beam_name, sheet_density=1.28e-8):
    self.beam_loc = pd.read_csv(beam_name, sep=r'\s+')/10.
    self.beam_loc.columns = ['x', 'y', 'z']
    
    # convert from Si To the unit of this program
    permittivity = 8.854e-12
    # divide 100 to convert C/m2 to C/cm2
    self.sheet_density = sheet_density/100/permittivity

  def SetGeo(self, x, y, z, h):
    # our sheet charge has to span 2 pixels
    beam_x = np.interp(z[z < self.beam_loc['z'].max()], self.beam_loc['z'], self.beam_loc['x'])
    beam_y = np.interp(z[z < self.beam_loc['z'].max()], self.beam_loc['z'], self.beam_loc['y'])
    # convert real location to index
    line_idx = [np.abs(x - X).argmin() for X in beam_x]
    line_idy = [np.abs(y - Height).argmin() for Height in beam_y]

    # draw geometry
    self.geo_index = [[], [], []]
    self.geo_charge = [] 
    factor_along_beam =  np.ones(z.shape)#1 - np.exp(-np.fabs(z)/10)
    for idz, (max_idy, idx) in enumerate(zip(line_idy, line_idx)):
      self.geo_index[0] += [idz for idy in range(max_idy)]
      self.geo_index[1] += [idy for idy in range(max_idy)]
      self.geo_index[2] += [idx for idy in range(max_idy)]
      self.geo_charge += [factor_along_beam[idz]*self.sheet_density/2. for idy in range(max_idy)]

      if idx + 1 < x.shape:
        self.geo_index[0] += [idz for idy in range(max_idy)]
        self.geo_index[1] += [idy for idy in range(max_idy)]
        self.geo_index[2] += [idx + 1 for idy in range(max_idy)]
        self.geo_charge += [factor_along_beam[idz]*(- beam_x[idz] + x[idx] + 0.5*h)*self.sheet_density/(2.*h) for idy in range(max_idy)]
      if idx - 1 >= 0:
        self.geo_index[0] += [idz for idy in range(max_idy)]
        self.geo_index[1] += [idy for idy in range(max_idy)]
        self.geo_index[2] += [idx - 1 for idy in range(max_idy)]
        self.geo_charge += [factor_along_beam[idz]*(beam_x[idz] - x[idx] + 0.5*h)*self.sheet_density/(2.*h) for idy in range(max_idy)]

    self.geo_index = [np.array(geo_index) for geo_index in self.geo_index]
    self.geo_charge = np.array(self.geo_charge)/h#self.sheet_density/h # divide by h for line charge to conserve total charge

class BackFlowCharge:

  def __init__(self, beam_name, sheet_density=1.28e-8):
    self.beam_loc = pd.read_csv(beam_name, sep=r'\s+')/10.
    self.beam_loc.columns = ['x', 'y', 'z']
    
    # convert from Si To the unit of this program
    permittivity = 8.854e-12
    # divide 100 to convert C/m2 to C/cm2
    self.sheet_density = sheet_density/100/permittivity

  def SetGeo(self, x, y, z, h):
    # our sheet charge has to span 2 pixels
    beam_z = z[( 140 < z) & (z < 143.95)]
    beam_x = np.interp(beam_z, self.beam_loc['z'], self.beam_loc['x'])
    beam_y = np.interp(beam_z, self.beam_loc['z'], self.beam_loc['y'])
    # convert real location to index
    line_idx = [np.abs(x - X).argmin() for X in beam_x]
    line_idy = [len(y) - 1 for Height in beam_y]
    line_idz = [idz for idz, IsBGArea in enumerate((140 < z) & (z < 143.95)) if IsBGArea]

    # draw geometry
    self.geo_index = [[], [], []]
    self.geo_charge = [] 
    for idarr, (idz, max_idy, idx) in enumerate(zip(line_idz, line_idy, line_idx)):
      self.geo_index[0] += [idz for idy in range(max_idy)]
      self.geo_index[1] += [idy for idy in range(max_idy)]
      self.geo_index[2] += [idx for idy in range(max_idy)]
      self.geo_charge += [self.sheet_density/2. for idy in range(max_idy)]

      if idx + 1 < x.shape:
        self.geo_index[0] += [idz for idy in range(max_idy)]
        self.geo_index[1] += [idy for idy in range(max_idy)]
        self.geo_index[2] += [idx + 1 for idy in range(max_idy)]
        self.geo_charge += [(- beam_x[idarr] + x[idx] + 0.5*h)*self.sheet_density/(2.*h) for idy in range(max_idy)]
      if idx - 1 >= 0:
        self.geo_index[0] += [idz for idy in range(max_idy)]
        self.geo_index[1] += [idy for idy in range(max_idy)]
        self.geo_index[2] += [idx - 1 for idy in range(max_idy)]
        self.geo_charge += [(beam_x[idarr] - x[idx] + 0.5*h)*self.sheet_density/(2.*h) for idy in range(max_idy)]

    self.geo_index = [np.array(geo_index) for geo_index in self.geo_index]
    self.geo_charge = np.array(self.geo_charge)/h#self.sheet_density/h # divide by h for line charge to conserve total charge
 
      
class PossionSolver:

  def __init__(self, grid, charges):
    self.grid = grid
    self.charges = charges
    for charge in self.charges:
      charge.SetGeo(grid.TPCx, grid.TPCy, grid.TPCz, grid.dx)

  def Stepper(self, save_error=False):
    if save_error:
      self.grid.Snapshot()
    inv_dsum = 1/(2*(1/(self.grid.dx**2) + 1/(self.grid.dy**2) + 1/(self.grid.dz**2)))
    self.grid.V[1:-1, 1:-1, 1:-1] = inv_dsum*((self.grid.V[1:-1, 1:-1, :-2]
                                             + self.grid.V[1:-1, 1:-1, 2:])/(self.grid.dx**2)
                                             + (self.grid.V[1:-1, :-2, 1:-1]
                                             + self.grid.V[1:-1, 2:, 1:-1])/(self.grid.dy**2)
                                             + (self.grid.V[:-2, 1:-1, 1:-1]
                                             + self.grid.V[2:, 1:-1, 1:-1])/(self.grid.dz**2))
    for charge in self.charges:
      self.grid.V[tuple(charge.geo_index)] += inv_dsum*charge.geo_charge
    self.grid.UpdateBC()

  def Solve(self, tol=1e-4, max_iter=5000):
    idx = 0
    while True:
      self.Stepper(save_error=True)
      idx += 1
      if self.grid.Error() < tol or idx > max_iter:
        print('Total iteration: %d, Average RMS Error: %.2f' % (idx, self.grid.Error()))
        break

def VToE(content, x, y, z):
  Ez, Ey, Ex = np.gradient(-content, z, y, x)
  return Ex, Ey, Ez

def CalculateEField(strength_and_beamfile):
  # load beam line data
  strength = strength_and_beamfile[0]
  strength_backflow = strength_and_beamfile[1]
  beam_file = strength_and_beamfile[2]

  grid = TPCGrid()
  line_charge = LineCharge(beam_file, strength)
  backflow_charge = BackFlowCharge(beam_file, strength_backflow)
  solver = PossionSolver(grid, [line_charge, backflow_charge])
  solver.Solve()
  result = grid.V

  density_grid = np.zeros(grid.V.shape)
  for charge in solver.charges:
    density_grid[tuple(charge.geo_index)] += charge.geo_charge

  #ArrayToGif(grid.TPCx, grid.TPCy, grid.TPCz, result, 'strength%.2E_%s.gif' % (strength, beam_file))
  Ex, Ey, Ez = VToE(result, grid.TPCx, grid.TPCy, grid.TPCz)
  E_strength = np.sqrt(Ex*Ex + Ey*Ey + Ez*Ez)
  print('Maximum E-field strength: %.2f V/cm' % E_strength.ravel().max())
  print('Minimum E-field strength: %.2f V/cm' % E_strength.ravel().min())

  return grid.TPCx, grid.TPCy, grid.TPCz, Ex, Ey, Ez, result, density_grid, beam_file

def ArrayToGif(x, y, z, content, title):
  fig, ax = plt.subplots()
  plt.subplots_adjust(bottom=0.25)
  #cmap = plt.get_cmap('inferno')
  levels = MaxNLocator(nbins=15).tick_values(content.ravel().min(), content.ravel().max())
  norm = BoundaryNorm(levels, ncolors=cmap.N)

  x_factor, y_factor, z_factor = (2, 4, 2)
  new_content = zoom(content, (z_factor, y_factor, x_factor))
  new_x = zoom(x, x_factor)
  new_y = zoom(y, y_factor)
  new_z = zoom(z, z_factor)
  im = ax.pcolormesh(new_x, new_y, new_content[0, :-1, :-1], cmap=cmap, norm=norm)
  ax.set_xlabel('x (cm)')
  ax.set_ylabel('y (cm)')
  ax.set_title('Potential map (V)')
  fig.colorbar(im, ax=ax)
  z_slider = Slider(plt.axes([0.1,0.1, 0.7, 0.03]), 'Z (cm)', z.min(), z.max(), valinit=z.min(), valstep=len(new_z))

  def update(i):
    z_slider.set_val(new_z[i])
    im.set_array(new_content[i, :-1, :-1].ravel())


  #anim = FuncAnimation(fig, update, frames=np.arange(0, new_content.shape[0]), interval=33)
  #anim.save(title, writer='imagemagick')

def Fill3DHist(hist, content, x, y, z):
  dx = np.fabs(x[1] - x[0])
  dy = np.fabs(y[1] - y[0])
  dz = np.fabs(z[1] - z[0])
  # pad the array for TH3D interpolation
  padx = np.pad(x, (1,1), 'constant', constant_values=(x.min()-dx, x.max()+dx))
  pady = np.pad(y, (1,1), 'constant', constant_values=(y.min()-dy, y.max()+dy))
  padz = np.pad(z, (1,1), 'constant', constant_values=(z.min()-dz, z.max()+dz))
  pad_content = np.pad(content, ((1,1),(1,1),(1,1)), 'edge')
  for index, val in np.ndenumerate(pad_content):
    hist.Fill(padx[index[2]], pady[index[1]]+dy, padz[index[0]], val)

if __name__ == '__main__':
  file_ = ROOT.TFile.Open('E-field.root', 'RECREATE', '', 9)

  # lets try different a
  fig, ax = plt.subplots()
  #strengths = np.linspace(0.0)
  beam_files = ['_132Sn_BeamTrack.data', '_124Sn_BeamTrack.data', '_108Sn_BeamTrack.data', '_112Sn_BeamTrack.data']
  beam_name = ['132Sn', '124Sn', '108Sn', '112Sn']
  strengths_and_beamfile = [(3.14e-8*factor,0, beam_file) for beam_file in beam_files for factor in range(0,2)]
  strengthsBF_and_beamfile = [(0, 3.14e-8, beam_file) for beam_file in beam_files]

  pool = Pool(1)
  results = pool.map(CalculateEField, strengths_and_beamfile + strengthsBF_and_beamfile)
  #resultsBF = pool.map(CalculateEField, strengthsBF_and_beamfile)
  pool.close()
  pool.join()

  results, resultsBF = results[:-4], results[-4:]
  # homogeneous solution
  print('Export result to ROOT')
  for beam_file, homo_result, nohomo_result, bf_result in zip(beam_name, results[::2], results[1::2], resultsBF):
    x, y, z, Ex_homo, Ey_homo, Ez_homo, V_homo, rho_homo, filename = homo_result
    # non-homogeneous solution
    _, _, _, Ex_nohomo, Ey_nohomo, Ez_nohomo, V_nohomo, rho_nohomo, nohomo_filename = nohomo_result
    _, _, _, Ex_bf, Ey_bf, Ez_bf, V_bf, rho_bf, bf_filename = bf_result

    Ex_nohomo = Ex_nohomo - Ex_homo
    Ey_nohomo = Ey_nohomo - Ey_homo
    Ez_nohomo = Ez_nohomo - Ez_homo
    Ex_bf = Ex_bf - Ex_homo
    Ey_bf = Ey_bf - Ey_homo
    Ez_bf = Ez_bf - Ez_homo
    V_nohomo = V_nohomo - V_homo
    V_bf = V_bf - V_homo

    # save everything to ROOT
    for type_, (Ex, Ey, Ez, V, rho) in zip(['homo', 'nohomo', 'bf'], [[Ex_homo, Ey_homo, Ez_homo, V_homo, rho_homo], [Ex_nohomo, Ey_nohomo, Ez_nohomo, V_nohomo, rho_nohomo], [Ex_bf, Ey_bf, Ez_bf, V_bf, rho_bf]]):
      for title, content in zip(['Ex', 'Ey', 'Ez', 'V', 'rho'], [Ex, Ey, Ez, V, rho]):
        dx = np.fabs(x[1] - x[0])
        dy = np.fabs(y[1] - y[0])
        dz = np.fabs(z[1] - z[0])
        hist = ROOT.TH3D(title, type_, len(x) + 2, x.min() - dx, x.max() + dx
                                     , len(y) + 2, y.min(), y.max() + 2*dy
                                     , len(z) + 2, z.min() - dz, z.max() + dz)

        Fill3DHist(hist, content, x, y, z)
        hist.Write('%s_%s_%s' % (type_, beam_file, title))
        hist.Delete()

  file_.Close()
  
