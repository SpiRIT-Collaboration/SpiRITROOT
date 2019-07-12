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
from scipy.interpolate import RegularGridInterpolator
from matplotlib.colors import BoundaryNorm
from matplotlib.ticker import MaxNLocator
from scipy.ndimage import zoom
from matplotlib.widgets import Slider

class TPCGrid:
  Width = 100.#86.4
  Height = 55.#50.61
  Length = 150.#134.
  
  Vbottom = -7023.5#6463
  Vtop = 0

  def __init__(self, h=1.4):#1.4):
    # h must equally divide y for boundary condition to be exact
    h = self.Height/int(self.Height/h)
    self.h = h
    self.x = np.arange(0, self.Width + h, h)
    self.y = np.arange(0, self.Height + h, h)
    self.z = np.arange(0, self.Length + h, h)
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
    pass

  @property
  def TPCx(self):
    return self.Width/2. - self.x

  @property
  def TPCy(self):
    return self.y - self.Height

  @property
  def TPCz(self):
    return self.z

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
    # convert to my calculation coordinates

    line_idx = [np.abs(x - X).argmin() for X in beam_x]
    line_idy = [np.abs(y - Height).argmin() for Height in beam_y]
    self.geo_index = [[], [], []]
    self.geo_charge = []
    for idz, (max_idy, idx) in enumerate(zip(line_idy, line_idx)):
      # cannot modify boundaries. Therefore range start from 1
      if idx == 0 or idx == x.shape[0] - 1:
        continue
      if idz == 0 or idz == z.shape[0] - 1:
        continue
      if max_idy == y.shape[0] - 1:
        max_idy -= 1
      self.geo_index[0] += [idz for idy in range(1, max_idy)]
      self.geo_index[1] += [idy for idy in range(1, max_idy)]
      self.geo_index[2] += [idx for idy in range(1, max_idy)]
      self.geo_charge += [self.sheet_density/2. for idy in range(1, max_idy)]

    for idz, (max_idy, idx) in enumerate(zip(line_idy, line_idx)):
      # cannot modify boundaries. Therefore range start from 1
      if idx + 1 == x.shape[0] - 1:
        continue
      if idz == 0 or idz == z.shape[0] - 1:
        continue
      if max_idy == y.shape[0] - 1:
        max_idy -= 1
      self.geo_index[0] += [idz for idy in range(1, max_idy)]
      self.geo_index[1] += [idy for idy in range(1, max_idy)]
      self.geo_index[2] += [idx + 1 for idy in range(1, max_idy)]
      self.geo_charge += [(- beam_x[idz] + x[idx] + 0.5*h)*self.sheet_density/(2.*h) for idy in range(1, max_idy)]

    for idz, (max_idy, idx) in enumerate(zip(line_idy, line_idx)):
      # cannot modify boundaries. Therefore range start from 1
      if idx - 1 == 0:
        continue
      if idz == 0 or idz == z.shape[0] - 1:
        continue
      if max_idy == y.shape[0] - 1:
        max_idy -= 1
      self.geo_index[0] += [idz for idy in range(1, max_idy)]
      self.geo_index[1] += [idy for idy in range(1, max_idy)]
      self.geo_index[2] += [idx - 1 for idy in range(1, max_idy)]
      self.geo_charge += [(beam_x[idz] - x[idx] + 0.5*h)*self.sheet_density/(2.*h) for idy in range(1, max_idy)]


    self.geo_index = [np.array(geo_index) for geo_index in self.geo_index]
    self.geo_charge = np.array(self.geo_charge)/h#self.sheet_density/h # divide by h for line charge to conserve total charge
      
