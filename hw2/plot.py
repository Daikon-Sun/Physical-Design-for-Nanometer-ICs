import seaborn as sns
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

case = 'ami49'

data1 = open('logs/'+case+'_1', 'r').read().splitlines()
data1 = np.array([datum.split() for datum in data1], dtype=np.float32)

data2 = open('logs/'+case+'_2', 'r').read().splitlines()
data2 = np.array([datum.split() for datum in data2], dtype=np.float32)

data3 = open('logs/'+case+'_3', 'r').read().splitlines()
data3 = np.array([datum.split() for datum in data3], dtype=np.float32)

data4 = open('logs/'+case+'_4', 'r').read().splitlines()
data4 = np.array([datum.split() for datum in data4], dtype=np.float32)

data = np.vstack((data1, data2, data3, data4))
cate = np.array([['one-stage']]*500+[['two-stage']]*500+[['nonzero beta']]*500+[['combine']]*500)
max_val = np.max(data[:, :4], 0)
min_val = np.min(data[:, :4], 0)
mean_val1 = np.mean(data[:500, :4], 0)
mean_val2 = np.mean(data[500:1000, :4], 0)
mean_val3 = np.mean(data[1000:1500, :4], 0)
mean_val4 = np.mean(data[1500:, :4], 0)
data = np.hstack((data, cate))

df = pd.DataFrame(data=data, columns=['quality', 'runtime', 'method'])
g = sns.FacetGrid(df, hue='method')
g = g.map(plt.scatter, 'quality', 'runtime', edgecolor='w')
plt.legend(loc='upper right')
plt.title('case: '+case)
plt.ylim([0,max_val[1]*1.01])
plt.xlim([min_val[0]*0.99, max_val[0]*1.01])
plt.plot([min_val[0]*0.99, max_val[0]*1.01], [mean_val1[1], mean_val1[1]], 'b-')
plt.plot([min_val[0]*0.99, max_val[0]*1.01], [mean_val2[1], mean_val2[1]], 'g-')
plt.plot([min_val[0]*0.99, max_val[0]*1.01], [mean_val3[1], mean_val3[1]], 'r-')
plt.plot([min_val[0]*0.99, max_val[0]*1.01], [mean_val4[1], mean_val4[1]], 'm-')
plt.plot([mean_val1[0], mean_val1[0]], [0, max_val[1]*1.01], 'b-')
plt.plot([mean_val2[0], mean_val2[0]], [0, max_val[1]*1.01], 'g-')
plt.plot([mean_val3[0], mean_val3[0]], [0, max_val[1]*1.01], 'r-')
plt.plot([mean_val4[0], mean_val4[0]], [0, max_val[1]*1.01], 'm-')

fig = plt.gcf()
fig.set_size_inches(10, 5)
fig.savefig('graphs/'+case+'.png')
fig.show()
input()
