import numpy as np
import pandas as pd
import os
import datetime
import matplotlib as mpl
from svgpathtools import svg2paths
from svgpath2mpl import parse_path
from matplotlib import pyplot as plt

def contributionPlot(date, activity_observed, by = "month", save = "/home/pi/project/canvas/con/"):
	if (save == "temp"):
		data_path = "/tmp/.umjicanvas/"
		png_path = data_path + "temp.png"
	else:
		data_path = save + "img/"
		png_path = data_path + datetime.datetime.now().strftime("%Y-%m-%d") + ".png"
	activity = activity_observed
	if not (os.path.exists(data_path)):
		os.makedirs(data_path)

	if (by == "month"):
		row = 5
		days = 31
		fig_len = 3
	elif (by == "year"):
		row = 53
		days = 366
		fig_len = 30
	if (len(activity) < days):
		for i in range(days - len(activity)):
			activity = np.insert(activity, 0, 0)
			# print("insert")
	elif (len(activity) > days):
		activity = activity[-days:]
	for i in range(row * 7 - days):
		activity = np.append(activity, -100000)

	planet_path, attributes = svg2paths("/home/pi/project/canvas/con/rounded.svg")
	planet_marker = parse_path(attributes[0]['d'])
	planet_marker.vertices -= planet_marker.vertices.mean(axis=0)
	planet_marker = planet_marker.transformed(mpl.transforms.Affine2D().rotate_deg(180))
	planet_marker = planet_marker.transformed(mpl.transforms.Affine2D().scale(-1,1))
	x0 = np.zeros(7, dtype = int)
	y0 = np.arange(7)[::-1]
	x = x0
	y = y0
	for i in range(row - 1):
		x = np.append(x, x0 + i + 1)
		y = np.append(y, y0)
	# print(x)
	# print(y)
	c = np.random.randint(100, size = 371)#activity
	plt.figure(figsize = (fig_len, 4))
	plt.style.use('dark_background')
	plt.scatter(x = x, y = y, c = c, s = 700, marker = planet_marker, cmap = "bone", vmin = -max(c) * 0.1, vmax = max(c))
	# plt.text(-10, 6.5, "June", fontsize = 20)
	# plt.colorbar()
	plt.axis("off")
	plt.xlim([-1, row])
	plt.ylim([-1, 7])
	plt.savefig(png_path)
	if (save == "temp"):
		os.system("gthumb " + png_path)
		os.system("rm " + png_path)
	else:
		print("Saved contribution plot at " + png_path)