class PossionSolver:

  def __init__(self, grid, charge):
    self.grid = grid
    self.charge = charge
    self.charge.SetGeo(grid.TPCx, grid.TPCy, grid.TPCz, grid.h)

  def Stepper(self, save_error=False):
    if save_error:
      self.grid.Snapshot()
    self.grid.V[1:-1, 1:-1, 1:-1] = 1/6.*(self.grid.V[1:-1, 1:-1, :-2]
                                        + self.grid.V[1:-1, 1:-1, 2:]
                                        + self.grid.V[1:-1, :-2, 1:-1]
                                        + self.grid.V[1:-1, 2:, 1:-1]
                                        + self.grid.V[2:, 1:-1, 1:-1]
                                        + self.grid.V[:-2, 1:-1, 1:-1])
    self.grid.V[tuple(self.charge.geo_index)] += 1/6.*self.grid.h*self.grid.h*self.charge.geo_charge
    #self.grid.UpdateBC()

  def Solve(self, tol=1e-2, max_iter=5000):
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
  beam_file = strength_and_beamfile[1]

  grid = TPCGrid()
  charge = LineCharge(beam_file, strength)
  solver = PossionSolver(grid, charge)
  solver.Solve()
  result = grid.V

  #ArrayToGif(grid.TPCx, grid.TPCy, grid.TPCz, result, 'strength%.2E.gif' % strength)
  Ex, Ey, Ez = VToE(result, grid.TPCx, grid.TPCy, grid.TPCz)
  E_strength = np.sqrt(Ex*Ex + Ey*Ey + Ez*Ez)
  print('Maximum E-field strength: %.2f V/cm' % E_strength.ravel().max())
  print('Minimum E-field strength: %.2f V/cm' % E_strength.ravel().min())
  density_grid = np.zeros(grid.V.shape)
  density_grid[tuple(charge.geo_index)] = charge.geo_charge
  return grid.TPCx, grid.TPCy, grid.TPCz, Ex, Ey, Ez, result, density_grid

def ArrayToGif(x, y, z, content, title):
  fig, ax = plt.subplots()
  plt.subplots_adjust(bottom=0.25)
  cmap = plt.get_cmap('inferno')
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


  anim = FuncAnimation(fig, update, frames=np.arange(0, new_content.shape[0]), interval=33)
  anim.save(title, writer='imagemagick')

def Fill3DHist(hist, content, x, y, z):
  for index, val in np.ndenumerate(content):
    hist.Fill(x[index[2]], y[index[1]], z[index[0]], val)

if __name__ == '__main__':
  file_ = ROOT.TFile.Open('E-field.root', 'RECREATE')

  # lets try different a
  fig, ax = plt.subplots()
  #strengths = np.linspace(0.0)
  beam_files = ['_132Sn_BeamTrack.data']
  beam_name = ['132Sn']
  strengths_and_beamfile = [(3.14e-8*factor, beam_file) for factor in range(0,2) for beam_file in beam_files]
  
  pool = Pool()
  results = pool.map(CalculateEField, strengths_and_beamfile)
  pool.close()
  pool.join()

  # homogeneous solution
  print('Export result to ROOT')
  for beam_file, homo_result, nohomo_result in zip(beam_name, results[::2], results[1::2]):
    x, y, z, Ex_homo, Ey_homo, Ez_homo, V_homo, rho_homo = results[0]
    # non-homogeneous solution
    _, _, _, Ex_nohomo, Ey_nohomo, Ez_nohomo, V_nohomo, rho_nohomo = results[1]
    Ex_nohomo = Ex_nohomo - Ex_homo
    Ey_nohomo = Ey_nohomo - Ey_homo
    Ez_nohomo = Ez_nohomo - Ez_homo
    V_nohomo = V_nohomo - V_homo

    # save everything to ROOT
    for type_, (Ex, Ey, Ez, V, rho) in zip(['homo', 'nohomo'], [[Ex_homo, Ey_homo, Ez_homo, V_homo, rho_homo], [Ex_nohomo, Ey_nohomo, Ez_nohomo, V_nohomo, rho_nohomo]]):
      for title, content in zip(['Ex', 'Ey', 'Ez', 'V', 'rho'], [Ex, Ey, Ez, V, rho]):
        hist = ROOT.TH3D(title, type_, len(x), x.min(), x.max()
                                     , len(y), y.min(), y.max()
                                     , len(z), z.min(), z.max())
        Fill3DHist(hist, content, x, y, z)
        hist.Write('%s_%s_%s' % (type_, beam_file, title))
        hist.Delete()

  file_.Close()
  
