import pandas as pd
import numpy as np
import datetime
import os

def main():
	data_path = "./data/"
	length = []
	today = datetime.date.today().isoformat()
	yesterday = (datetime.date.today() - datetime.timedelta(1)).isoformat()

	folders = next(os.walk(data_path), (None, None, []))[1]
	filenames = next(os.walk(os.path.join(data_path, folders[0])), (None, None, []))[2]
	for course in filenames:
		activities = pd.DataFrame()
		activities["id"] = pd.read_csv(os.path.join(data_path, folders[0], course), index_col = 0)["student_id"]
		for day in folders:
			df = pd.read_csv(os.path.join(data_path, day, course), index_col = 0)
			if ((day == today) | (day == yesterday)):
				if not (os.path.exists(os.path.join(data_path, ".cache", day))):
					os.makedirs(os.path.join(data_path, ".cache", day))
				df.to_csv(os.path.join(data_path, ".cache", day, course + ".gz"), compression = "gzip")
				os.remove(os.path.join(data_path, day, course))
			if (day != today):
				activities[day] = df["total_activity_time"]
		activities.to_csv(os.path.join(data_path, course + ".gz"), compression = "gzip")
		# os.rename(os.path.join(data_path, folders[-1], course))
		df = pd.read_csv(os.path.join(data_path, ".cache", today, course + ".gz"), index_col = 0, compression = "gzip")
		df.to_csv(os.path.join(data_path, course.rstrip(".csv") + "_info.csv.gz"), compression = "gzip")

	for folder in folders:
		for file in filenames:
			if ((folder == today) | (folder == yesterday)):
				continue
			os.remove(os.path.join(data_path, folder, file))
		os.rmdir(os.path.join(data_path, folder))

	df = pd.read_csv(os.path.join(data_path, filenames[2] + ".gz"), index_col = 0, compression ="gzip")
	# print(df)

if __name__ == "__main__":
    main()
