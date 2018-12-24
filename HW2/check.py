#!/usr/bin/python3
import sys
import numpy as np

with open("input_pa2/"+sys.argv[1]+".block", 'r') as f:
  tmp = f.readline().split()
  width = int(tmp[1])
  height = int(tmp[2])
  num_blocks = int(f.readline().split()[1])
  num_terminals = int(f.readline().split()[1])

  blocks = []
  while len(blocks) < num_blocks:
    tmp = f.readline().split()
    if len(tmp) == 0: continue
    blocks.append([tmp[0], int(tmp[1]), int(tmp[2])])

  terminals = []
  while len(terminals) < num_terminals:
    tmp = f.readline().split()
    if len(tmp) == 0: continue
    terminals.append([tmp[0], int(tmp[2]), int(tmp[3])])

with open("input_pa2/"+sys.argv[1]+".nets", 'r') as f:
  num_nets = int(f.readline().split()[1])
  nets = []
  for i in range(num_nets):
    net = []
    deg = int(f.readline().split()[1])
    for j in range(deg):
      net.append(f.readline()[:-1])
    nets.append(net)

with open(sys.argv[2], 'r') as f:
  cost = float(f.readline())
  wirelength = float(f.readline())
  chip_area = float(f.readline())
  tmp = f.readline().split()
  chip_width, chip_height = int(tmp[0]), int(tmp[1])
  time = float(f.readline())
  macros = [ line.split() for line in f ]
  names = [ macro[0] for macro in macros ]
  macros = np.array([ [int(macro[1]), int(macro[2]), int(macro[3]),
              int(macro[4])] for macro in macros ])
  max_x = np.max(macros[:, 2])
  max_y = np.max(macros[:, 3])

def overlap(m1, m2):
  return \
    not (m1[2] <= m2[0] or m1[0] >= m2[2] or m1[3] <= m2[1] or m1[1] >= m2[3])

good = True
for i in range(macros.shape[0]):
  for j in range(macros.shape[0]):
    if i != j and overlap(macros[i], macros[j]):
      good = False
      break
  if not good: break

same = (len(names) == len(blocks))
for i, name in enumerate(names):
  vis = 0
  for block in blocks:
    if block[0] == name:
      vis += 1
      len_w = macros[i, 2]-macros[i, 0]
      len_h = macros[i, 3]-macros[i, 1]
      len_x = min(block[1], block[2])
      len_y = max(block[1], block[2])
      same = (same and len_x == min(len_w, len_h) and len_y == max(len_w, len_h))
  same = (same and (vis == 1))
  if not same: break

area = hpwl = 0
for j, net in enumerate(nets):
  mn_x = mn_y = 1000000000000
  mx_x = mx_y = 0
  for pin in net:
    done = False
    for i, m in enumerate(macros):
      if names[i] == pin:
        done = True
        x = 2*m[0]+(m[2]-m[0])
        y = 2*m[1]+(m[3]-m[1])
        break
    if not done:
      for term in terminals:
        if term[0] == pin:
          x = 2*term[1]
          y = 2*term[2]
          done = True
          break
    assert done
    mn_x = min(mn_x, x)
    mn_y = min(mn_y, y)
    mx_x = max(mx_x, x)
    mx_y = max(mx_y, y)
  hpwl += (mx_x-mn_x+mx_y-mn_y)

area = max_x*max_y

print("#"*70)
print("           input: {}".format(sys.argv[1]))
print("   num of blocks: {}".format(len(blocks)))
print("num of terminals: {}".format(len(terminals)))
print("     num of nets: {}".format(len(nets)))
print("            area: {}".format(area))
print(" area difference: {}".format((area-chip_area)/area))
print("            hpwl: {}".format(hpwl/2))
print(" hpwl difference: {}".format((hpwl/2-wirelength)/hpwl/2))
print("      total cost: {}".format((hpwl/2+area)/2))
print("            SAME" if same else
      "       DIFFERENT")
print("           LEGAL" if good else
      "         ILLEGAL")
print("    OUT OF BOUND" if (max_x>width or max_y>height) else
      "        IN BOUND")
print("#"*70)
