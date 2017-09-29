-- set some global variables
e0 = 0.244816
gamma = 1.4

-- mesh sizes
num_cells = {20, 20}
length = {1.2, 1.2}

-- compute the reference cell size
delta_vol = 1
delta_r = 1.e-12
for i = 1,2 do
  local dx = length[i]/num_cells[i]
  delta_vol = delta_vol * dx
  delta_r = delta_r + math.pow( dx/2, 2 )
end
delta_r = math.sqrt( delta_r )

-- Begin Main Input
hydro = {

  -- The case prefix and postfixes
  prefix = "sedov_2d",
  postfix = "dat",
  -- The frequency of outputs
  output_freq = "20",
  -- The time stepping parameters
  final_time = 1.0,
  max_steps = 20,
  initial_time_step = 1.e-5,
  CFL = { acoustic = 0.25, volume = 0.1, growth = 1.01 },

  -- the mesh
  mesh = {
    type = "box",
    dimensions = num_cells,
    xmin = {0, 0},
    xmax = length
  },

  -- the equation of state
  eos = {
    type = "ideal_gas",
    gas_constant = 1.4,
    specific_heat = 1.0
  },

  -- the initial conditions
  -- return density, velocity, pressure
  ics = function (x,y,t)
    local d = 1.0
    local v = {0,0}
    local p = 1.e-6
    local r = math.sqrt( x*x + y*y )
    if r < delta_r then
      p = (gamma - 1) * d * e0 / delta_vol
    end
    return d, v, p
  end,

  -- the boundary conditions
  --
  -- - both +ve and -ve side boundaries can be installed at once since
  --   they will never overlap
  -- - if they did overlap, then you need to define them seperately or else
  --   its hard to count the number of different conditions on points or edges.
  bcs1 = {
    type = "symmetry",
    func = function (x,y,t)
      if x == 0 or x == length[1] then
        return true
      else
        return false
      end
    end,
  },

  bcs2 = {
    type = "symmetry",
    func = function (x,y,t)
      if y == 0 or y == length[2] then
        return true
      else
        return false
      end
    end,
  }

} -- hydro
